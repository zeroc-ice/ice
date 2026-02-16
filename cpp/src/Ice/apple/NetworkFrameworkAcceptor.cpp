// Copyright (c) ZeroC, Inc.

#include "../Network.h"

#if defined(ICE_USE_NETWORK_FRAMEWORK)

#    include "Ice/LocalExceptions.h"
#    include "Ice/LoggerUtil.h"
#    include "Ice/Properties.h"
#    include "../ProtocolInstance.h"
#    include "../TcpEndpointI.h"
#    include "NetworkFrameworkAcceptor.h"
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
    if (_listener)
    {
        nw_listener_cancel(_listener);
    }

    // Release any queued connections that haven't been accepted.
    lock_guard lock(_acceptState->mutex);
    for (auto& conn : _acceptState->connections)
    {
        nw_release(conn);
    }
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
    nw_listener_set_state_changed_handler(_listener, ^(nw_listener_state_t state, nw_error_t error) {
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
    nw_listener_set_new_connection_handler(_listener, ^(nw_connection_t connection) {
        nw_retain(connection);
        lock_guard lock(acceptState->mutex);
        acceptState->connections.push_back(connection);
        if (acceptState->waiting)
        {
            acceptState->waiting = false;
            nativeInfo->completed(SocketOperationRead);
        }
    });

    //
    // Start the listener and wait for it to become ready.
    //
    nw_listener_set_queue(_listener, _dispatchQueue);
    nw_listener_start(_listener);

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
    nw_listener_set_state_changed_handler(_listener, nullptr);

    //
    // Retrieve the actual port assigned by the OS (important for port 0).
    //
    _port = nw_listener_get_port(_listener);

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
    nw_connection_t connection;
    {
        lock_guard lock(_acceptState->mutex);
        if (_acceptState->connections.empty())
        {
            throw SocketException(__FILE__, __LINE__, 0);
        }
        connection = _acceptState->connections.front();
        _acceptState->connections.pop_front();
    }

    try
    {
        auto transceiver = make_shared<NetworkFrameworkTransceiver>(_instance, connection);
        nw_release(connection); // The transceiver retains its own reference.
        return transceiver;
    }
    catch (...)
    {
        nw_release(connection);
        throw;
    }
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
    int port)
    : _endpoint(std::move(endpoint)),
      _instance(instance),
      _nativeInfo(make_shared<NativeInfo>(INVALID_SOCKET)),
      _listener(nullptr),
      _dispatchQueue(nullptr),
      _host(host),
      _port(static_cast<uint16_t>(port))
{
    //
    // Create TCP parameters (no TLS for plain TCP).
    //
    nw_parameters_t parameters = nw_parameters_create_secure_tcp(
        NW_PARAMETERS_DISABLE_PROTOCOL,
        NW_PARAMETERS_DEFAULT_CONFIGURATION);

    if (!parameters)
    {
        throw SocketException(__FILE__, __LINE__, 0);
    }

    //
    // If a specific local address is requested, set it on the parameters.
    // Use port "0" here — the actual listen port is specified separately
    // via nw_listener_create_with_port to avoid specifying the port twice.
    //
    if (!host.empty() && host != "0.0.0.0" && host != "::" && host != "*")
    {
        nw_endpoint_t localEndpoint = nw_endpoint_create_host(host.c_str(), "0");
        if (localEndpoint)
        {
            nw_parameters_set_local_endpoint(parameters, localEndpoint);
            nw_release(localEndpoint);
        }
    }

    //
    // Create the listener — with a specific port or an OS-assigned port.
    //
    if (port > 0)
    {
        _listener = nw_listener_create_with_port(to_string(port).c_str(), parameters);
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

    _dispatchQueue = dispatch_queue_create("com.zeroc.ice.nw-listener", DISPATCH_QUEUE_SERIAL);
    _acceptState = make_shared<AcceptState>();
}

IceInternal::NetworkFrameworkAcceptor::~NetworkFrameworkAcceptor()
{
    if (_listener)
    {
        nw_release(_listener);
    }
    if (_dispatchQueue)
    {
        dispatch_release(_dispatchQueue);
    }
}

#endif
