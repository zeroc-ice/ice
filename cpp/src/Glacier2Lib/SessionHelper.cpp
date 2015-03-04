// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <Glacier2/SessionHelper.h>

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>

#include <algorithm> // required by max

using namespace std;

namespace
{

class ConnectStrategy : public IceUtil::Shared
{

public:
    
    virtual Glacier2::SessionPrx connect(const Glacier2::RouterPrx& router) = 0;    
};
typedef IceUtil::Handle< ConnectStrategy> ConnectStrategyPtr;

class Disconnected : public Ice::DispatcherCall
{

public:
    
    Disconnected(const Glacier2::SessionHelperPtr& session, const Glacier2::SessionCallbackPtr& callback) :
        _session(session),
        _callback(callback)
    {
    }
    
    
    virtual
    void run()
    {

        _callback->disconnected(_session);
    }
    
private:

    const Glacier2::SessionHelperPtr _session;
    const Glacier2::SessionCallbackPtr _callback;
};

class SessionRefreshThread : public IceUtil::Thread
{

public:
    
    SessionRefreshThread(const Glacier2::SessionHelperPtr&, const Glacier2::RouterPrx&, Ice::Long);
    virtual void run();
    void done();
    void success();
    void failure(const Ice::Exception&);
    
private:
    
    const Glacier2::SessionHelperPtr _session;
    const Glacier2::RouterPrx _router;
    Ice::Long _period;
    bool _done;
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};
typedef IceUtil::Handle<SessionRefreshThread> SessionRefreshThreadPtr;

class SessionHelperI : public Glacier2::SessionHelper
{
    
public:
    
    SessionHelperI(const Glacier2::SessionCallbackPtr&, const Ice::InitializationData&);
    void destroy();
    Ice::CommunicatorPtr communicator() const;
    std::string categoryForClient() const;
    Ice::ObjectPrx addWithUUID(const Ice::ObjectPtr&);
    Glacier2::SessionPrx session() const;
    bool isConnected() const;
    Ice::ObjectAdapterPtr objectAdapter();
    
    friend class DestroyInternal;
    friend class ConnectThread;
    friend class DispatchCallThread;
    friend class Glacier2::SessionFactoryHelper;

private:
    
    void destroy(const IceUtil::ThreadPtr&);

    Ice::ObjectAdapterPtr internalObjectAdapter();
    void connected(const Glacier2::RouterPrx&, const Glacier2::SessionPrx&);
    void destroyInternal(const Ice::DispatcherCallPtr&);
    void connectFailed();
    
    void connect(const std::map<std::string, std::string>&);
    void connect(const std::string&, const std::string&, const std::map<std::string, std::string>&);
    
    void connectImpl(const ConnectStrategyPtr&);
    void dispatchCallback(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);
    void dispatchCallbackAndWait(const Ice::DispatcherCallPtr&, const Ice::ConnectionPtr&);

    IceUtil::Mutex _mutex;
    Ice::CommunicatorPtr _communicator;
    Ice::ObjectAdapterPtr _adapter;
    Glacier2::RouterPrx _router;
    Glacier2::SessionPrx _session;
    SessionRefreshThreadPtr _refreshThread;
    std::string _category;
    bool _connected;
    bool _destroy;
    const Ice::InitializationData _initData;
    const Glacier2::SessionCallbackPtr _callback;
};
typedef IceUtil::Handle<SessionHelperI> SessionHelperIPtr;

SessionRefreshThread::SessionRefreshThread(const Glacier2::SessionHelperPtr& session, 
                                           const Glacier2::RouterPrx& router, Ice::Long period) :
    _session(session),
    _router(router),
    _period(period),
    _done(false)
{
}

void
SessionRefreshThread::run()
{
    Glacier2::Callback_Router_refreshSessionPtr cb = 
        Glacier2::newCallback_Router_refreshSession(this, &SessionRefreshThread::failure);
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    while(true)
    {
        try
        {
            _router->begin_refreshSession(cb);
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            //
            // AMI requests can raise CommunicatorDestroyedException directly.
            //
            break;
        }

        if(!_done)
        {
            _monitor.timedWait(IceUtil::Time::seconds(_period));
        }

        if(_done)
        {
            break;
        }
    }
}

void
SessionRefreshThread::done()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    if(!_done)
    {
        _done = true;
        _monitor.notify();
    }
}

void
SessionRefreshThread::failure(const Ice::Exception&)
{
    done();
    _session->destroy();
}

class DestroyInternal : public IceUtil::Thread
{

public:
    
    DestroyInternal(const SessionHelperIPtr& session, const Glacier2::SessionCallbackPtr& callback) :
        _session(session),
        _disconnected(new Disconnected(session, callback))
    {
    }
    
    virtual void run()
    {
        _session->destroyInternal(_disconnected);
    }
    
private:
    
    const SessionHelperIPtr _session;
    const Ice::DispatcherCallPtr _disconnected;
};

}

SessionHelperI::SessionHelperI(const Glacier2::SessionCallbackPtr& callback, 
                               const Ice::InitializationData& initData) :
    _connected(false),
    _destroy(false),
    _initData(initData),
    _callback(callback)
{
}

void
SessionHelperI::destroy()
{
    IceUtil::Mutex::Lock sync(_mutex);
    destroy(new DestroyInternal(this, _callback));
}

void
SessionHelperI::destroy(const IceUtil::ThreadPtr& destroyInternal)
{
    if(_destroy)
    {
        return;
    }
    _destroy = true;

    if(!_connected)
    {
        //
        // In this case a connecting session is being
        // destroyed. The communicator and session will be
        // destroyed when the connection establishment has
        // completed.
        //
        return;
    }
    _session = 0;
    _connected = false;

    //
    // Run the destroyInternal in a thread. This is because it
    // destroyInternal makes remote invocations.
    //
    destroyInternal->start().detach();
}

Ice::CommunicatorPtr
SessionHelperI::communicator() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _communicator;
}

string
SessionHelperI::categoryForClient() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_router)
    {
        throw Glacier2::SessionNotExistException();
    }
    return _category;
}

Ice::ObjectPrx
SessionHelperI::addWithUUID(const Ice::ObjectPtr& servant)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_router)
    {
        throw Glacier2::SessionNotExistException();
    }
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    id.category = _category;
    return internalObjectAdapter()->add(servant, id);
}

Glacier2::SessionPrx
SessionHelperI::session() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_session)
    {
        throw new Glacier2::SessionNotExistException();
    }
    return _session;
}

bool
SessionHelperI::isConnected() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _connected;
}

Ice::ObjectAdapterPtr
SessionHelperI::objectAdapter()
{
    IceUtil::Mutex::Lock sync(_mutex);
    return internalObjectAdapter();
}

bool
Glacier2::SessionHelper::operator==(const Glacier2::SessionHelper& other) const
{    
    return this == &other;
}

bool
Glacier2::SessionHelper::operator!=(const Glacier2::SessionHelper& other) const
{    
    return this != &other;
}

Ice::ObjectAdapterPtr
SessionHelperI::internalObjectAdapter()
{
    if(!_router)
    {
        throw Glacier2::SessionNotExistException();
    }
    if(!_adapter)
    {
        _adapter = _communicator->createObjectAdapterWithRouter("", _router);
        _adapter->activate();
    }
    return _adapter;
}

namespace
{

class ConnectStrategySecureConnection : public ConnectStrategy
{

public:
    
    ConnectStrategySecureConnection(const map<string, string>& context) :
        _context(context)
    {
    }
    
    virtual Glacier2::SessionPrx
    connect(const Glacier2::RouterPrx& router)
    {
        return router->createSessionFromSecureConnection(_context);
    }
    
private:
    
    const map<string, string> _context;
};

class ConnectStrategyUserPassword : public ConnectStrategy
{

public:
    
    ConnectStrategyUserPassword(const string& user, const string& password, const map<string, string>& context) :
        _user(user),
        _password(password),
        _context(context)
    {
    }
    
    virtual Glacier2::SessionPrx
    connect(const Glacier2::RouterPrx& router)
    {
        return router->createSession(_user, _password, _context);
    }
    
private:
    
    const string _user;
    const string _password;
    const map<string, string> _context;
};

}

void
SessionHelperI::connect(const map<string, string>& context)
{
    IceUtil::Mutex::Lock sync(_mutex);
    connectImpl(new ConnectStrategySecureConnection(context));
}

void
SessionHelperI::connect(const string& user, const string& password, const map<string, string>& context)
{
    IceUtil::Mutex::Lock sync(_mutex);
    connectImpl(new ConnectStrategyUserPassword(user, password, context));
}

void
SessionHelperI::destroyInternal(const Ice::DispatcherCallPtr& disconnected)
{
    assert(_destroy);
    Ice::CommunicatorPtr communicator;
    Glacier2::RouterPrx router;
    SessionRefreshThreadPtr refreshThread;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        router = _router;
        _router = 0;
        _connected = false;

        refreshThread = _refreshThread;
        _refreshThread = 0;
        
        communicator = _communicator;
    }
    
    if(router)
    {
        try
        {
            router->destroySession();
        }
        catch(const Ice::ConnectionLostException&)
        {
            //
            // Expected if another thread invoked on an object from the session concurrently.
            //
        }
        catch(const Glacier2::SessionNotExistException&)
        {
            //
            // This can also occur.
            //
        }
        catch(const std::exception& ex)
        {
            //
            // Not expected.
            //
            if(communicator)
            {
                Ice::Warning warn(communicator->getLogger());
                warn << "SessionHelper: unexpected exception when destroying the session:\n" << ex;
            }
        }
    }
    
    if(refreshThread)
    {
        refreshThread->done();
        refreshThread->getThreadControl().join();
        refreshThread = 0;
    }
    
    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(...)
        {
        }
        communicator = 0;
    }
    dispatchCallback(disconnected, 0);
}

void
SessionHelperI::connectFailed()
{
    Ice::CommunicatorPtr communicator;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        communicator = _communicator;
    }
    
    if(communicator)
    {
        try
        {
            communicator->destroy();
        }
        catch(...)
        {
        }
    }
}

namespace
{
    
class ConnectFailed : public Ice::DispatcherCall
{

public:
    
    ConnectFailed(const Glacier2::SessionCallbackPtr& callback, const Glacier2::SessionHelperPtr& session,
                  const Ice::Exception& ex) :
        _callback(callback),
        _session(session)
    {
        _ex.reset(ex.ice_clone());
    }
    
    virtual void
    run()
    {
        const Ice::Exception* ex(_ex.get());
        _callback->connectFailed(_session, *ex);
    }
    
private:
    
    const Glacier2::SessionCallbackPtr _callback;
    const Glacier2::SessionHelperPtr _session;
    IceUtil::UniquePtr<Ice::Exception> _ex;
};

class CreatedCommunicator : public Ice::DispatcherCall
{
    
public:
    
    CreatedCommunicator(const Glacier2::SessionCallbackPtr& callback, const Glacier2::SessionHelperPtr& session) :
        _callback(callback),
        _session(session)
    {
    }
    
    virtual void
    run()
    {
        _callback->createdCommunicator(_session);
    }

private:
    
    const Glacier2::SessionCallbackPtr _callback;
    const Glacier2::SessionHelperPtr _session;
};

class ConnectThread : public IceUtil::Thread
{

public:
    
    ConnectThread(const Glacier2::SessionCallbackPtr& callback, const SessionHelperIPtr& session,
                  const ConnectStrategyPtr& factory, const Ice::CommunicatorPtr& communicator) :
        _callback(callback),
        _session(session),
        _factory(factory),
        _communicator(communicator)
    {
    }
    
    virtual void
    run()
    {
            try
            {
                _session->dispatchCallbackAndWait(new CreatedCommunicator(_callback, _session), 0);
                Glacier2::RouterPrx routerPrx = Glacier2::RouterPrx::uncheckedCast(_communicator->getDefaultRouter());
                Glacier2::SessionPrx session = _factory->connect(routerPrx);
                _session->connected(routerPrx, session);
            }
            catch(const Ice::Exception& ex)
            {
                try
                {
                    _session->connectFailed();
                }
                catch(...)
                {
                }

                _session->dispatchCallback(new ConnectFailed(_callback, _session, ex), 0);
            }
    }
    
private:
    
    const Glacier2::SessionCallbackPtr _callback;
    const SessionHelperIPtr _session;
    const ConnectStrategyPtr _factory;
    const Ice::CommunicatorPtr _communicator;
};


class DispatchCallThread : public IceUtil::Thread
{

public:
    
    DispatchCallThread(const SessionHelperIPtr& session, const Ice::DispatcherCallPtr& call,
                       const Ice::ConnectionPtr& conn) :
        _session(session),
        _call(call),
        _conn(conn)
    {
    }
    
    virtual void run()
    {
        _session->dispatchCallback(_call, _conn);
    }
    
private:
    
    const SessionHelperIPtr _session;
    const Ice::DispatcherCallPtr _call;
    const Ice::ConnectionPtr _conn;
};

}

void
SessionHelperI::connectImpl(const ConnectStrategyPtr& factory)
{
    assert(!_destroy);

    try
    {
        _communicator = Ice::initialize(_initData);
    }
    catch(const Ice::LocalException& ex)
    {
        _destroy = true;
        IceUtil::ThreadPtr thread = new DispatchCallThread(this, new ConnectFailed(_callback, this, ex), 0);
        thread->start().detach();
        return;
    }

    IceUtil::ThreadPtr connectThread = new ConnectThread(_callback, this, factory, _communicator);
    connectThread->start().detach();
}

namespace
{
    
class Connected : public Ice::DispatcherCall
{

public:
    
    Connected(const Glacier2::SessionCallbackPtr& callback, const Glacier2::SessionHelperPtr& session) :
        _callback(callback),
        _session(session)
    {
    }
    
    virtual void
    run()
    {
        try
        {
            _callback->connected(_session);
        }
        catch(const Glacier2::SessionNotExistException&)
        {
            _session->destroy();
        }
    }

private:
    
    const Glacier2::SessionCallbackPtr _callback;
    const Glacier2::SessionHelperPtr _session;
};

}

void
SessionHelperI::connected(const Glacier2::RouterPrx& router, const Glacier2::SessionPrx& session)
{
    //
    // Remote invocation should be done without acquire a mutex lock.
    //
    assert(router);
    Ice::ConnectionPtr conn = router->ice_getCachedConnection();
    string category = router->getCategoryForClient();
    Ice::Long timeout = router->getSessionTimeout();
    
    {
        IceUtil::Mutex::Lock sync(_mutex);
        _router = router;

        if(_destroy)
        {
            //
            // Run the destroyInternal in a thread. This is because it
            // destroyInternal makes remote invocations.
            //
            IceUtil::ThreadPtr thread = new DestroyInternal(this, _callback);
            thread->start().detach();
            return;
        }

        //
        // Cache the category.
        //
        _category = category;

        //
        // Assign the session after _destroy is checked.
        //
        _session = session;
        _connected = true;

        assert(!_refreshThread);
        
        if(timeout > 0)
        {
            _refreshThread = new SessionRefreshThread(this, _router, (timeout)/2);
            _refreshThread->start();
        }
    }
    dispatchCallback(new Connected(_callback, this), conn);
}

void
SessionHelperI::dispatchCallback(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& conn)
{
    if(_initData.dispatcher)
    {
        _initData.dispatcher->dispatch(call, conn);
    }
    else
    {
        call->run();
    }
    
}

namespace
{
    
class DispatcherCallWait : public Ice::DispatcherCall
{

public:
    
    DispatcherCallWait(IceUtilInternal::CountDownLatch& cdl, const Ice::DispatcherCallPtr& call) :
        _cdl(cdl),
        _call(call)
    {
    }
    
    virtual void
    run()
    {
        _call->run();
        _cdl.countDown();
    }
    
private:
    
    IceUtilInternal::CountDownLatch& _cdl;
    const Ice::DispatcherCallPtr _call;
};

}

void
SessionHelperI::dispatchCallbackAndWait(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& conn)
{
    if(_initData.dispatcher)
    {
        IceUtilInternal::CountDownLatch cdl(1);
        Ice::DispatcherCallPtr callWait = new DispatcherCallWait(cdl, call);
        _initData.dispatcher->dispatch(callWait, conn);
        cdl.await();
    }
    else
    {
        call->run();
    }
}

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const SessionCallbackPtr& callback) :
    _routerHost("localhost"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _callback(callback)
{
    _identity.name = "router";
    _identity.category = "Glacier2";
    _initData.properties = Ice::createProperties();
    setDefaultProperties();
}

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const Ice::InitializationData& initData,
                                           const SessionCallbackPtr& callback) :
    _routerHost("localhost"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _initData(initData),
    _callback(callback)
{
    _identity.name = "router";
    _identity.category = "Glacier2";
    if(!initData.properties)
    {
        _initData.properties = Ice::createProperties();
    }
    setDefaultProperties();
}

Glacier2::SessionFactoryHelper::SessionFactoryHelper(const Ice::PropertiesPtr& properties, const SessionCallbackPtr& callback) :
    _routerHost("localhost"),
    _secure(true),
    _port(0),
    _timeout(10000),
    _callback(callback)
{
    if(!properties)
    {
        throw Ice::InitializationException(
            __FILE__, __LINE__, "Attempt to create a SessionFactoryHelper with a null Properties argument");
    }
    _identity.name = "router";
    _identity.category = "Glacier2";
    _initData.properties = properties;
    setDefaultProperties();
}

void
Glacier2::SessionFactoryHelper::setRouterIdentity(const Ice::Identity& identity)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _identity = identity;
}

Ice::Identity
Glacier2::SessionFactoryHelper::getRouterIdentity() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _identity;
}

void
Glacier2::SessionFactoryHelper::setRouterHost(const string& hostname)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _routerHost = hostname;
}

string
Glacier2::SessionFactoryHelper::getRouterHost() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _routerHost;
}

void
Glacier2::SessionFactoryHelper::setSecure(bool secure)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _secure = secure;
}

bool
Glacier2::SessionFactoryHelper::getSecure() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _secure;
}

void
Glacier2::SessionFactoryHelper::setTimeout(int timeout)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _timeout = timeout;
}

int
Glacier2::SessionFactoryHelper::getTimeout() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _timeout;
}

void
Glacier2::SessionFactoryHelper::setPort(int port)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _port = port;
}

int
Glacier2::SessionFactoryHelper::getPort() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _port;
}

Ice::InitializationData
Glacier2::SessionFactoryHelper::getInitializationData() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _initData;
}

void
Glacier2::SessionFactoryHelper::setConnectContext(map<string, string> context)
{
    IceUtil::Mutex::Lock sync(_mutex);
    _context = context;
}

Glacier2::SessionHelperPtr
Glacier2::SessionFactoryHelper::connect()
{
    IceUtil::Mutex::Lock sync(_mutex);
    SessionHelperIPtr session = new SessionHelperI(_callback, createInitData());
    session->connect(_context);
    return session;
}

Glacier2::SessionHelperPtr
Glacier2::SessionFactoryHelper::connect(const string& user,  const string& password)
{
    IceUtil::Mutex::Lock sync(_mutex);
    SessionHelperIPtr session = new SessionHelperI(_callback, createInitData());
    session->connect(user, password, _context);
    return session;
}

Ice::InitializationData
Glacier2::SessionFactoryHelper::createInitData()
{
    //
    // Clone the initialization data and properties.
    //
    Ice::InitializationData initData = _initData;
    initData.properties = initData.properties->clone();

    if(initData.properties->getProperty("Ice.Default.Router").size() == 0)
    {
        ostringstream os;
        os << "\"";
        
        //
        // TODO replace with identityToString, we cannot use the Communicator::identityToString
        // current implementation because we need to do that before the communicator has been
        // initialized.
        //
        if(!_identity.category.empty())
        {
            os << _identity.category << "/";
        }
        os << _identity.name;
        
        os << "\"";
        os << ":";
        if(_secure)
        {
            os << "ssl -p ";
        }
        else
        {
            os << "tcp -p ";
        }
        
        if(_port != 0)
        {
            os << _port;
        }
        else
        {
            if(_secure)
            {
                os << GLACIER2_SSL_PORT;
            }
            else
            {
                os << GLACIER2_TCP_PORT;
            }
        }

        os << " -h ";
        os << _routerHost;
        if(_timeout > 0)
        {
            os << " -t ";
            os << _timeout;
        }
        initData.properties->setProperty("Ice.Default.Router", os.str());
#ifndef ICE_OS_WINRT
        //
        // If using a secure connection setup the IceSSL plug-in, if IceSSL
        // plug-in has already been setup we don't want to override the
        // configuration so it can be loaded from a custom location.
        //
        if(_secure && initData.properties->getProperty("Ice.Plugin.IceSSL").empty())
        {
            initData.properties->setProperty("Ice.Plugin.IceSSL","IceSSL:createIceSSL");
        }
#endif
    }
    return initData;
}

void
Glacier2::SessionFactoryHelper::setDefaultProperties()
{
    assert(_initData.properties);
    _initData.properties->setProperty("Ice.ACM.Client", "0");
    _initData.properties->setProperty("Ice.RetryIntervals", "-1");
}
