// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/Buffer.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/Properties.h"
#    include "../ProtocolInstance.h"
#    include "../UdpEndpointI.h"
#    include "NetworkFrameworkUdpTransceiver.h"

#    include <condition_variable>
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

namespace
{
    string
    nwEndpointToString(nw_endpoint_t endpoint)
    {
        if (!endpoint)
        {
            return "<not connected>";
        }
        string host = nw_endpoint_get_hostname(endpoint);
        uint16_t port = nw_endpoint_get_port(endpoint);
        return host + ":" + to_string(port);
    }

    // Start a recursive receive on a peer connection. Each time a datagram arrives,
    // it is queued in the shared server state and a new receive is started.
    void
    startPeerReceive(
        nw_connection_t connection,
        shared_ptr<NetworkFrameworkUdpTransceiver::ServerState> serverState,
        NativeInfoPtr nativeInfo)
    {
        nw_connection_receive(
            connection,
            1,     // minimum bytes
            65535, // maximum bytes (max UDP datagram)
            ^(dispatch_data_t content, nw_content_context_t, bool, nw_error_t error) {
                if (error)
                {
                    int code = nw_error_get_error_code(error);
                    // ECANCELED means the connection was cancelled (normal shutdown).
                    if (code == ECANCELED)
                    {
                        return;
                    }
                    // Other errors on a peer connection — the peer may have gone away.
                    // Just stop receiving on this connection.
                    return;
                }

                if (!content)
                {
                    // No content and no error — EOF. Unusual for UDP, but stop receiving.
                    return;
                }

                // Extract the datagram data.
                __block vector<byte> data;
                dispatch_data_apply(content, ^bool(dispatch_data_t, size_t, const void* buffer, size_t size) {
                    auto* bytes = static_cast<const byte*>(buffer);
                    data.insert(data.end(), bytes, bytes + size);
                    return true;
                });

                // Queue the datagram.
                {
                    lock_guard lock(serverState->mutex);
                    nw_retain(connection);
                    serverState->received.push_back({std::move(data), connection});
                    if (serverState->readWaiting)
                    {
                        serverState->readWaiting = false;
                        nativeInfo->completed(SocketOperationRead);
                    }
                }

                // Start the next receive on this peer connection.
                startPeerReceive(connection, serverState, nativeInfo);
            });
    }
}

//
// Client constructor — connected UDP via connector.
//
IceInternal::NetworkFrameworkUdpTransceiver::NetworkFrameworkUdpTransceiver(
    ProtocolInstancePtr instance,
    nw_connection_t connection)
    : _instance(std::move(instance)),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _connection(connection),
      _listener(nullptr),
      _mcastReadSource(nullptr),
      _incoming(false),
      _state(StateNeedConnect),
      _port(0),
      _connectState(make_shared<ConnectState>()),
      _writeState(make_shared<WriteState>()),
      _readState(make_shared<ReadState>()),
      _currentPeer(nullptr),
      _rcvSize(_maxPacketSize),
      _sndSize(_maxPacketSize)
{
    assert(_connection);
    nw_retain(_connection);
    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-udp-connection", DISPATCH_QUEUE_SERIAL);

    // Query OS default buffer sizes via a temporary UDP socket, matching BSD UdpTransceiver behavior.
    Address addr;
    memset(&addr, 0, sizeof(addr));
    addr.saStorage.ss_family = AF_INET;
    SOCKET tmpFd = createSocket(true, addr);
    _sndSize = getSendBufferSize(tmpFd);
    _rcvSize = getRecvBufferSize(tmpFd);
    closeSocketNoThrow(tmpFd);

    // Override with Ice.UDP properties if explicitly set.
    const string sndSizeProperty = _instance->properties()->getProperty("Ice.UDP.SndSize");
    if (!sndSizeProperty.empty())
    {
        int sz = _instance->properties()->getPropertyAsInt("Ice.UDP.SndSize");
        if (sz >= 1024)
        {
            _sndSize = sz;
        }
    }
    const string rcvSizeProperty = _instance->properties()->getProperty("Ice.UDP.RcvSize");
    if (!rcvSizeProperty.empty())
    {
        int sz = _instance->properties()->getPropertyAsInt("Ice.UDP.RcvSize");
        if (sz >= 1024)
        {
            _rcvSize = sz;
        }
    }
}

//
// Client constructor — multicast via BSD socket (NF doesn't support multicast on all interfaces).
//
IceInternal::NetworkFrameworkUdpTransceiver::NetworkFrameworkUdpTransceiver(ProtocolInstancePtr instance, SOCKET mcastFd)
    : _instance(std::move(instance)),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _connection(nullptr),
      _listener(nullptr),
      _mcastReadSource(nullptr),
      _incoming(false),
      _state(StateConnected),
      _port(0),
      _writeState(make_shared<WriteState>()),
      _currentPeer(nullptr),
      _rcvSize(_maxPacketSize),
      _sndSize(_maxPacketSize)
{
    _mcastFd = mcastFd;
    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-udp-mcast", DISPATCH_QUEUE_SERIAL);
}

//
// Server constructor — listener UDP via endpoint->transceiver().
//
IceInternal::NetworkFrameworkUdpTransceiver::NetworkFrameworkUdpTransceiver(
    UdpEndpointIPtr endpoint,
    ProtocolInstancePtr instance,
    const string& host,
    int port,
    string mcastInterface)
    : _endpoint(std::move(endpoint)),
      _instance(std::move(instance)),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _connection(nullptr),
      _listener(nullptr),
      _mcastReadSource(nullptr),
      _incoming(true),
      _state(StateNotConnected),
      _host(host),
      _port(static_cast<uint16_t>(port)),
      _mcastInterface(std::move(mcastInterface)),
      _serverState(make_shared<ServerState>()),
      _writeState(make_shared<WriteState>()),
      _currentPeer(nullptr),
      _rcvSize(_maxPacketSize),
      _sndSize(_maxPacketSize)
{
    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-udp-listener", DISPATCH_QUEUE_SERIAL);

    // Query OS default buffer sizes via a temporary UDP socket, matching BSD UdpTransceiver behavior.
    Address addr;
    memset(&addr, 0, sizeof(addr));
    addr.saStorage.ss_family = AF_INET;
    SOCKET tmpFd = createSocket(true, addr);
    _sndSize = getSendBufferSize(tmpFd);
    _rcvSize = getRecvBufferSize(tmpFd);
    closeSocketNoThrow(tmpFd);

    // Override with Ice.UDP properties if explicitly set.
    const string sndSizeProperty = _instance->properties()->getProperty("Ice.UDP.SndSize");
    if (!sndSizeProperty.empty())
    {
        int sz = _instance->properties()->getPropertyAsInt("Ice.UDP.SndSize");
        if (sz >= 1024)
        {
            _sndSize = sz;
        }
    }
    const string rcvSizeProperty = _instance->properties()->getProperty("Ice.UDP.RcvSize");
    if (!rcvSizeProperty.empty())
    {
        int sz = _instance->properties()->getPropertyAsInt("Ice.UDP.RcvSize");
        if (sz >= 1024)
        {
            _rcvSize = sz;
        }
    }
}

IceInternal::NetworkFrameworkUdpTransceiver::~NetworkFrameworkUdpTransceiver()
{
    if (_connection)
    {
        nw_release(_connection);
    }
    if (_listener)
    {
        nw_release(_listener);
    }
    if (_mcastReadSource)
    {
        // The source owns the multicast socket: its cancellation handler closes the socket once the event handler
        // can no longer run (see bind()). Cancelling is idempotent, close() normally did it already.
        dispatch_source_cancel(_mcastReadSource);
        dispatch_release(_mcastReadSource);
    }
    else if (_mcastFd != INVALID_SOCKET)
    {
        closeSocketNoThrow(_mcastFd);
    }
    if (_currentPeer)
    {
        nw_release(_currentPeer);
    }
    if (_serverState)
    {
        lock_guard lock(_serverState->mutex);
        for (auto& conn : _serverState->connections)
        {
            nw_release(conn);
        }
        _serverState->connections.clear();
        for (auto& dg : _serverState->received)
        {
            if (dg.source) // Null for multicast datagrams.
            {
                nw_release(dg.source);
            }
        }
        _serverState->received.clear();
    }
    if (_dispatchQueue)
    {
        dispatch_release(_dispatchQueue);
    }
}

NativeInfoPtr
IceInternal::NetworkFrameworkUdpTransceiver::getNativeInfo()
{
    return _nativeInfo;
}

SocketOperation
IceInternal::NetworkFrameworkUdpTransceiver::initialize(Buffer&, Buffer&)
{
    if (_state == StateNeedConnect)
    {
        _state = StateConnectPending;

        // Set up the state change handler. Capture shared state and NativeInfo by shared_ptr
        // so the block is safe even if the transceiver is destroyed before it fires.
        auto connectState = _connectState;
        NativeInfoPtr nativeInfo = _nativeInfo;

        nw_connection_set_state_changed_handler(_connection, ^(nw_connection_state_t state, nw_error_t error) {
            switch (state)
            {
                case nw_connection_state_ready:
                {
                    connectState->connected.store(true);
                    nativeInfo->completed(SocketOperationConnect);
                    break;
                }
                case nw_connection_state_failed:
                {
                    if (!connectState->connected.load())
                    {
                        connectState->error.store(error ? nw_error_get_error_code(error) : ECONNREFUSED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_waiting:
                {
                    if (!connectState->connected.load())
                    {
                        connectState->error.store(error ? nw_error_get_error_code(error) : ECONNREFUSED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_cancelled:
                {
                    if (!connectState->connected.load() && connectState->error.load() == 0)
                    {
                        connectState->error.store(ECANCELED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_preparing:
                case nw_connection_state_invalid:
                    break;
            }
        });

        nw_connection_set_queue(_connection, _dispatchQueue);
        nw_connection_start(_connection);
        return SocketOperationConnect;
    }
    else if (_state == StateConnectPending)
    {
        int error = _connectState->error.load();
        if (error != 0)
        {
            throw ConnectFailedException(__FILE__, __LINE__, error);
        }

        if (!_connectState->connected.load())
        {
            throw ConnectFailedException(__FILE__, __LINE__, 0);
        }

        _state = StateConnected;
        return SocketOperationNone;
    }

    // StateConnected or StateNotConnected — nothing to do.
    return SocketOperationNone;
}

SocketOperation
IceInternal::NetworkFrameworkUdpTransceiver::closing(bool, exception_ptr)
{
    // Nothing to do for UDP.
    return SocketOperationNone;
}

void
IceInternal::NetworkFrameworkUdpTransceiver::close()
{
    if (_connection)
    {
        nw_connection_cancel(_connection);
    }
    if (_listener)
    {
        nw_listener_cancel(_listener);
    }
    if (_mcastReadSource)
    {
        dispatch_source_cancel(_mcastReadSource);
    }

    // Multicast BSD socket mode (client send-only) — signal read completion to unblock the thread pool.
    // The pending startRead() never started a real async operation; this completion
    // allows the thread pool to call finishRead() which throws ConnectionLostException.
    if (!_incoming && _mcastFd != INVALID_SOCKET && !_connection)
    {
        _nativeInfo->completed(SocketOperationRead);
    }

    // Cancel all peer connections and release queued datagrams.
    if (_serverState)
    {
        lock_guard lock(_serverState->mutex);
        for (auto& conn : _serverState->connections)
        {
            nw_connection_cancel(conn);
        }
        // If startRead() is waiting, signal to unblock the thread pool.
        if (_serverState->readWaiting)
        {
            _serverState->readWaiting = false;
            _nativeInfo->completed(SocketOperationRead);
        }
    }
}

EndpointIPtr
IceInternal::NetworkFrameworkUdpTransceiver::bind()
{
    assert(_incoming);

    //
    // Check if the host is a multicast address.
    //
    Address addr = getNumericAddress(_host);
    setPort(addr, _port);
    bool multicast = isAddressValid(addr) && isMulticast(addr);

    auto serverState = _serverState;
    NativeInfoPtr nativeInfo = _nativeInfo;

    if (multicast)
    {
        //
        // Multicast: use a BSD socket with dispatch_source.
        //
        // Network.framework's nw_connection_group_t does not support SO_REUSEADDR/SO_REUSEPORT
        // properly — multiple processes cannot join the same multicast group. Fall back to a
        // traditional BSD socket for multicast receive.
        //

        _mcastFd = createSocket(true, addr); // true = UDP
        setReuseAddress(_mcastFd, true);
        setBlock(_mcastFd, false);

        Address mcastAddr = addr;
#ifdef _WIN32
        const_cast<Address&>(addr) = getAddressForServer("", _port, getProtocolSupport(addr), false, false);
#endif
        addr = doBind(_mcastFd, addr, _mcastInterface);
        if (getPort(mcastAddr) == 0)
        {
            setPort(mcastAddr, getPort(addr));
        }
        setMcastGroup(_mcastFd, mcastAddr, _mcastInterface);

        _port = static_cast<uint16_t>(getPort(addr));

        // Create a dispatch source to receive multicast datagrams.
        _mcastReadSource = dispatch_source_create(
            DISPATCH_SOURCE_TYPE_READ,
            static_cast<uintptr_t>(_mcastFd),
            0,
            _dispatchQueue);

        // The handlers must not touch this transceiver: they can run after close() cancelled the source, while the
        // transceiver is being destroyed. They only use state they own or share (the socket, the server state and
        // the native info), and the socket is closed from the cancellation handler, which runs once the event
        // handler can no longer be invoked, as required by dispatch_source_set_cancel_handler.
        const SOCKET fd = _mcastFd;
        const int maxPacketSize = _maxPacketSize;
        dispatch_source_set_event_handler(_mcastReadSource, ^{
            // Read all available datagrams.
            while (true)
            {
                vector<byte> data(static_cast<size_t>(maxPacketSize));
                ssize_t ret = ::recvfrom(fd, data.data(), data.size(), 0, nullptr, nullptr);
                if (ret <= 0)
                {
                    break;
                }
                data.resize(static_cast<size_t>(ret));

                lock_guard lock(serverState->mutex);
                serverState->received.push_back({std::move(data), nullptr});
                if (serverState->readWaiting)
                {
                    serverState->readWaiting = false;
                    nativeInfo->completed(SocketOperationRead);
                }
            }
        });
        dispatch_source_set_cancel_handler(_mcastReadSource, ^{ closeSocketNoThrow(fd); });

        dispatch_resume(_mcastReadSource);

        // No need to wait — BSD socket bind is synchronous.
    }
    else
    {
        //
        // Unicast: use nw_listener_t for per-peer connections.
        //

        nw_parameters_t parameters =
            nw_parameters_create_secure_udp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION);
        if (!parameters)
        {
            throw SocketException(__FILE__, __LINE__, 0);
        }

        nw_parameters_set_reuse_local_address(parameters, true);

        // Enable IP fragmentation for large UDP datagrams, matching BSD socket behavior.
        nw_protocol_stack_t protocolStack = nw_parameters_copy_default_protocol_stack(parameters);
        nw_protocol_options_t ipOptions = nw_protocol_stack_copy_internet_protocol(protocolStack);
        nw_ip_options_set_disable_fragmentation(ipOptions, false);
        nw_release(ipOptions);
        nw_release(protocolStack);

        struct ReadyState
        {
            mutex mutex;
            condition_variable cv;
            bool signaled{false};
            int error{0};
        };
        auto readyState = make_shared<ReadyState>();

        // If a specific local address is requested, set it on the parameters.
        if (!_host.empty() && _host != "0.0.0.0" && _host != "::" && _host != "*")
        {
            nw_endpoint_t localEndpoint = nw_endpoint_create_host(_host.c_str(), "0");
            if (localEndpoint)
            {
                nw_parameters_set_local_endpoint(parameters, localEndpoint);
                nw_release(localEndpoint);
            }
        }

        // Create the listener with a specific port or an OS-assigned port.
        if (_port > 0)
        {
            _listener = nw_listener_create_with_port(to_string(_port).c_str(), parameters);
        }
        else
        {
            _listener = nw_listener_create(parameters);
        }
        nw_release(parameters);

        if (!_listener)
        {
            throw SocketException(__FILE__, __LINE__, 0);
        }

        nw_listener_set_state_changed_handler(_listener, ^(nw_listener_state_t state, nw_error_t error) {
            if (state == nw_listener_state_ready || state == nw_listener_state_failed)
            {
                lock_guard lock(readyState->mutex);
                readyState->signaled = true;
                if (state == nw_listener_state_failed)
                {
                    readyState->error = error ? nw_error_get_error_code(error) : EADDRINUSE;
                }
                readyState->cv.notify_one();
            }
        });

        // Set new connection handler for incoming UDP peers.
        dispatch_queue_t dispatchQueue = _dispatchQueue;
        nw_listener_set_new_connection_handler(_listener, ^(nw_connection_t peerConnection) {
            nw_connection_set_state_changed_handler(peerConnection, ^(nw_connection_state_t state, nw_error_t) {
                if (state == nw_connection_state_failed || state == nw_connection_state_cancelled)
                {
                    lock_guard lock(serverState->mutex);
                    auto& conns = serverState->connections;
                    auto it = find(conns.begin(), conns.end(), peerConnection);
                    if (it != conns.end())
                    {
                        nw_release(*it);
                        conns.erase(it);
                    }
                }
            });

            nw_connection_set_queue(peerConnection, dispatchQueue);
            nw_connection_start(peerConnection);

            {
                // The connections vector owns one reference to each peer, released when the peer fails or is
                // cancelled (see the state changed handler above).
                lock_guard lock(serverState->mutex);
                serverState->connections.push_back(peerConnection);
                nw_retain(peerConnection);
            }

            startPeerReceive(peerConnection, serverState, nativeInfo);
        });

        // Start the listener.
        nw_listener_set_queue(_listener, _dispatchQueue);
        nw_listener_start(_listener);

        // Wait for the listener to become ready.
        {
            unique_lock lock(readyState->mutex);
            readyState->cv.wait(lock, [&] { return readyState->signaled; });
        }

        if (readyState->error != 0)
        {
            throw SocketException(__FILE__, __LINE__, readyState->error);
        }

        nw_listener_set_state_changed_handler(_listener, nullptr);
        _port = nw_listener_get_port(_listener);
    }

    _endpoint = _endpoint->endpoint(_port);
    return _endpoint;
}

SocketOperation
IceInternal::NetworkFrameworkUdpTransceiver::write(Buffer& buf)
{
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    return SocketOperationWrite;
}

SocketOperation
IceInternal::NetworkFrameworkUdpTransceiver::read(Buffer& buf)
{
    // Multicast BSD socket mode (client send-only) — no data will be received on the send-only socket.
    if (!_incoming && _mcastFd != INVALID_SOCKET && !_connection)
    {
        return SocketOperationNone;
    }
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    return SocketOperationRead;
}

bool
IceInternal::NetworkFrameworkUdpTransceiver::startWrite(Buffer& buf)
{
    if (_state == StateConnectPending)
    {
        // The connection is being established by Network.framework.
        return false;
    }

    assert(buf.i == buf.b.begin());

    // The caller is supposed to check the send size before by calling checkSendSize.
    assert(min(_maxPacketSize, _sndSize - _udpOverhead) >= static_cast<int>(buf.b.size()));

    size_t length = buf.b.size();

    // Reset write state before starting the operation.
    {
        lock_guard lock(_writeState->mutex);
        _writeState->error = 0;
    }

    auto writeState = _writeState;
    NativeInfoPtr nativeInfo = _nativeInfo;

    // Multicast BSD socket mode (client send-only) — send via the connected BSD socket.
    if (!_incoming && _mcastFd != INVALID_SOCKET && _state == StateConnected && !_connection)
    {
        ssize_t ret = ::send(_mcastFd, &*buf.i, buf.b.size(), 0);
        {
            lock_guard lock(writeState->mutex);
            if (ret < 0)
            {
                writeState->error = errno;
            }
        }
        nativeInfo->completed(SocketOperationWrite);
        return true;
    }

    dispatch_data_t data = dispatch_data_create(&*buf.i, length, _dispatchQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    // Determine which connection to send on.
    nw_connection_t sendConnection;
    if (_state == StateConnected)
    {
        sendConnection = _connection;
    }
    else
    {
        // Server mode — send reply on the peer that sent the last datagram.
        sendConnection = _currentPeer;
        if (!sendConnection)
        {
            dispatch_release(data);
            throw SocketException(__FILE__, __LINE__, 0);
        }
    }

    nw_connection_send(
        sendConnection,
        data,
        NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
        true, // is_complete — this is a complete UDP datagram
        ^(nw_error_t error) {
            lock_guard lock(writeState->mutex);
            if (error)
            {
                writeState->error = nw_error_get_error_code(error);
            }
            nativeInfo->completed(SocketOperationWrite);
        });

    dispatch_release(data);

    return true;
}

void
IceInternal::NetworkFrameworkUdpTransceiver::finishWrite(Buffer& buf)
{
    if (_state < StateConnected)
    {
        return;
    }

    lock_guard lock(_writeState->mutex);

    if (_writeState->error != 0)
    {
        if (_writeState->error == EMSGSIZE)
        {
            // Datagram too large for the network path. Silently drop it — UDP is fire-and-forget,
            // and this matches the behavior of a datagram lost in transit. Keeping the connection
            // alive is important for subsequent operations (e.g. bi-dir).
            buf.i = buf.b.end();
            return;
        }
        throw ConnectionLostException(__FILE__, __LINE__, _writeState->error);
    }

    // UDP sends the entire datagram atomically.
    buf.i = buf.b.end();
}

void
IceInternal::NetworkFrameworkUdpTransceiver::startRead(Buffer& buf)
{
    // Resize buffer to max packet size for UDP.
    const int packetSize = min(_maxPacketSize, _rcvSize - _udpOverhead);
    buf.b.resize(static_cast<size_t>(packetSize));
    buf.i = buf.b.begin();

    // Multicast BSD socket mode (client send-only) — no reads expected.
    // Don't start any async read. The read stays "started but not completed" until close()
    // signals completion to unblock the thread pool.
    if (!_incoming && _mcastFd != INVALID_SOCKET && !_connection)
    {
        return;
    }

    if (_state == StateNotConnected)
    {
        // Server mode — check if a datagram is already queued.
        lock_guard lock(_serverState->mutex);
        if (!_serverState->received.empty())
        {
            // Data is already available — signal immediately.
            _nativeInfo->completed(SocketOperationRead);
        }
        else
        {
            // No data yet — the receive callback will signal when data arrives.
            _serverState->readWaiting = true;
        }
    }
    else
    {
        // Client mode — start a receive on the connected UDP socket.
        assert(_state == StateConnected);

        {
            lock_guard lock(_readState->mutex);
            _readState->data.clear();
            _readState->error = 0;
        }

        auto readState = _readState;
        NativeInfoPtr nativeInfo = _nativeInfo;

        nw_connection_receive(
            _connection,
            1,     // minimum bytes
            65535, // maximum bytes (max UDP datagram)
            ^(dispatch_data_t content, nw_content_context_t, bool, nw_error_t error) {
                lock_guard lock(readState->mutex);
                if (error)
                {
                    readState->error = nw_error_get_error_code(error);
                }
                else if (content)
                {
                    dispatch_data_apply(content, ^bool(dispatch_data_t, size_t, const void* buffer, size_t size) {
                        auto* bytes = static_cast<const byte*>(buffer);
                        readState->data.insert(readState->data.end(), bytes, bytes + size);
                        return true;
                    });
                }
                else
                {
                    // No content and no error — EOF.
                    readState->error = ECONNRESET;
                }
                nativeInfo->completed(SocketOperationRead);
            });
    }
}

void
IceInternal::NetworkFrameworkUdpTransceiver::finishRead(Buffer& buf)
{
    // Multicast BSD socket mode (client send-only) — finishRead is only called during close() when the
    // pending read completion fires. Throw to indicate the connection is done.
    if (!_incoming && _mcastFd != INVALID_SOCKET && !_connection)
    {
        throw ConnectionLostException(__FILE__, __LINE__);
    }

    if (_state == StateNotConnected)
    {
        // Server mode — dequeue the next datagram.
        lock_guard lock(_serverState->mutex);

        if (_serverState->received.empty())
        {
            throw ConnectionLostException(__FILE__, __LINE__);
        }

        auto datagram = std::move(_serverState->received.front());
        _serverState->received.pop_front();

        // Store the source peer for reply writes (may be nullptr for multicast).
        if (_currentPeer)
        {
            nw_release(_currentPeer);
            _currentPeer = nullptr;
        }
        if (datagram.source)
        {
            _currentPeer = datagram.source; // Takes ownership of the retained reference.
        }

        // Copy datagram data into the buffer and resize to actual size.
        size_t count = datagram.data.size();
        if (count > buf.b.size())
        {
            count = buf.b.size();
        }
        memcpy(&buf.b[0], datagram.data.data(), count);
        buf.b.resize(count);
        buf.i = buf.b.end();
    }
    else
    {
        // Client mode — get data from the read state.
        lock_guard lock(_readState->mutex);

        if (_readState->error != 0)
        {
            throw ConnectionLostException(__FILE__, __LINE__, _readState->error);
        }

        if (_readState->data.empty())
        {
            throw ConnectionLostException(__FILE__, __LINE__);
        }

        size_t count = _readState->data.size();
        if (count > buf.b.size())
        {
            count = buf.b.size();
        }
        memcpy(&buf.b[0], _readState->data.data(), count);
        buf.b.resize(count);
        buf.i = buf.b.end();
    }
}

string
IceInternal::NetworkFrameworkUdpTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::NetworkFrameworkUdpTransceiver::toString() const
{
    if (_incoming)
    {
        ostringstream s;
        s << "local address = " << _host << ":" << _port;
        return s.str();
    }
    else if (_connection)
    {
        nw_endpoint_t remoteEndpoint = nw_connection_copy_endpoint(_connection);
        if (remoteEndpoint)
        {
            string desc = "local address = <nw> remote address = " + nwEndpointToString(remoteEndpoint);
            nw_release(remoteEndpoint);
            return desc;
        }
    }
    return "<nw udp>";
}

string
IceInternal::NetworkFrameworkUdpTransceiver::toDetailedString() const
{
    return toString();
}

ConnectionInfoPtr
IceInternal::NetworkFrameworkUdpTransceiver::getInfo(bool incoming, string adapterName, string connectionId) const
{
    string localAddress;
    int localPort = -1;
    string remoteAddress;
    int remotePort = -1;
    string mcastAddress;
    int mcastPort = -1;

    if (_incoming)
    {
        localAddress = _host;
        localPort = _port;
    }
    else if (_connection)
    {
        nw_path_t path = nw_connection_copy_current_path(_connection);
        if (path)
        {
            nw_endpoint_t localEndpoint = nw_path_copy_effective_local_endpoint(path);
            if (localEndpoint)
            {
                localAddress = nw_endpoint_get_hostname(localEndpoint);
                localPort = nw_endpoint_get_port(localEndpoint);
                nw_release(localEndpoint);
            }
            nw_endpoint_t remoteEndpoint = nw_path_copy_effective_remote_endpoint(path);
            if (remoteEndpoint)
            {
                remoteAddress = nw_endpoint_get_hostname(remoteEndpoint);
                remotePort = nw_endpoint_get_port(remoteEndpoint);
                nw_release(remoteEndpoint);
            }
            nw_release(path);
        }
    }

    return make_shared<UDPConnectionInfo>(
        incoming,
        std::move(adapterName),
        std::move(connectionId),
        std::move(localAddress),
        localPort,
        std::move(remoteAddress),
        remotePort,
        std::move(mcastAddress),
        mcastPort,
        _rcvSize,
        _sndSize);
}

void
IceInternal::NetworkFrameworkUdpTransceiver::checkSendSize(const Buffer& buf)
{
    // The maximum packetSize is either the maximum allowable UDP packet size, or
    // the UDP send buffer size (whichever is smaller).
    const int packetSize = min(_maxPacketSize, _sndSize - _udpOverhead);
    if (packetSize < static_cast<int>(buf.b.size()))
    {
        throw DatagramLimitException(__FILE__, __LINE__);
    }
}

void
IceInternal::NetworkFrameworkUdpTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    // Network.framework manages buffers internally. Just store the requested sizes
    // for checkSendSize calculations.
    if (rcvSize > 0)
    {
        _rcvSize = rcvSize;
    }
    if (sndSize > 0)
    {
        _sndSize = sndSize;
    }
}

int
IceInternal::NetworkFrameworkUdpTransceiver::effectivePort() const
{
    return _port;
}

//
// The maximum IP datagram size is 65535. Subtract 20 bytes for the IP header and 8 bytes for the UDP header
// to get the maximum payload.
//
const int IceInternal::NetworkFrameworkUdpTransceiver::_udpOverhead = 20 + 8;
const int IceInternal::NetworkFrameworkUdpTransceiver::_maxPacketSize = 65535 - _udpOverhead;

#endif
