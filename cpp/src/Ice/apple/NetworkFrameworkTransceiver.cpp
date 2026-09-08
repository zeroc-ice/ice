// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/Buffer.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include "../NetworkProxy.h"
#    include "../ProtocolInstance.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <Security/Security.h>

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
}

IceInternal::NetworkFrameworkTransceiver::NetworkFrameworkTransceiver(
    ProtocolInstancePtr instance,
    nw_connection_t connection,
    bool secure,
    NetworkProxyPtr proxy,
    const Address& addr)
    : _instance(std::move(instance)),
      _proxy(std::move(proxy)),
      _addr(addr),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _connection(connection),
      _state(StateNeedsConnect),
      _connectState(make_shared<ConnectState>()),
      _readState(make_shared<ReadState>()),
      _writeState(make_shared<WriteState>()),
      _secure(secure)
{
    assert(_connection);
    nw_retain(_connection);

    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-connection", DISPATCH_QUEUE_SERIAL);
}

IceInternal::NetworkFrameworkTransceiver::~NetworkFrameworkTransceiver()
{
    if (_connection)
    {
        nw_release(_connection);
    }
    if (_dispatchQueue)
    {
        dispatch_release(_dispatchQueue);
    }
}

NativeInfoPtr
IceInternal::NetworkFrameworkTransceiver::getNativeInfo()
{
    return _nativeInfo;
}

SocketOperation
IceInternal::NetworkFrameworkTransceiver::initialize(Buffer& readBuffer, Buffer& writeBuffer)
{
    if (_state == StateNeedsConnect)
    {
        _state = StateConnectPending;

        //
        // Set up the state change handler. Capture shared state and NativeInfo by shared_ptr
        // so the block is safe even if the transceiver is destroyed before it fires.
        //
        auto connectState = _connectState;
        NativeInfoPtr nativeInfo = _nativeInfo;
        bool secure = _secure;
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
                    // Only signal connect completion if the connection hasn't been established yet.
                    // An already-connected connection can transition to failed (e.g., peer reset),
                    // but the read/write callbacks handle that — a spurious Connect completion here
                    // would be processed after the handler is destroyed, causing bad_weak_ptr.
                    if (!connectState->connected.load())
                    {
                        if (error && nw_error_get_error_domain(error) == nw_error_domain_tls)
                        {
                            connectState->tlsError.store(true);
                        }
                        connectState->error.store(error ? nw_error_get_error_code(error) : ECONNREFUSED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_waiting:
                {
                    // The connection is waiting for network conditions to change. Treat this as a
                    // connection failure — Ice has its own retry logic and should not rely on NF's
                    // built-in reconnection behavior. Only signal if not yet connected.
                    if (!connectState->connected.load())
                    {
                        if (error && nw_error_get_error_domain(error) == nw_error_domain_tls)
                        {
                            connectState->tlsError.store(true);
                        }
                        connectState->error.store(error ? nw_error_get_error_code(error) : ECONNREFUSED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_cancelled:
                {
                    // Signal connect completion if the connection was cancelled before it
                    // was established. This ensures the thread pool can process the pending
                    // connect operation and clean up.
                    if (!connectState->connected.load() && connectState->error.load() == 0)
                    {
                        connectState->error.store(ECANCELED);
                        nativeInfo->completed(SocketOperationConnect);
                    }
                    break;
                }
                case nw_connection_state_preparing:
                    break;
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
        //
        // Check if the connection succeeded or failed. The state handler has already
        // fired and set the shared connect state atomics.
        //
        int error = _connectState->error.load();
        if (error != 0)
        {
            if (_connectState->tlsError.load())
            {
                // If _localVerifyRejected is set, our local verify block called complete(false),
                // meaning we rejected the peer's certificate → SecurityException.
                // If _localVerifyRejected is not set or is false, the peer rejected us (e.g.,
                // server rejected our client cert) → ConnectionLostException.
                if (!_localVerifyRejected || _localVerifyRejected->load())
                {
                    throw Ice::SecurityException(
                        __FILE__,
                        __LINE__,
                        "SSL transport: TLS handshake failed with error " + to_string(error));
                }
                else
                {
                    throw ConnectionLostException(
                        __FILE__,
                        __LINE__,
                        "connection lost (TLS error " + to_string(error) + ")");
                }
            }
            throw ConnectFailedException(__FILE__, __LINE__, error);
        }

        if (!_connectState->connected.load())
        {
            // State handler hasn't fired yet — shouldn't normally happen
            // since we only get here via a completion.
            throw ConnectFailedException(__FILE__, __LINE__, 0);
        }

        _state = (_proxy && !_secure) ? StateProxyWrite : StateConnected;
        _desc = describe();

        if (_secure && _peerVerifier)
        {
            // TLS validated the certificate the peer presented, if any. When the peer presented none (for example a
            // server that does not require client certificates), the engine's trust rules still have to run; they
            // reject the connection by throwing. Secure connections never use the in-band proxy handshake below.
            auto info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(getInfo(_incoming, _adapterName, ""));
            assert(info);
            if (!info->peerCertificate)
            {
                _peerVerifier(info);
            }
        }
    }

    //
    // Proxy handshake, driven by the connection's read and write buffers like StreamSocket::connect: the connection
    // request is written, the response read (finishWrite/finishRead advance the state), and the proxy finally
    // consumes any extra data before Ice starts using the connection.
    //
    if (_state == StateProxyWrite)
    {
        _proxy->beginWrite(_addr, writeBuffer);
        return SocketOperationWrite;
    }
    else if (_state == StateProxyRead)
    {
        _proxy->beginRead(readBuffer);
        return SocketOperationRead;
    }
    else if (_state == StateProxyConnected)
    {
        _proxy->finish(readBuffer, writeBuffer);

        readBuffer.b.clear();
        readBuffer.i = readBuffer.b.end();

        writeBuffer.b.clear();
        writeBuffer.i = writeBuffer.b.end();

        _state = StateConnected;
    }

    assert(_state == StateConnected);
    return SocketOperationNone;
}

SocketOperation
IceInternal::NetworkFrameworkTransceiver::closing(bool initiator, exception_ptr)
{
    if (initiator)
    {
        // When the initiator sends a close-connection message, it waits for the peer to close the TCP
        // connection. Return SocketOperationRead so the connection engine starts a read to wait for the
        // peer's close. If the peer doesn't close within the close timeout, the connection engine will
        // call close() which cancels the NF connection and forces the pending receive to complete.
        return SocketOperationRead;
    }
    return SocketOperationNone;
}

void
IceInternal::NetworkFrameworkTransceiver::close()
{
    if (_connection)
    {
        nw_connection_cancel(_connection);
    }
}

SocketOperation
IceInternal::NetworkFrameworkTransceiver::write(Buffer& buf)
{
    // Network.framework doesn't support synchronous writes. Return SocketOperationWrite to
    // trigger the async write path (startWrite/finishWrite) when there's still data to send.
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    return SocketOperationWrite;
}

SocketOperation
IceInternal::NetworkFrameworkTransceiver::read(Buffer& buf)
{
    // Network.framework doesn't support synchronous reads. Return SocketOperationRead to
    // trigger the async read path (startRead/finishRead) when more data is needed.
    if (buf.i == buf.b.end())
    {
        return SocketOperationNone;
    }
    return SocketOperationRead;
}

bool
IceInternal::NetworkFrameworkTransceiver::startWrite(Buffer& buf)
{
    if (_state == StateConnectPending)
    {
        // The connection is being established by Network.framework.
        // startAsync is called with SocketOperationConnect but the connect
        // is already in progress — nothing to do.
        return false;
    }

    assert(_state == StateProxyWrite || _state == StateConnected);

    assert(buf.b.end() - buf.i > 0);
    size_t length = static_cast<size_t>(buf.b.end() - buf.i);

    //
    // Create dispatch_data_t from the buffer. dispatch_data_create with
    // DISPATCH_DATA_DESTRUCTOR_DEFAULT copies the data.
    //
    dispatch_data_t data = dispatch_data_create(&*buf.i, length, _dispatchQueue, DISPATCH_DATA_DESTRUCTOR_DEFAULT);

    // Reset write state before starting the operation.
    {
        lock_guard lock(_writeState->mutex);
        _writeState->count = length;
        _writeState->error = 0;
    }

    auto writeState = _writeState;
    NativeInfoPtr nativeInfo = _nativeInfo;

    nw_connection_send(
        _connection,
        data,
        NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
        true, // is_complete — this completes the "message" (just means send all data)
        ^(nw_error_t error) {
            lock_guard lock(writeState->mutex);
            if (error)
            {
                writeState->error = nw_error_get_error_code(error);
                writeState->count = 0;
            }
            // On success, count remains set to length (all data was sent).
            nativeInfo->completed(SocketOperationWrite);
        });

    dispatch_release(data);

    return true;
}

void
IceInternal::NetworkFrameworkTransceiver::finishWrite(Buffer& buf)
{
    if (_state < StateProxyWrite)
    {
        return;
    }

    lock_guard lock(_writeState->mutex);

    if (_writeState->error != 0)
    {
        throw ConnectionLostException(__FILE__, __LINE__, _writeState->error);
    }

    buf.i += _writeState->count;

    if (_state == StateProxyWrite)
    {
        _state = toState(_proxy->endWrite(buf));
    }
}

void
IceInternal::NetworkFrameworkTransceiver::startRead(Buffer& buf)
{
    assert(_state == StateProxyRead || _state == StateConnected);
    assert(buf.b.end() - buf.i > 0);
    size_t length = static_cast<size_t>(buf.b.end() - buf.i);

    // Reset read state before starting the operation.
    {
        lock_guard lock(_readState->mutex);
        _readState->data.clear();
        _readState->error = 0;
    }

    auto readState = _readState;
    NativeInfoPtr nativeInfo = _nativeInfo;

    nw_connection_receive(
        _connection,
        1,                                  // minimum bytes
        static_cast<uint32_t>(length),      // maximum bytes
        ^(dispatch_data_t content, nw_content_context_t, bool, nw_error_t error) {
            lock_guard lock(readState->mutex);
            if (error)
            {
                readState->error = nw_error_get_error_code(error);
            }
            else if (content)
            {
                // Extract the received data from dispatch_data_t.
                dispatch_data_apply(content, ^bool(dispatch_data_t, size_t, const void* buffer, size_t size) {
                    auto* bytes = static_cast<const std::byte*>(buffer);
                    readState->data.insert(readState->data.end(), bytes, bytes + size);
                    return true;
                });
            }
            else
            {
                // No content and no error means EOF — the peer closed the connection.
                readState->error = ECONNRESET;
            }
            nativeInfo->completed(SocketOperationRead);
        });
}

void
IceInternal::NetworkFrameworkTransceiver::finishRead(Buffer& buf)
{
    lock_guard lock(_readState->mutex);

    if (_readState->error != 0)
    {
        throw ConnectionLostException(__FILE__, __LINE__, _readState->error);
    }

    if (_readState->data.empty())
    {
        throw ConnectionLostException(__FILE__, __LINE__);
    }

    size_t available = _readState->data.size();
    size_t remaining = static_cast<size_t>(buf.b.end() - buf.i);
    size_t count = min(available, remaining);

    memcpy(&*buf.i, _readState->data.data(), count);
    buf.i += count;

    if (_state == StateProxyRead)
    {
        _state = toState(_proxy->endRead(buf));
    }
}

string
IceInternal::NetworkFrameworkTransceiver::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::NetworkFrameworkTransceiver::toString() const
{
    return _desc;
}

string
IceInternal::NetworkFrameworkTransceiver::toDetailedString() const
{
    return toString();
}

ConnectionInfoPtr
IceInternal::NetworkFrameworkTransceiver::getInfo(bool incoming, string adapterName, string connectionId) const
{
    // For Network.framework, we don't have a socket FD to query. Build connection info from
    // the nw_connection endpoint.
    string localAddress;
    int localPort = -1;
    string remoteAddress;
    int remotePort = -1;

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

    if (_proxy)
    {
        // Like the socket transports, report the proxy as the remote peer: it is the peer of the transport
        // connection. For secure connections Network.framework reports the destination as the effective remote
        // endpoint even though the connection goes through the proxy.
        addrToAddressAndPort(_proxy->getAddress(), remoteAddress, remotePort);
    }

    auto tcpInfo = make_shared<TCPConnectionInfo>(
        incoming,
        std::move(adapterName),
        std::move(connectionId),
        std::move(localAddress),
        localPort,
        std::move(remoteAddress),
        remotePort,
        _rcvSize,
        _sndSize);

    if (_secure)
    {
        // Extract the peer certificate from the TLS protocol metadata.
        __block SecCertificateRef peerCertificate = nullptr;

        nw_protocol_definition_t tlsDefinition = nw_protocol_copy_tls_definition();
        nw_protocol_metadata_t tlsMetadata = nw_connection_copy_protocol_metadata(_connection, tlsDefinition);
        nw_release(tlsDefinition);

        if (tlsMetadata)
        {
            sec_protocol_metadata_t secMetadata = nw_tls_copy_sec_protocol_metadata(tlsMetadata);
            if (secMetadata)
            {
                sec_protocol_metadata_access_peer_certificate_chain(
                    secMetadata,
                    ^(sec_certificate_t cert) {
                        if (!peerCertificate)
                        {
                            SecCertificateRef secCert = sec_certificate_copy_ref(cert);
                            if (secCert)
                            {
                                peerCertificate = secCert; // Retained — ownership transferred to ConnectionInfo
                            }
                        }
                    });
                sec_release(secMetadata);
            }
            nw_release(tlsMetadata);
        }

        return make_shared<Ice::SSL::ConnectionInfo>(tcpInfo, peerCertificate);
    }

    return tcpInfo;
}

void
IceInternal::NetworkFrameworkTransceiver::checkSendSize(const Buffer&)
{
}

string
IceInternal::NetworkFrameworkTransceiver::describe() const
{
    // Same format as fdToString for the BSD socket transports: with a proxy, the effective remote endpoint is
    // the proxy and the destination address is listed separately.
    string localAddress = "<not available>";
    string remoteAddress = "<not available>";
    nw_path_t path = nw_connection_copy_current_path(_connection);
    if (path)
    {
        nw_endpoint_t localEndpoint = nw_path_copy_effective_local_endpoint(path);
        if (localEndpoint)
        {
            localAddress = nwEndpointToString(localEndpoint);
            nw_release(localEndpoint);
        }
        nw_endpoint_t remoteEndpoint = nw_path_copy_effective_remote_endpoint(path);
        if (remoteEndpoint)
        {
            remoteAddress = nwEndpointToString(remoteEndpoint);
            nw_release(remoteEndpoint);
        }
        nw_release(path);
    }

    ostringstream os;
    os << "local address = " << localAddress;
    if (_proxy)
    {
        os << "\n" << _proxy->getName() << " proxy address = " << addrToString(_proxy->getAddress());
        os << "\nremote address = " << addrToString(_addr);
    }
    else
    {
        os << " remote address = " << remoteAddress;
    }
    return os.str();
}

IceInternal::NetworkFrameworkTransceiver::State
IceInternal::NetworkFrameworkTransceiver::toState(SocketOperation operation)
{
    switch (operation)
    {
        case SocketOperationRead:
            return StateProxyRead;
        case SocketOperationWrite:
            return StateProxyWrite;
        default:
            return StateProxyConnected;
    }
}

void
IceInternal::NetworkFrameworkTransceiver::setBufferSize(int rcvSize, int sndSize)
{
    _rcvSize = rcvSize;
    _sndSize = sndSize;
}

#endif
