//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Connection.h>
#include <Ice/ObjectAdapter.h>
#include <Ice/Service.h>
#include <Ice/UUID.h>
#include <IceUtil/Options.h>

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
    QueuedDispatch(pair<const Byte*, const Byte*> p,
                   function<void(bool, const pair<const Byte*, const Byte*>&)>&& r,
                   function<void(exception_ptr)>&& e,
                   const Current& c) :
        inParams(p.first, p.second), response(r), error(e), current(c)
    {
    }

    QueuedDispatch(QueuedDispatch&&) = default;

    // Make sure we don't copy this struct by accident
    QueuedDispatch(const QueuedDispatch&) = delete;

    const vector<Byte> inParams;
    const function<void(bool, const pair<const Byte*, const Byte*>&)> response;
    const function<void(exception_ptr)> error;
    const Current current;
};

//
// Allows the bridge to be used as an Ice router.
//
class RouterI final : public Router
{
public:

    virtual shared_ptr<ObjectPrx> getClientProxy(Ice::optional<bool>& hasRoutingTable, const Current&) const override
    {
        hasRoutingTable = false; // We don't maintain a routing table, no need to call addProxies on this impl.
        return nullptr;
    }

    virtual shared_ptr<ObjectPrx> getServerProxy(const Current& current) const override
    {
        //
        // We return a non-nil dummy proxy here so that a client is able to configure its
        // callback object adapter with a router proxy.
        //
        return current.adapter->getCommunicator()->stringToProxy("dummy");
    }

    virtual ObjectProxySeq addProxies(ObjectProxySeq, const Current&) override
    {
        return ObjectProxySeq();
    }
};

class FinderI final : public RouterFinder
{
public:

    FinderI(shared_ptr<RouterPrx> router) :
        _router(move(router))
    {
    }

    virtual shared_ptr<RouterPrx> getRouter(const Current&) override
    {
        return _router;
    }

private:

    const shared_ptr<RouterPrx> _router;
};

//
// Represents a pair of connections (shared object)
//
class BridgeConnection final
{
public:

    BridgeConnection(shared_ptr<ObjectAdapter>, shared_ptr<ObjectPrx>, shared_ptr<Connection>);

    void outgoingSuccess(shared_ptr<Connection>);
    void outgoingException(exception_ptr);

    void closed(const shared_ptr<Connection>&);
    void dispatch(pair<const Byte*, const Byte*>,
                  function<void(bool, const pair<const Byte*, const Byte*>&)>,
                  function<void(exception_ptr)>,
                  const Current&);
private:

    void send(const shared_ptr<Connection>&,
              pair<const Byte*, const Byte*>,
              function<void(bool, const pair<const Byte*, const Byte*>&)>,
              function<void(exception_ptr)>,
              const Current& current);

    shared_ptr<ObjectAdapter> _adapter;
    shared_ptr<ObjectPrx> _target;
    shared_ptr<Connection> _incoming;

    std::mutex _lock;
    shared_ptr<Connection> _outgoing;
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

    BridgeI(shared_ptr<ObjectAdapter> adapter, shared_ptr<ObjectPrx> target);

    virtual void ice_invokeAsync(pair<const Byte*, const Byte*> inEncaps,
                                 function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                                 function<void(exception_ptr)> error,
                                 const Current& current) override;

    void closed(const shared_ptr<Connection>&);
    void outgoingSuccess(shared_ptr<BridgeConnection>, shared_ptr<Connection>);
    void outgoingException(shared_ptr<BridgeConnection>, exception_ptr);

private:

    const shared_ptr<ObjectAdapter> _adapter;
    const shared_ptr<ObjectPrx> _target;

    std::mutex _lock;
    map<shared_ptr<Connection>, shared_ptr<BridgeConnection>> _connections;
};

class BridgeService final : public Service
{
protected:

    virtual bool start(int, char*[], int&) override;
    virtual bool stop() override;
    virtual shared_ptr<Communicator> initializeCommunicator(int&, char*[], const InitializationData&, int) override;

private:

    void usage(const std::string&);
};

}

BridgeConnection::BridgeConnection(shared_ptr<ObjectAdapter> adapter,
                                   shared_ptr<ObjectPrx> target,
                                   shared_ptr<Connection> inc) :
    _adapter(move(adapter)), _target(move(target)), _incoming(move(inc))
{
}

void
BridgeConnection::outgoingSuccess(shared_ptr<Connection> outgoing)
{
    lock_guard<mutex> lg(_lock);
    assert(!_outgoing && outgoing);
    if(_exception)
    {
        //
        // The incoming connection is already closed. There's no point in leaving the outgoing
        // connection open.
        //
        outgoing->close(ConnectionClose::Gracefully);
        return;
    }

    _outgoing = move(outgoing);

    //
    // Register hearbeat callbacks on both connections.
    //
    _incoming->setHeartbeatCallback([con = _outgoing](const auto&)
                                    {
                                        try
                                        {
                                            con->heartbeatAsync(nullptr);
                                        }
                                        catch(...)
                                        {
                                        }
                                    });

    _outgoing->setHeartbeatCallback([con = _incoming](const auto&)
                                    {
                                        try
                                        {
                                            con->heartbeatAsync(nullptr);
                                        }
                                        catch(...)
                                        {
                                        }
                                    });

    //
    // Configure the outgoing connection for bidirectional requests.
    //
    _outgoing->setAdapter(_adapter);

    //
    // Flush any queued dispatches
    //
    for(auto& p : _queue)
    {
        auto inParams = make_pair(p.inParams.data(), p.inParams.data() + p.inParams.size());
        send(outgoing, inParams, p.response, p.error, p.current);
    }
    _queue.clear();
}

void
BridgeConnection::outgoingException(exception_ptr ex)
{
    lock_guard<mutex> lg(_lock);
    if(_exception)
    {
        return;
    }
    _exception = ex;

    //
    // The outgoing connection failed so we close the incoming connection. closed() will eventually
    // be called for it when the connection's dispatch count reaches zero.
    //
    _incoming->close(ConnectionClose::Gracefully);

    //
    // Complete the queued incoming dispatch, otherwise the incoming connection will never
    // complete its graceful closure. This is only necessary on the server side.
    //
    // The client will receive an UnknownLocalException whose reason member contains information
    // about the failure.
    //
    for(auto& p : _queue)
    {
        p.error(ex);
    }
    _queue.clear();
}

void
BridgeConnection::closed(const shared_ptr<Connection>& con)
{
    lock_guard<mutex> lg(_lock);
    if(_exception)
    {
        return; // Nothing to do if the exception is already set, both connections have been closed already.
    }

    auto toBeClosed = con == _incoming ? _outgoing : _incoming;
    try
    {
        con->throwException();
    }
    catch(const Ice::CloseConnectionException&)
    {
        _exception = current_exception();
        if(toBeClosed)
        {
            toBeClosed->close(ConnectionClose::Gracefully);
        }
    }
    catch(const std::exception&)
    {
        _exception = current_exception();
        if(toBeClosed)
        {
            toBeClosed->close(ConnectionClose::Forcefully);
        }
    }

    //
    // Even though the connection is already closed, we still need to "complete" the pending dispatches so
    // that the connection's dispatch count is updated correctly.
    //
    for(auto& p : _queue)
    {
        p.error(_exception);
    }
    _queue.clear();
}

void
BridgeConnection::dispatch(pair<const Byte*, const Byte*> inParams,
                           function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                           function<void(exception_ptr)> error,
                           const Current& current)
{
    //
    // We've received an invocation, either from the client via the incoming connection, or from
    // the server via the outgoing (bidirectional) connection. The current.con member tells us
    // the connection over which the request arrived.
    //
    lock_guard<mutex> lg(_lock);
    if(_exception)
    {
        error(_exception);
    }
    else if(!_outgoing)
    {
        //
        // Queue the invocation until the outgoing connection is established.
        //
        assert(current.con == _incoming);
        _queue.emplace_back(inParams, move(response), move(error), current);
    }
    else
    {
        send(current.con == _incoming ? _outgoing : _incoming, inParams, response, error, current);
    }
}

void
BridgeConnection::send(const shared_ptr<Connection>& dest,
                       pair<const Byte*, const Byte*> inParams,
                       function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                       function<void(exception_ptr)> error,
                       const Current& current)
{
    try
    {
        //
        // Create a proxy having the same identity as the request.
        //
        auto prx = dest->createProxy(current.id);

        if(!current.requestId)
        {
            // Oneway request
            if(prx->ice_isTwoway())
            {
                prx = prx->ice_oneway();
            }
            prx->ice_invokeAsync(current.operation, current.mode, inParams, nullptr, error,
                                 [response = move(response)](bool){ response(true, make_pair(nullptr, nullptr)); },
                                 current.ctx);
        }
        else
        {
            // Twoway request
            prx->ice_invokeAsync(current.operation, current.mode, inParams, response, error, nullptr, current.ctx);
        }
    }
    catch(const std::exception&)
    {
        error(current_exception());
    }
}

BridgeI::BridgeI(shared_ptr<ObjectAdapter> adapter, shared_ptr<ObjectPrx> target) :
    _adapter(move(adapter)), _target(move(target))
{
}

void
BridgeI::ice_invokeAsync(pair<const Byte*, const Byte*> inParams,
                         function<void(bool, const pair<const Byte*, const Byte*>&)> response,
                         function<void(exception_ptr)> error,
                         const Current& current)
{
    shared_ptr<BridgeConnection> bc;
    {
        lock_guard<mutex> lg(_lock);

        auto p = _connections.find(current.con);
        if(p == _connections.end())
        {
            //
            // The connection is unknown to us, it must be a new incoming connection.
            //
            auto info = current.con->getEndpoint()->getInfo();

            //
            // Create a target proxy that matches the configuration of the incoming connection.
            //
            shared_ptr<ObjectPrx> target;
            if(info->datagram())
            {
                target = _target->ice_datagram();
            }
            else if(info->secure())
            {
                target = _target->ice_secure(true);
            }
            else
            {
                target = _target;
            }

            //
            // Force the proxy to establish a new connection by using a unique connection ID.
            //
            target = target->ice_connectionId(Ice::generateUUID());

            bc = make_shared<BridgeConnection>(_adapter, target, current.con);
            _connections.emplace(make_pair(current.con, bc));

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
                    [self, bc](auto outgoing) { self->outgoingSuccess(move(bc), move(outgoing)); },
                    [self, bc](auto ex) { self->outgoingException(move(bc), ex); });
            }
            catch(const std::exception&)
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
    bc->dispatch(inParams, response, error, current);
}

void
BridgeI::closed(const shared_ptr<Connection>& con)
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
BridgeI::outgoingSuccess(shared_ptr<BridgeConnection> bc, shared_ptr<Connection> outgoing)
{
    //
    // An outgoing connection was established. Notify the BridgeConnection object.
    //
    {
        lock_guard<mutex> lg(_lock);
        _connections.insert(make_pair(outgoing, bc));
        outgoing->setCloseCallback([self = shared_from_this()](const auto& con) { self->closed(con); });
    }
    bc->outgoingSuccess(move(outgoing));
}

void
BridgeI::outgoingException(shared_ptr<BridgeConnection> bc, exception_ptr ex)
{
    //
    // An outgoing connection attempt failed. Notify the BridgeConnection object.
    //
    bc->outgoingException(ex);
}

bool
BridgeService::start(int argc, char* argv[], int& status)
{
    IceUtilInternal::Options opts;
    opts.addOpt("h", "help");
    opts.addOpt("v", "version");

    vector<string> args;
    try
    {
        args = opts.parse(argc, const_cast<const char**>(argv));
    }
    catch(const IceUtilInternal::BadOptException& e)
    {
        error(e.reason);
        usage(argv[0]);
        return false;
    }

    if(opts.isSet("help"))
    {
        usage(argv[0]);
        status = EXIT_SUCCESS;
        return false;
    }
    if(opts.isSet("version"))
    {
        print(ICE_STRING_VERSION);
        status = EXIT_SUCCESS;
        return false;
    }

    if(!args.empty())
    {
        cerr << argv[0] << ": too many arguments" << endl;
        usage(argv[0]);
        return false;
    }

    auto properties = communicator()->getProperties();

    const string targetProperty = "IceBridge.Target.Endpoints";
    const string targetEndpoints = properties->getProperty(targetProperty);
    if(targetEndpoints.empty())
    {
        error("property '" + targetProperty + "' is not set");
        return false;
    }

    shared_ptr<Ice::ObjectPrx> target;

    try
    {
        target = communicator()->stringToProxy("dummy:" + targetEndpoints);
    }
    catch(const std::exception& ex)
    {
        ServiceError err(this);
        err << "setting for target endpoints '" << targetEndpoints << "' is invalid:\n" << ex;
        return false;
    }

    //
    // Initialize the object adapter.
    //
    const string sourceProperty = "IceBridge.Source.Endpoints";
    if(properties->getProperty(sourceProperty).empty())
    {
        error("property '" + sourceProperty + "' is not set");
        return false;
    }

    auto adapter = communicator()->createObjectAdapter("IceBridge.Source");

    adapter->addDefaultServant(make_shared<BridgeI>(adapter, move(target)), "");

    string instanceName = properties->getPropertyWithDefault("IceBridge.InstanceName", "IceBridge");
    auto router = uncheckedCast<RouterPrx>(adapter->add(make_shared<RouterI>(),
                                                        stringToIdentity(instanceName + "/router")));
    adapter->add(make_shared<FinderI>(router), stringToIdentity("Ice/RouterFinder"));

    try
    {
        adapter->activate();
    }
    catch(const std::exception& ex)
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

shared_ptr<Communicator>
BridgeService::initializeCommunicator(int& argc, char* argv[], const InitializationData& initializationData,
                                      int version)
{
    InitializationData initData = initializationData;
    initData.properties = createProperties(argc, argv, initializationData.properties);

    StringSeq args = argsToStringSeq(argc, argv);
    args = initData.properties->parseCommandLineOptions("IceBridge", args);
    stringSeqToArgs(args, argc, argv);

    //
    // Disable automatic retry by default.
    //
    if(initData.properties->getProperty("Ice.RetryIntervals").empty())
    {
        initData.properties->setProperty("Ice.RetryIntervals", "-1");
    }

    return Service::initializeCommunicator(argc, argv, initData, version);
}

void
BridgeService::usage(const string& appName)
{
    string options =
        "Options:\n"
        "-h, --help           Show this message.\n"
        "-v, --version        Display the Ice version.\n";
#ifndef _WIN32
    options.append(
        "--daemon             Run as a daemon.\n"
        "--pidfile FILE       Write process ID into FILE.\n"
        "--noclose            Do not close open file descriptors.\n"
        "--nochdir            Do not change the current working directory.\n"
    );
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
    return svc.main(argc, argv);
}
