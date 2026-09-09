// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "../ProtocolInstance.h"
#    include "../TcpEndpointI.h"
#    include "Ice/LocalExceptions.h"
#    include "Ice/Properties.h"
#    include "NetworkFrameworkAcceptor.h"
#    include "NetworkFrameworkTLS.h"
#    include "NetworkFrameworkTransceiver.h"

#    include <utility>

using namespace std;
using namespace Ice;
using namespace IceInternal;

NativeInfoPtr
IceInternal::NetworkFrameworkAcceptor::getNativeInfo()
{
    return _nativeInfo;
}

void
IceInternal::NetworkFrameworkAcceptor::close()
{
    // Cancel the listener and wait for it to reach the cancelled state, its terminal state: the listening port is
    // released before close() returns, so that it can be reused right away (for example when an adapter is destroyed
    // and recreated). Cancelling is distinct from releasing the listener, which the acceptor does when destroyed.
    auto semaphore = DispatchRef<dispatch_semaphore_t>::adopt(dispatch_semaphore_create(0));
    dispatch_semaphore_t cancelled = semaphore.get(); // Signaled before the wait below returns.
    nw_listener_set_state_changed_handler(_listener.get(), ^(nw_listener_state_t state, nw_error_t) {
      if (state == nw_listener_state_cancelled)
      {
          dispatch_semaphore_signal(cancelled);
      }
    });
    nw_listener_cancel(_listener.get());
    dispatch_semaphore_wait(semaphore.get(), DISPATCH_TIME_FOREVER);

    // Release the queued connections no transceiver adopted.
    lock_guard lock(_acceptState->mutex);
    _acceptState->connections.clear();

    // If startAccept() is waiting for a connection, signal the completion so the thread pool
    // can process the close and transition IncomingConnectionFactory to StateFinished.
    if (_acceptState->waiting)
    {
        _acceptState->waiting = false;
        _nativeInfo->completed(SocketOperationRead);
    }
}

EndpointIPtr
IceInternal::NetworkFrameworkAcceptor::listen()
{
    //
    // Synchronization state for waiting until the listener is ready.
    //
    struct ListenState
    {
        std::mutex mutex;
        std::condition_variable cv;
        bool signaled{false};
        int error{0};
    };
    auto listenState = make_shared<ListenState>();

    //
    // Set state changed handler to detect when the listener is ready.
    //
    nw_listener_set_state_changed_handler(_listener.get(), ^(nw_listener_state_t state, nw_error_t error) {
      if (state == nw_listener_state_ready || state == nw_listener_state_failed)
      {
          lock_guard lock(listenState->mutex);
          listenState->signaled = true;
          if (state == nw_listener_state_failed)
          {
              listenState->error = error ? nw_error_get_error_code(error) : EADDRINUSE;
          }
          listenState->cv.notify_one();
      }
    });

    //
    // Set new connection handler. When a client connects, the handler queues
    // the nw_connection and, if the acceptor is waiting for a connection,
    // signals the selector so the thread pool can pick it up.
    //
    auto acceptState = _acceptState;
    NativeInfoPtr nativeInfo = _nativeInfo;
    nw_listener_set_new_connection_handler(_listener.get(), ^(nw_connection_t connection) {
      // The connection is borrowed from Network.framework: retain it until a transceiver adopts it.
      lock_guard lock(acceptState->mutex);
      acceptState->connections.push_back(NetworkRef<nw_connection_t>::retain(connection));
      if (acceptState->waiting)
      {
          acceptState->waiting = false;
          nativeInfo->completed(SocketOperationRead);
      }
    });

    //
    // Start the listener and wait for it to become ready.
    //
    nw_listener_set_queue(_listener.get(), _dispatchQueue.get());
    nw_listener_start(_listener.get());

    {
        unique_lock lock(listenState->mutex);
        listenState->cv.wait(lock, [&] { return listenState->signaled; });
    }

    if (listenState->error != 0)
    {
        throw SocketException(__FILE__, __LINE__, listenState->error);
    }

    //
    // Clear the state changed handler — we no longer need it for the listen flow.
    //
    nw_listener_set_state_changed_handler(_listener.get(), nullptr);

    //
    // Retrieve the actual port assigned by the OS (important for port 0).
    //
    _port = nw_listener_get_port(_listener.get());

    _endpoint = _endpoint->endpoint(shared_from_this());
    return _endpoint;
}

void
IceInternal::NetworkFrameworkAcceptor::startAccept()
{
    lock_guard lock(_acceptState->mutex);
    if (!_acceptState->connections.empty())
    {
        //
        // A connection is already queued — signal immediately so the
        // thread pool picks it up without waiting.
        //
        _nativeInfo->completed(SocketOperationRead);
    }
    else
    {
        //
        // No connections yet — the new_connection_handler will signal
        // when one arrives.
        //
        _acceptState->waiting = true;
    }
}

void
IceInternal::NetworkFrameworkAcceptor::finishAccept()
{
    // Nothing to do — the connection is already queued in _acceptState
    // and will be retrieved by accept().
}

TransceiverPtr
IceInternal::NetworkFrameworkAcceptor::accept()
{
    NetworkRef<nw_connection_t> connection;
    {
        lock_guard lock(_acceptState->mutex);
        if (_acceptState->connections.empty())
        {
            throw SocketException(__FILE__, __LINE__, 0);
        }
        connection = std::move(_acceptState->connections.front());
        _acceptState->connections.pop_front();
    }
    return make_shared<NetworkFrameworkTransceiver>(_instance, std::move(connection), _secure);
}

string
IceInternal::NetworkFrameworkAcceptor::protocol() const
{
    return _instance->protocol();
}

string
IceInternal::NetworkFrameworkAcceptor::toString() const
{
    return _host + ":" + to_string(_port);
}

string
IceInternal::NetworkFrameworkAcceptor::toDetailedString() const
{
    return "local address = " + toString();
}

int
IceInternal::NetworkFrameworkAcceptor::effectivePort() const
{
    return _port;
}

IceInternal::NetworkFrameworkAcceptor::NetworkFrameworkAcceptor(
    TcpEndpointIPtr endpoint,
    const ProtocolInstancePtr& instance,
    const string& host,
    int port,
    const string& adapterName,
    const optional<Ice::SSL::ServerAuthenticationOptions>& serverAuthenticationOptions)
    : _endpoint(std::move(endpoint)),
      _instance(instance),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _host(host),
      _port(static_cast<uint16_t>(port)),
      _secure(serverAuthenticationOptions.has_value())
{
    //
    // Create TCP parameters — with or without TLS.
    //
    NetworkRef<nw_parameters_t> parameters;
    if (serverAuthenticationOptions)
    {
        // The immutable configuration is owned by the blocks Network.framework invokes during the handshakes of the
        // accepted connections; the configuration result is a separate, per-listener value.
        auto configuration = make_shared<const NetworkFrameworkTLS::ServerConfiguration>(
            *serverAuthenticationOptions,
            adapterName,
            instance->logger());
        __block exception_ptr error;
        parameters = NetworkRef<nw_parameters_t>::adopt(nw_parameters_create_secure_tcp(
            ^(nw_protocol_options_t tlsOptions) {
              error = NetworkFrameworkTLS::configureServerTLS(tlsOptions, configuration);
            },
            NW_PARAMETERS_DEFAULT_CONFIGURATION));

        if (error)
        {
            // Set by the configure block, which nw_parameters_create_secure_tcp invoked synchronously.
            rethrow_exception(error);
        }
    }
    else
    {
        parameters = NetworkRef<nw_parameters_t>::adopt(
            nw_parameters_create_secure_tcp(NW_PARAMETERS_DISABLE_PROTOCOL, NW_PARAMETERS_DEFAULT_CONFIGURATION));
    }

    if (!parameters)
    {
        throw SocketException(__FILE__, __LINE__, 0);
    }

    // Allow the listener to bind a port that is still in the TIME_WAIT state, as the BSD acceptor does with
    // SO_REUSEADDR: an adapter that is destroyed and recreated, or the test suite reusing a fixed port, would
    // otherwise fail to bind with "address already in use".
    nw_parameters_set_reuse_local_address(parameters.get(), true);

    //
    // If a specific local address is requested, set it on the parameters.
    // Use port "0" here — the actual listen port is specified separately
    // via nw_listener_create_with_port to avoid specifying the port twice.
    //
    if (!host.empty() && host != "0.0.0.0" && host != "::" && host != "*")
    {
        auto localEndpoint = NetworkRef<nw_endpoint_t>::adopt(nw_endpoint_create_host(host.c_str(), "0"));
        if (localEndpoint)
        {
            nw_parameters_set_local_endpoint(parameters.get(), localEndpoint.get());
        }
    }

    //
    // Create the listener — with a specific port or an OS-assigned port.
    //
    _listener = NetworkRef<nw_listener_t>::adopt(
        port > 0 ? nw_listener_create_with_port(to_string(port).c_str(), parameters.get())
                 : nw_listener_create(parameters.get()));
    if (!_listener)
    {
        throw SocketException(__FILE__, __LINE__, 0);
    }

    _dispatchQueue =
        DispatchRef<dispatch_queue_t>::adopt(dispatch_queue_create("com.zeroc.ice.nw-listener", DISPATCH_QUEUE_SERIAL));
    _acceptState = make_shared<AcceptState>();
}

#endif
