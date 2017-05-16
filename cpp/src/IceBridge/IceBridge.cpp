// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

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
// Represents a pending invocation.
//
class Invocation : public IceUtil::Shared
{
public:

    Invocation(const AMD_Object_ice_invokePtr& cb) :
        _cb(cb)
    {
    }

    void success(bool ok, const pair<const Byte*, const Byte*>& results)
    {
        _cb->ice_response(ok, results);
    }

    void exception(const Exception& ex)
    {
        _cb->ice_exception(ex);
    }

    void sent(bool)
    {
        _cb->ice_response(true, vector<Byte>()); // For oneway invocations
    }

private:

    const AMD_Object_ice_invokePtr _cb;
};
typedef IceUtil::Handle<Invocation> InvocationPtr;

//
// Holds information about an incoming invocation that's been queued until an outgoing connection has
// been established.
//
struct QueuedInvocation : public IceUtil::Shared
{
    //
    // The pointers in paramData refer to the Ice marshaling buffer and won't remain valid after
    // ice_invoke_async completes, so we have to make a copy of the parameter data.
    //
    QueuedInvocation(const AMD_Object_ice_invokePtr& c, const pair<const Byte*, const Byte*>& p, const Current& curr) :
        cb(c), paramData(p.first, p.second), current(curr)
    {
    }

    const AMD_Object_ice_invokePtr cb;
    const vector<Byte> paramData;
    const Current current;
};
typedef IceUtil::Handle<QueuedInvocation> QueuedInvocationPtr;

//
// Relays heartbeat messages.
//
class HeartbeatCallbackI : public HeartbeatCallback
{
public:

    HeartbeatCallbackI(const ConnectionPtr&);

    virtual void heartbeat(const ConnectionPtr&);

private:

    const ConnectionPtr _connection;
};

class BridgeI;
typedef IceUtil::Handle<BridgeI> BridgeIPtr;

class BridgeConnection;
typedef IceUtil::Handle<BridgeConnection> BridgeConnectionPtr;

class CloseCallbackI : public CloseCallback
{
public:

    CloseCallbackI(const BridgeIPtr&);

    virtual void closed(const ConnectionPtr&);

private:

    const BridgeIPtr _bridge;
};

class GetConnectionCallback : public IceUtil::Shared
{
public:

    GetConnectionCallback(const BridgeIPtr&, const BridgeConnectionPtr&);
    void success(const ConnectionPtr&);
    void exception(const Exception&);

private:

    const BridgeIPtr _bridge;
    const BridgeConnectionPtr _bc;
};

//
// Allows the bridge to be used as an Ice router.
//
class RouterI : public Router
{
public:

    virtual ObjectPrx getClientProxy(IceUtil::Optional<bool>& hasRoutingTable, const Current&) const
    {
        hasRoutingTable = false; // We don't maintain a routing table, no need to call addProxies on this impl.
        return 0;
    }

    virtual ObjectPrx getServerProxy(const Current&) const
    {
        return 0;
    }

    virtual ObjectProxySeq addProxies(const ObjectProxySeq&, const Current&)
    {
        return ObjectProxySeq();
    }
};

class FinderI : public RouterFinder
{
public:

    FinderI(const RouterPrx& router) :
        _router(router)
    {
    }

    virtual RouterPrx getRouter(const Current&)
    {
        return _router;
    }

private:

    const RouterPrx _router;
};

//
// Represents a pair of bridged connections.
//
class BridgeConnection : public IceUtil::Shared
{
public:

    BridgeConnection(const ObjectAdapterPtr&, const ObjectPrx&, const ConnectionPtr&);

    void outgoingSuccess(const ConnectionPtr&);
    void outgoingException(const Exception&);

    void closed(const ConnectionPtr&);
    void dispatch(const AMD_Object_ice_invokePtr&, const pair<const Byte*, const Byte*>&, const Current&);

private:

    void send(const ConnectionPtr&,
              const AMD_Object_ice_invokePtr&,
              const pair<const Byte*, const Byte*>&,
              const Current&);

    const ObjectAdapterPtr _adapter;
    const ObjectPrx _target;
    const ConnectionPtr _incoming;

    IceUtil::Mutex _lock;
    ConnectionPtr _outgoing;
    IceInternal::UniquePtr<Exception> _exception;

    //
    // We maintain our own queue for invocations that arrive on the incoming connection before the outgoing
    // connection has been established. We don't want to forward these to proxies and let the proxies handle
    // the queuing because then the invocations could be sent out of order (e.g., when invocations are split
    // among twoway/oneway/datagram proxies).
    //
    vector<QueuedInvocationPtr> _queue;
};

//
// The main bridge servant.
//
class BridgeI : public Ice::BlobjectArrayAsync
{
public:

    BridgeI(const ObjectAdapterPtr& adapter, const ObjectPrx& target);

    virtual void ice_invoke_async(const AMD_Object_ice_invokePtr&,
                                  const std::pair<const Byte*, const Byte*>&,
                                  const Current&);

    void closed(const ConnectionPtr&);
    void outgoingSuccess(const BridgeConnectionPtr&, const ConnectionPtr&);
    void outgoingException(const BridgeConnectionPtr&, const Exception&);

private:

    const ObjectAdapterPtr _adapter;
    const ObjectPrx _target;

    IceUtil::Mutex _lock;
    map<ConnectionPtr, BridgeConnectionPtr> _connections;
};

class BridgeService : public Service
{
public:

    BridgeService();

protected:

    virtual bool start(int, char*[], int&);
    virtual bool stop();
    virtual CommunicatorPtr initializeCommunicator(int&, char*[], const InitializationData&, int);

private:

    void usage(const std::string&);
};

}

HeartbeatCallbackI::HeartbeatCallbackI(const ConnectionPtr& con) :
    _connection(con)
{
}

void
HeartbeatCallbackI::heartbeat(const ConnectionPtr&)
{
    //
    // When a connection receives a heartbeat message, we send one over its corresponding connection.
    //
    try
    {
        _connection->begin_heartbeat();
    }
    catch(...)
    {
    }
}

CloseCallbackI::CloseCallbackI(const BridgeIPtr& bridge) :
    _bridge(bridge)
{
}

void
CloseCallbackI::closed(const ConnectionPtr& con)
{
    _bridge->closed(con);
}

GetConnectionCallback::GetConnectionCallback(const BridgeIPtr& bridge, const BridgeConnectionPtr& bc) :
    _bridge(bridge), _bc(bc)
{
}

void
GetConnectionCallback::success(const ConnectionPtr& outgoing)
{
    _bridge->outgoingSuccess(_bc, outgoing);
}

void
GetConnectionCallback::exception(const Exception& ex)
{
    _bridge->outgoingException(_bc, ex);
}

BridgeConnection::BridgeConnection(const ObjectAdapterPtr& adapter, const ObjectPrx& target, const ConnectionPtr& inc) :
    _adapter(adapter), _target(target), _incoming(inc)
{
}

void
BridgeConnection::outgoingSuccess(const ConnectionPtr& outgoing)
{
    IceUtil::Mutex::Lock lock(_lock);
    assert(!_outgoing && outgoing);
    if(_exception)
    {
        //
        // The incoming connection is already closed. There's no point in leaving the outgoing
        // connection open.
        //
        outgoing->close(ICE_SCOPED_ENUM(ConnectionClose, Gracefully));
        return;
    }

    _outgoing = outgoing;

    //
    // Register hearbeat callbacks on both connections.
    //
    _incoming->setHeartbeatCallback(new HeartbeatCallbackI(_outgoing));
    _outgoing->setHeartbeatCallback(new HeartbeatCallbackI(_incoming));

    //
    // Configure the outgoing connection for bidirectional requests.
    //
    _outgoing->setAdapter(_adapter);

    //
    // Flush any queued invocations.
    //
    for(vector<QueuedInvocationPtr>::const_iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        pair<const Byte*, const Byte*> paramData(&(*p)->paramData[0], &(*p)->paramData[0] + (*p)->paramData.size());
        send(outgoing, (*p)->cb, paramData, (*p)->current);
    }
    _queue.clear();
}

void
BridgeConnection::outgoingException(const Exception& ex)
{
    IceUtil::Mutex::Lock lock(_lock);
    if(_exception)
    {
        return;
    }
    _exception.reset(ex.ice_clone());

    //
    // The outgoing connection failed so we close the incoming connection. closed() will eventually
    // be called for it when the connection's dispatch count reaches zero.
    //
    _incoming->close(ICE_SCOPED_ENUM(ConnectionClose, Gracefully));

    //
    // Complete the queued incoming invocations, otherwise the incoming connection will never
    // complete its graceful closure. This is only necessary on the server side.
    //
    // The client will receive an UnknownLocalException whose reason member contains information
    // about the failure.
    //
    for(vector<QueuedInvocationPtr>::iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        (*p)->cb->ice_exception(ex);
    }
    _queue.clear();
}

void
BridgeConnection::dispatch(const AMD_Object_ice_invokePtr& cb, const pair<const Byte*, const Byte*>& paramData,
                           const Current& current)
{
    //
    // We've received an invocation, either from the client via the incoming connection, or from
    // the server via the outgoing (bidirectional) connection. The current.con member tells us
    // the connection over which the request arrived.
    //
    IceUtil::Mutex::Lock lock(_lock);
    if(_exception)
    {
        cb->ice_exception(*_exception.get());
    }
    else if(!_outgoing)
    {
        //
        // Queue the invocation until the outgoing connection is established.
        //
        assert(current.con == _incoming);
        _queue.push_back(new QueuedInvocation(cb, paramData, current));
    }
    else
    {
        send(current.con == _incoming ? _outgoing : _incoming, cb, paramData, current);
    }
}

void
BridgeConnection::send(const ConnectionPtr& dest,
                       const AMD_Object_ice_invokePtr& cb,
                       const pair<const Byte*, const Byte*>& paramData,
                       const Current& current)
{
    try
    {
        //
        // Create a proxy having the same identity as the request.
        //
        ObjectPrx prx = dest->createProxy(current.id);

        //
        // Examine the request to determine whether it should be forwarded as a oneway or a twoway.
        //
        Callback_Object_ice_invokePtr d;
        if(!current.requestId)
        {
            if(prx->ice_isTwoway())
            {
                prx = prx->ice_oneway();
            }
            d = newCallback_Object_ice_invoke(new Invocation(cb), &Invocation::exception, &Invocation::sent);
        }
        else
        {
            d = newCallback_Object_ice_invoke(new Invocation(cb), &Invocation::success, &Invocation::exception);
        }
        prx->begin_ice_invoke(current.operation, current.mode, paramData, current.ctx, d);
    }
    catch(const std::exception& ex)
    {
        cb->ice_exception(ex);
    }
}

BridgeI::BridgeI(const ObjectAdapterPtr& adapter, const ObjectPrx& target) :
    _adapter(adapter), _target(target)
{
}

void
BridgeI::ice_invoke_async(const AMD_Object_ice_invokePtr& cb,
                          const std::pair<const Byte*, const Byte*>& paramData,
                          const Current& current)
{
    BridgeConnectionPtr bc;

    {
        IceUtil::Mutex::Lock lock(_lock);

        map<ConnectionPtr, BridgeConnectionPtr>::iterator p = _connections.find(current.con);
        if(p == _connections.end())
        {
            //
            // The connection is unknown to us, it must be a new incoming connection.
            //

            EndpointInfoPtr info = current.con->getEndpoint()->getInfo();

            //
            // Create a target proxy that matches the configuration of the incoming connection.
            //
            ObjectPrx target;
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

            bc = new BridgeConnection(_adapter, target, current.con);
            _connections.insert(make_pair(current.con, bc));
            current.con->setCloseCallback(new CloseCallbackI(this));

            //
            // Try to establish the outgoing connection.
            //
            try
            {
                //
                // Begin the connection establishment process asynchronously. This can take a while to complete,
                // especially when using Bluetooth.
                //
                Callback_Object_ice_getConnectionPtr d =
                    newCallback_Object_ice_getConnection(new GetConnectionCallback(this, bc),
                                                         &GetConnectionCallback::success,
                                                         &GetConnectionCallback::exception);
                target->begin_ice_getConnection(d);
            }
            catch(const Exception& ex)
            {
                cb->ice_exception(ex);
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
    bc->dispatch(cb, paramData, current);
}

void
BridgeI::closed(const ConnectionPtr& con)
{
    //
    // Notify the BridgeConnection that a connection has closed. We also need to remove it from our map.
    //
    BridgeConnectionPtr bc;
    {
        IceUtil::Mutex::Lock lock(_lock);
        map<ConnectionPtr, BridgeConnectionPtr>::iterator p = _connections.find(con);
        assert(p != _connections.end());
        bc = p->second;
        _connections.erase(p);
    }
    assert(bc && con);
    bc->closed(con);
}

void
BridgeConnection::closed(const ConnectionPtr& con)
{
    IceUtil::Mutex::Lock lock(_lock);
    if(_exception)
    {
        return; // Nothing to do if the exception is already set, both connections have been closed already.
    }

    ConnectionPtr toBeClosed = con == _incoming ? _outgoing : _incoming;
    try
    {
        con->throwException();
    }
    catch(const Ice::CloseConnectionException& ex)
    {
        _exception.reset(ex.ice_clone());
        if(toBeClosed)
        {
            toBeClosed->close(ICE_SCOPED_ENUM(ConnectionClose, Gracefully));
        }
    }
    catch(const Ice::Exception& ex)
    {
        _exception.reset(ex.ice_clone());
        if(toBeClosed)
        {
            toBeClosed->close(ICE_SCOPED_ENUM(ConnectionClose, Forcefully));
        }
    }

    //
    // Even though the connection is already closed, we still need to "complete" the pending invocations so
    // that the connection's dispatch count is updated correctly.
    //
    for(vector<QueuedInvocationPtr>::iterator p = _queue.begin(); p != _queue.end(); ++p)
    {
        (*p)->cb->ice_exception(*_exception.get());
    }
    _queue.clear();
}

void
BridgeI::outgoingSuccess(const BridgeConnectionPtr& bc, const ConnectionPtr& outgoing)
{
    //
    // An outgoing connection was established. Notify the BridgeConnection object.
    //
    {
        IceUtil::Mutex::Lock lock(_lock);
        _connections.insert(make_pair(outgoing, bc));
        outgoing->setCloseCallback(new CloseCallbackI(this));
    }
    bc->outgoingSuccess(outgoing);
}

void
BridgeI::outgoingException(const BridgeConnectionPtr& bc, const Exception& ex)
{
    //
    // An outgoing connection attempt failed. Notify the BridgeConnection object.
    //
    bc->outgoingException(ex);
}

BridgeService::BridgeService()
{
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

    PropertiesPtr properties = communicator()->getProperties();

    const string targetProperty = "IceBridge.Target.Endpoints";
    const string targetEndpoints = properties->getProperty(targetProperty);
    if(targetEndpoints.empty())
    {
        error("property '" + targetProperty + "' is not set");
        return false;
    }

    Ice::ObjectPrx target;

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

    ObjectAdapterPtr adapter = communicator()->createObjectAdapter("IceBridge.Source");

    adapter->addDefaultServant(new BridgeI(adapter, target), "");

    string instanceName = properties->getPropertyWithDefault("IceBridge.InstanceName", "IceBridge");
    RouterPrx router = RouterPrx::uncheckedCast(adapter->add(new RouterI, stringToIdentity(instanceName + "/router")));
    adapter->add(new FinderI(router), stringToIdentity("Ice/RouterFinder"));

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

CommunicatorPtr
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
