// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../NetworkProxy.h"
#    include "../ProtocolInstance.h"
#    include "Ice/Buffer.h"
#    include "Ice/Connection.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/SSL/ConnectionInfo.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <Security/Security.h>

#    include <cerrno>
#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

using AsyncState = IceInternal::NetworkFrameworkTransceiver::AsyncState;

namespace
{
    string nwEndpointToString(nw_endpoint_t endpoint)
    {
        if (!endpoint)
        {
            return "<not connected>";
        }
        string host = nw_endpoint_get_hostname(endpoint);
        uint16_t port = nw_endpoint_get_port(endpoint);
        return host + ":" + to_string(port);
    }

    // The functions below are called with the state mutex held.

    // Completes the operation with its result and posts its completion to the selector, unless the operation was
    // already completed: the selector receives exactly one completion per started operation.
    template<typename Result>
    void complete(AsyncState& state, AsyncOperation<Result>& operation, SocketOperation kind, Result result)
    {
        if (operation.complete(std::move(result)))
        {
            state.nativeInfo->completed(kind);
        }
    }

    // Cancellation is the terminal result of the pending operations: the thread pool waits for their completions
    // before it finishes the connection. The blocks Network.framework invokes for these operations afterwards
    // complete nothing.
    void cancel(AsyncState& state)
    {
        complete(state, state.connect, SocketOperationConnect, AsyncState::ConnectResult{ECANCELED, false});
        complete(state, state.read, SocketOperationRead, AsyncState::ReadResult{{}, ECANCELED});
        complete(state, state.write, SocketOperationWrite, AsyncState::WriteResult{0, ECANCELED});
    }

    AsyncState::ConnectResult connectFailure(nw_error_t error)
    {
        if (!error)
        {
            return {ECONNREFUSED, false};
        }
        return {nw_error_get_error_code(error), nw_error_get_error_domain(error) == nw_error_domain_tls};
    }
}

IceInternal::NetworkFrameworkTransceiver::AsyncState::AsyncState(NativeInfoPtr info) : nativeInfo(std::move(info)) {}

IceInternal::NetworkFrameworkTransceiver::NetworkFrameworkTransceiver(
    ProtocolInstancePtr instance,
    NetworkRef<nw_connection_t> connection,
    bool secure,
    NetworkProxyPtr proxy,
    const Address& addr)
    : _instance(std::move(instance)),
      _proxy(std::move(proxy)),
      _addr(addr),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _connection(std::move(connection)),
      _dispatchQueue(DispatchRef<dispatch_queue_t>::adopt(
          dispatch_queue_create("com.zeroc.ice.nw-connection", DISPATCH_QUEUE_SERIAL))),
      _async(make_shared<AsyncState>(_nativeInfo)),
      _state(StateNeedsConnect),
      _secure(secure)
{
    assert(_connection);
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

        // The connection reports several state transitions, of which the first terminal one completes the connect
        // operation: ready, or the failure. Later transitions leave the completed operation unchanged, for example
        // a connection that fails or is cancelled once established (the read and write operations report that), or
        // a connection that becomes ready after it reported that it was waiting.
        {
            lock_guard lock(_async->mutex);
            _async->connect.start();
        }
        auto async = _async; // The block captures a copy: it can run after the transceiver is destroyed.
        nw_connection_set_state_changed_handler(_connection.get(), ^(nw_connection_state_t state, nw_error_t error) {
          lock_guard lock(async->mutex);
          switch (state)
          {
              case nw_connection_state_ready:
                  complete(*async, async->connect, SocketOperationConnect, AsyncState::ConnectResult{});
                  break;
              case nw_connection_state_waiting:
                  // Waiting for the network conditions to change: the connect fails instead, Ice has its own retry
                  // logic and does not rely on Network.framework's.
                  [[fallthrough]];
              case nw_connection_state_failed:
                  complete(*async, async->connect, SocketOperationConnect, connectFailure(error));
                  break;
              case nw_connection_state_cancelled:
                  // close() completed the pending operations already; the transition completes nothing then.
                  complete(*async, async->connect, SocketOperationConnect, AsyncState::ConnectResult{ECANCELED, false});
                  break;
              case nw_connection_state_preparing:
              case nw_connection_state_invalid:
                  break;
          }
        });

        nw_connection_set_queue(_connection.get(), _dispatchQueue.get());
        nw_connection_start(_connection.get());
        return SocketOperationConnect;
    }
    else if (_state == StateConnectPending)
    {
        AsyncState::ConnectResult result;
        {
            lock_guard lock(_async->mutex);
            result = _async->connect.consume();
        }

        if (result.error != 0)
        {
            if (result.tlsError)
            {
                // The verification of this connection rejected the peer (SecurityException), or the peer rejected
                // this connection, for example a server that rejected the client certificate (ConnectionLost).
                if (!_localVerifyRejected || _localVerifyRejected->load())
                {
                    throw Ice::SecurityException(
                        __FILE__,
                        __LINE__,
                        "SSL transport: TLS handshake failed with error " + to_string(result.error));
                }
                else
                {
                    throw ConnectionLostException(
                        __FILE__,
                        __LINE__,
                        "connection lost (TLS error " + to_string(result.error) + ")");
                }
            }
            // A reset by the peer (for example a server that accepts then closes an over-limit connection)
            // reaches Network.framework as a failed connection rather than a lost one. Classify it the way the
            // BSD-socket transports do, so that it reports ConnectionLostException rather than ConnectFailedException.
            errno = result.error;
            if (connectionLost())
            {
                throw ConnectionLostException(__FILE__, __LINE__, result.error);
            }
            throw ConnectFailedException(__FILE__, __LINE__, result.error);
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
    // Cancelling is distinct from releasing the connection, which the transceiver does when it is destroyed.
    nw_connection_cancel(_connection.get());

    lock_guard lock(_async->mutex);
    cancel(*_async);
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
    assert(_state == StateProxyWrite || _state == StateConnected);
    assert(buf.b.end() - buf.i > 0);
    size_t length = static_cast<size_t>(buf.b.end() - buf.i);

    //
    // Create dispatch_data_t from the buffer. dispatch_data_create with
    // DISPATCH_DATA_DESTRUCTOR_DEFAULT copies the data.
    //
    DispatchRef<dispatch_data_t> data = DispatchRef<dispatch_data_t>::adopt(
        dispatch_data_create(&*buf.i, length, _dispatchQueue.get(), DISPATCH_DATA_DESTRUCTOR_DEFAULT));

    {
        lock_guard lock(_async->mutex);
        _async->write.start();
    }
    auto async = _async; // See initialize.
    nw_connection_send(
        _connection.get(),
        data.get(),
        NW_CONNECTION_DEFAULT_MESSAGE_CONTEXT,
        true, // The message is complete: all the data is sent.
        ^(nw_error_t error) {
          lock_guard lock(async->mutex);
          complete(
              *async,
              async->write,
              SocketOperationWrite,
              error ? AsyncState::WriteResult{0, nw_error_get_error_code(error)} : AsyncState::WriteResult{length, 0});
        });

    return true;
}

void
IceInternal::NetworkFrameworkTransceiver::finishWrite(Buffer& buf)
{
    assert(_state == StateProxyWrite || _state == StateConnected);

    AsyncState::WriteResult result;
    {
        lock_guard lock(_async->mutex);
        result = _async->write.consume();
    }

    if (result.error != 0)
    {
        throw ConnectionLostException(__FILE__, __LINE__, result.error);
    }

    buf.i += result.count;

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

    {
        lock_guard lock(_async->mutex);
        _async->read.start();
    }
    auto async = _async; // See initialize.
    nw_connection_receive(
        _connection.get(),
        1,                             // minimum bytes
        static_cast<uint32_t>(length), // maximum bytes
        ^(dispatch_data_t content, nw_content_context_t, bool, nw_error_t error) {
          // The result is built before the state is locked.
          __block AsyncState::ReadResult result;
          if (error)
          {
              result.error = nw_error_get_error_code(error);
          }
          else if (content)
          {
              dispatch_data_apply(content, ^bool(dispatch_data_t, size_t, const void* buffer, size_t size) {
                auto* bytes = static_cast<const std::byte*>(buffer);
                result.data.insert(result.data.end(), bytes, bytes + size);
                return true;
              });
          }
          else
          {
              result.error = ECONNRESET; // No content and no error: the peer closed the connection.
          }

          lock_guard lock(async->mutex);
          complete(*async, async->read, SocketOperationRead, std::move(result));
        });
}

void
IceInternal::NetworkFrameworkTransceiver::finishRead(Buffer& buf)
{
    assert(_state == StateProxyRead || _state == StateConnected);

    AsyncState::ReadResult result;
    {
        lock_guard lock(_async->mutex);
        result = _async->read.consume();
    }

    if (result.error != 0)
    {
        throw ConnectionLostException(__FILE__, __LINE__, result.error);
    }

    if (result.data.empty())
    {
        throw ConnectionLostException(__FILE__, __LINE__);
    }

    size_t remaining = static_cast<size_t>(buf.b.end() - buf.i);
    size_t count = min(result.data.size(), remaining); // Never more than requested.

    memcpy(&*buf.i, result.data.data(), count);
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

    auto path = NetworkRef<nw_path_t>::adopt(nw_connection_copy_current_path(_connection.get()));
    if (path)
    {
        auto localEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_path_copy_effective_local_endpoint(path.get()));
        if (localEndpoint)
        {
            localAddress = nw_endpoint_get_hostname(localEndpoint.get());
            localPort = nw_endpoint_get_port(localEndpoint.get());
        }
        auto remoteEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_path_copy_effective_remote_endpoint(path.get()));
        if (remoteEndpoint)
        {
            remoteAddress = nw_endpoint_get_hostname(remoteEndpoint.get());
            remotePort = nw_endpoint_get_port(remoteEndpoint.get());
        }
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

        auto tlsDefinition = NetworkRef<nw_protocol_definition_t>::adopt(nw_protocol_copy_tls_definition());
        auto tlsMetadata = NetworkRef<nw_protocol_metadata_t>::adopt(
            nw_connection_copy_protocol_metadata(_connection.get(), tlsDefinition.get()));
        if (tlsMetadata)
        {
            auto secMetadata =
                SecRef<sec_protocol_metadata_t>::adopt(nw_tls_copy_sec_protocol_metadata(tlsMetadata.get()));
            if (secMetadata)
            {
                sec_protocol_metadata_access_peer_certificate_chain(secMetadata.get(), ^(sec_certificate_t cert) {
                  if (!peerCertificate)
                  {
                      peerCertificate = sec_certificate_copy_ref(cert); // Owned by the connection info.
                  }
                });
            }
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
    auto path = NetworkRef<nw_path_t>::adopt(nw_connection_copy_current_path(_connection.get()));
    if (path)
    {
        auto localEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_path_copy_effective_local_endpoint(path.get()));
        if (localEndpoint)
        {
            localAddress = nwEndpointToString(localEndpoint.get());
        }
        auto remoteEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_path_copy_effective_remote_endpoint(path.get()));
        if (remoteEndpoint)
        {
            remoteAddress = nwEndpointToString(remoteEndpoint.get());
        }
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
