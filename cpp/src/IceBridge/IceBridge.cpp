// Copyright (c) ZeroC, Inc.

#include "../Ice/Options.h"
#include "Ice/Connection.h"
#include "Ice/ObjectAdapter.h"
#include "Ice/Router.h"
#include "Ice/Service.h"
#include "Ice/UUID.h"

#include <iostream>

using namespace std;
using namespace Ice;

namespace
{
    //
    // Holds information about an incoming dispatch that's been queued until an outgoing connection has
    // been established.
    //
    struct QueuedDispatch final
    {
        //
        // The pointers in p refer to the Ice marshaling buffer and won't remain valid after
        // ice_invokeAsync completes, so we have to make a copy of the in parameters
        //
        QueuedDispatch(
            pair<const byte*, const byte*> p,
            function<void(bool, pair<const byte*, const byte*>)>&& r,
            function<void(exception_ptr)>&& e,
            Current c)
            : inParams(p.first, p.second),
              response(std::move(r)),
              error(std::move(e)),
              current(std::move(c))
        {
        }

        QueuedDispatch(QueuedDispatch&&) = default;

        // Make sure we don't copy this struct by accident
        QueuedDispatch(const QueuedDispatch&) = delete;

        const vector<byte> inParams;
        function<void(bool, pair<const byte*, const byte*>)> response;
        function<void(exception_ptr)> error;
        const Current current;
    };

    //
    // Allows the bridge to be used as an Ice router.
    //
    class RouterI final : public Router
    {
    public:
        optional<ObjectPrx> getClientProxy(optional<bool>& hasRoutingTable, const Current&) const final
        {
            hasRoutingTable = false; // We don't maintain a routing table, no need to call addProxies on this impl.
            return nullopt;
        }

        [[nodiscard]] optional<ObjectPrx> getServerProxy(const Current& current) const final
        {
            // We return a non-null dummy proxy here so that a client is able to configure its
            // callback object adapter with a router proxy.
            return ObjectPrx{current.adapter->getCommunicator(), "dummy"};
        }

        ObjectProxySeq addProxies(ObjectProxySeq, const Current&) final { return {}; }
    };

    class FinderI final : public RouterFinder
    {
    public:
        FinderI(RouterPrx router) : _router(std::move(router)) {}

        optional<RouterPrx> getRouter(const Current&) final { return _router; }

    private:
        const RouterPrx _router;
    };

    //
    // Represents a pair of connections (shared object)
    //
    class BridgeConnection final
    {
    public:
        BridgeConnection(ObjectAdapterPtr, ObjectPrx, ConnectionPtr);

        void outgoingSuccess(ConnectionPtr);
        void outgoingException(exception_ptr);

        void closed(const ConnectionPtr&);
        void dispatch(
            pair<const byte*, const byte*>,
            function<void(bool, pair<const byte*, const byte*>)>,
            function<void(exception_ptr)>,
            const Current&);

    private:
        void send(
            const ConnectionPtr&,
            pair<const byte*, const byte*>,
            function<void(bool, pair<const byte*, const byte*>)>,
            function<void(exception_ptr)>,
            const Current& current);

        const ObjectAdapterPtr _adapter;
        const ObjectPrx _target;
        const ConnectionPtr _incoming;

        std::mutex _lock;
        ConnectionPtr _outgoing;
        exception_ptr _exception;

        //
        // We maintain our own queue for dispatches that arrive on the incoming connection before the outgoing
        // connection has been established. We don't want to forward these to proxies and let the proxies handle
        // the queuing because then the invocations could be sent out of order (e.g., when invocations are split
        // among twoway/oneway/datagram proxies).
        //
        vector<QueuedDispatch> _queue;
    };

    //
    // The main bridge servant.
    //
    class BridgeI final : public Ice::BlobjectArrayAsync, public enable_shared_from_this<BridgeI>
    {
    public:
        BridgeI(ObjectAdapterPtr adapter, ObjectPrx target);

        void ice_invokeAsync(
            pair<const byte*, const byte*> inEncaps,
            function<void(bool, pair<const byte*, const byte*>)> response,
            function<void(exception_ptr)> error,
            const Current& current) final;

        void closed(const ConnectionPtr&);
        void outgoingSuccess(const shared_ptr<BridgeConnection>&, ConnectionPtr);
        void outgoingException(const shared_ptr<BridgeConnection>&, exception_ptr);

    private:
        const ObjectAdapterPtr _adapter;
        const ObjectPrx _target;

        std::mutex _lock;
        map<ConnectionPtr, shared_ptr<BridgeConnection>> _connections;
    };

    class BridgeService final : public Service
    {
    protected:
        bool start(int, char*[], int&) final;
        bool stop() final;
        CommunicatorPtr initializeCommunicator(int&, char*[], InitializationData) final;

    private:
        void usage(const std::string&);
    };
}

BridgeConnection::BridgeConnection(ObjectAdapterPtr adapter, ObjectPrx target, ConnectionPtr inc)
    : _adapter(std::move(adapter)),
      _target(std::move(target)),
      _incoming(std::move(inc))
{
}

void
BridgeConnection::outgoingSuccess(ConnectionPtr outgoing)
{
    lock_guard<mutex> lg(_lock);
    assert(!_outgoing && outgoing);
    if (_exception)
    {
        //
        // The incoming connection is already closed. There's no point in leaving the outgoing
        // connection open.
        //
        outgoing->close(nullptr, nullptr);
        return;
    }

    _outgoing = std::move(outgoing);

    //
    // Configure the outgoing connection for bidirectional requests.
    //
    _outgoing->setAdapter(_adapter);

    //
    // Flush any queued dispatches
    //
    for (auto& dispatch : _queue)
    {
        auto inParams = make_pair(dispatch.inParams.data(), dispatch.inParams.data() + dispatch.inParams.size());
        send(_outgoing, inParams, std::move(dispatch.response), std::move(dispatch.error), dispatch.current);
    }
    _queue.clear();
}

void
BridgeConnection::outgoingException(exception_ptr ex)
{
    lock_guard<mutex> lg(_lock);
    if (_exception)
    {
        return;
    }
    _exception = ex;

    //
    // The outgoing connection failed so we close the incoming connection. closed() will eventually
    // be called for it when the connection's dispatch count reaches zero.
    //
    _incoming->close(nullptr, nullptr);

    //
    // Complete the queued incoming dispatch, otherwise the incoming connection will never
    // complete its graceful closure. This is only necessary on the server side.
    //
    // The client will receive an UnknownLocalException whose reason member contains information
    // about the failure.
    //
    for (const auto& p : _queue)
    {
        p.error(ex);
    }
    _queue.clear();
}

void
BridgeConnection::closed(const ConnectionPtr& con)
{
    lock_guard<mutex> lg(_lock);
    if (_exception)
    {
        return; // Nothing to do if the exception is already set, both connections have been closed already.
    }

    auto toBeClosed = con == _incoming ? _outgoing : _incoming;
    try
    {
        con->throwException();
    }
    catch (const Ice::CloseConnectionException&)
    {
        _exception = current_exception();
        if (toBeClosed)
        {
            toBeClosed->close(nullptr, nullptr);
        }
    }
    catch (const std::exception&)
    {
        _exception = current_exception();
        if (toBeClosed)
        {
            toBeClosed->abort();
        }
    }

    //
    // Even though the connection is already closed, we still need to "complete" the pending dispatches so
    // that the connection's dispatch count is updated correctly.
    //
    for (const auto& p : _queue)
    {
        p.error(_exception);
    }
    _queue.clear();
}

void
BridgeConnection::dispatch(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    //
    // We've received an invocation, either from the client via the incoming connection, or from
    // the server via the outgoing (bidirectional) connection. The current.con member tells us
    // the connection over which the request arrived.
    //
    lock_guard<mutex> lg(_lock);
    if (_exception)
    {
        error(_exception);
    }
    else if (!_outgoing)
    {
        //
        // Queue the invocation until the outgoing connection is established.
        //
        assert(current.con == _incoming);
        _queue.emplace_back(inParams, std::move(response), std::move(error), current);
    }
    else
    {
        send(
            current.con == _incoming ? _outgoing : _incoming,
            inParams,
            std::move(response),
            std::move(error),
            current);
    }
}

void
BridgeConnection::send(
    const ConnectionPtr& dest,
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> error, // NOLINT(performance-unnecessary-value-param)
    const Current& current)
{
    try
    {
        //
        // Create a proxy having the same identity as the request.
        //
        auto prx = dest->createProxy(current.id);

        if (!current.requestId)
        {
            // Oneway request
            if (prx->ice_isTwoway())
            {
                prx = prx->ice_oneway();
            }
            prx->ice_invokeAsync(
                current.operation,
                current.mode,
                inParams,
                nullptr,
                error,
                [response = std::move(response)](bool) { response(true, {nullptr, nullptr}); },
                current.ctx);
        }
        else
        {
            // Twoway request
            prx->ice_invokeAsync(
                current.operation,
                current.mode,
                inParams,
                std::move(response),
                error,
                nullptr,
                current.ctx);
        }
    }
    catch (const std::exception&)
    {
        // can't move error parameter above since we need it here
        error(current_exception());
    }
}

BridgeI::BridgeI(ObjectAdapterPtr adapter, ObjectPrx target) : _adapter(std::move(adapter)), _target(std::move(target))
{
}

void
BridgeI::ice_invokeAsync(
    pair<const byte*, const byte*> inParams,
    function<void(bool, pair<const byte*, const byte*>)> response,
    function<void(exception_ptr)> error,
    const Current& current)
{
    shared_ptr<BridgeConnection> bc;
    {
        lock_guard<mutex> lg(_lock);

        auto p = _connections.find(current.con);
        if (p == _connections.end())
        {
            //
            // The connection is unknown to us, it must be a new incoming connection.
            //
            auto info = current.con->getEndpoint()->getInfo();

            //
            // Create a target proxy that matches the configuration of the incoming connection.
            //
            ObjectPrx target = _target;
            if (info->datagram())
            {
                target = target->ice_datagram();
            }
            else if (info->secure())
            {
                target = target->ice_secure(true);
            }

            //
            // Force the proxy to establish a new connection by using a unique connection ID.
            //
            target = target->ice_connectionId(Ice::generateUUID());

            bc = make_shared<BridgeConnection>(_adapter, target, current.con);
            _connections.emplace(current.con, bc);

            auto self = shared_from_this();
            current.con->setCloseCallback([self](const auto& con) { self->closed(con); });

            //
            // Try to establish the outgoing connection asynchronously
            //
            try
            {
                //
                // Begin the connection establishment process asynchronously. This can take a while to complete,
                // especially when using Bluetooth.
                //
                target->ice_getConnectionAsync(
                    [self, bc](auto outgoing) { self->outgoingSuccess(bc, std::move(outgoing)); },
                    [self, bc](auto ex) { self->outgoingException(bc, ex); });
            }
            catch (const std::exception&)
            {
                error(current_exception());
                return;
            }
        }
        else
        {
            bc = p->second;
        }
    }

    //
    // Delegate the invocation to the BridgeConnection object.
    //
    bc->dispatch(inParams, std::move(response), std::move(error), current);
}

void
BridgeI::closed(const ConnectionPtr& con)
{
    //
    // Notify the BridgeConnection that a connection has closed. We also need to remove it from our map.
    //
    shared_ptr<BridgeConnection> bc;
    {
        lock_guard<mutex> lg(_lock);
        auto p = _connections.find(con);
        assert(p != _connections.end());
        bc = p->second;
        _connections.erase(p);
    }
    assert(bc && con);
    bc->closed(con);
}

void
BridgeI::outgoingSuccess(const shared_ptr<BridgeConnection>& bc, ConnectionPtr outgoing)
{
    //
    // An outgoing connection was established. Notify the BridgeConnection object.
    //
    {
        lock_guard<mutex> lg(_lock);
        _connections.emplace(outgoing, bc);
        outgoing->setCloseCallback([self = shared_from_this()](const auto& con) { self->closed(con); });
    }
    bc->outgoingSuccess(std::move(outgoing));
}

void
BridgeI::outgoingException(const shared_ptr<BridgeConnection>& bc, exception_ptr ex)
{
    //
    // An outgoing connection attempt failed. Notify the BridgeConnection object.
    //
    bc->outgoingException(ex);
}

bool
BridgeService::start(int argc, char* argv[], int& status)
{
    IceInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch (const IceInternal::BadOptException& e)
    {
        error(e.what());
        usage(argv[0]);
        return false;
    }

    if (opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if (opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }

    if (!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    auto properties = communicator()->getProperties();

    const string targetProperty = "IceBridge.Target.Endpoints";
    const string targetEndpoints = properties->getProperty(targetProperty);
    if (targetEndpoints.empty())
    {
        error("property '" + targetProperty + "' is not set");
        return false;
    }

    //
    // Initialize the object adapter.
    //
    const string sourceProperty = "IceBridge.Source.Endpoints";
    if (properties->getProperty(sourceProperty).empty())
    {
        error("property '" + sourceProperty + "' is not set");
        return false;
    }

    auto adapter = communicator()->createObjectAdapter("IceBridge.Source");

    try
    {
        ObjectPrx target(communicator(), "dummy:" + targetEndpoints);
        adapter->addDefaultServant(make_shared<BridgeI>(adapter, std::move(target)), "");
    }
    catch (const std::exception& ex)
    {
        ServiceError err(this);
        err << "setting for target endpoints '" << targetEndpoints << "' is invalid:\n" << ex;
        return false;
    }

    string instanceName = properties->getIceProperty("IceBridge.InstanceName");
    auto router = adapter->add<RouterPrx>(make_shared<RouterI>(), stringToIdentity(instanceName + "/router"));
    adapter->add(make_shared<FinderI>(router), stringToIdentity("Ice/RouterFinder"));

    try
    {
        adapter->activate();
    }
    catch (const std::exception& ex)
    {
        {
            ServiceError err(this);
            err << "caught exception activating object adapter\n" << ex;
        }

        stop();
        return false;
    }

    return true;
}

bool
BridgeService::stop()
{
    return true;
}

CommunicatorPtr
BridgeService::initializeCommunicator(int& argc, char* argv[], InitializationData initData)
{
    initData.properties = createProperties(argc, argv, initData.properties);

    //
    // Disable automatic retry by default.
    //
    if (initData.properties->getIceProperty("Ice.RetryIntervals").empty())
    {
        initData.properties->setProperty("Ice.RetryIntervals", "-1");
    }

    return Service::initializeCommunicator(argc, argv, std::move(initData));
}

void
BridgeService::usage(const string& appName)
{
    string options = "Options:\n"
                     "-h, --help           Show this message.\n"
                     "-v, --version        Display the Ice version.\n";
#ifndef _WIN32
    options.append("--daemon             Run as a daemon.\n"
                   "--pidfile FILE       Write process ID into FILE.\n"
                   "--noclose            Do not close open file descriptors.\n"
                   "--nochdir            Do not change the current working directory.\n");
#endif
    print("Usage: " + appName + " [options]\n" + options);
}

#ifdef _WIN32
int
wmain(int argc, wchar_t* argv[])
#else
int
main(int argc, char* argv[])
#endif
{
    BridgeService svc;
    // Initialize the service with a Properties object with the correct property prefix enabled.
    Ice::InitializationData initData;
    initData.properties = make_shared<Properties>(vector<string>{"IceBridge"});
    return svc.main(argc, argv, std::move(initData));
}
