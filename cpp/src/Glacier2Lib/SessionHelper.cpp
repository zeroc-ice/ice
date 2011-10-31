// **********************************************************************
//
// Copyright (c) 2003-2011 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Glacier2/SessionHelper.h>

#include <IceUtil/UUID.h>
#include <IceUtil/CountDownLatch.h>

#include <Ice/Ice.h>

#include <algorithm> // required by max
#include <memory> // required by auto_ptr

using namespace std;

namespace
{
    
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

}

Glacier2::SessionRefreshThread::SessionRefreshThread(const Glacier2::SessionHelperPtr& session, 
                                                     const Glacier2::RouterPrx& router, long period) :
    _cb(Glacier2::newCallback_Router_refreshSession(this, &SessionRefreshThread::success, 
                                                    &SessionRefreshThread::failure)),
    _session(session),
    _router(router),
    _period(period),
    _done(false)
{
}

void
Glacier2::SessionRefreshThread::run()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    while(true)
    {
        try
        {
            _router->begin_refreshSession(_cb);
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
            _monitor.timedWait(IceUtil::Time::seconds((int)_period));
        }

        if(_done)
        {
            break;
        }
    }
}

void
Glacier2::SessionRefreshThread::done()
{
    IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
    if(!_done)
    {
        _done = true;
        _monitor.notify();
    }
}

void
Glacier2::SessionRefreshThread::success()
{
}

void
Glacier2::SessionRefreshThread::failure(const Ice::Exception&)
{
    done();
    _session->destroy();
}

class Glacier2::DestroyInternal : public IceUtil::Thread
{

public:
    
    DestroyInternal(const Glacier2::SessionHelperPtr& session, const Glacier2::SessionCallbackPtr& callback) :
        _session(session),
        _disconnected(new Disconnected(session, callback))
    {
    }
    
    virtual void run()
    {
        _session->destroyInternal(_disconnected);
    }
    
private:
    
    const Glacier2::SessionHelperPtr _session;
    const Ice::DispatcherCallPtr _disconnected;
};

Glacier2::SessionHelper::SessionHelper(const Glacier2::SessionCallbackPtr& callback, 
                                       const Ice::InitializationData& initData) :
    _connected(false),
    _destroy(false),
    _initData(initData),
    _callback(callback)
{
}

void
Glacier2::SessionHelper::destroy()
{
    IceUtil::Mutex::Lock sync(_mutex);
    destroy(new Glacier2::DestroyInternal(this, _callback));
}

void
Glacier2::SessionHelper::destroy(const IceUtil::ThreadPtr& destroyInternal)
{
    if(_destroy)
    {
        return;
    }
    _destroy = true;

    if(!_refreshThread)
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

    //
    // Run the destroyInternal in a thread. This is because it
    // destroyInternal makes remote invocations.
    //
    destroyInternal->start();
}

Ice::CommunicatorPtr
Glacier2::SessionHelper::communicator() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _communicator;
}

string
Glacier2::SessionHelper::categoryForClient() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_router)
    {
        throw SessionNotExistException();
    }

    return _category;
}

Ice::ObjectPrx
Glacier2::SessionHelper::addWithUUID(const Ice::ObjectPtr& servant)
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_router)
    {
        throw SessionNotExistException();
    }
    Ice::Identity id;
    id.name = IceUtil::generateUUID();
    id.category = _category;
    return internalObjectAdapter()->add(servant, id);
}

Glacier2::SessionPrx
Glacier2::SessionHelper::session() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    if(!_session)
    {
        throw new SessionNotExistException();
    }
    return _session;
}

bool
Glacier2::SessionHelper::isConnected() const
{
    IceUtil::Mutex::Lock sync(_mutex);
    return _connected;
}

Ice::ObjectAdapterPtr
Glacier2::SessionHelper::objectAdapter()
{
    IceUtil::Mutex::Lock sync(_mutex);
    return internalObjectAdapter();
}

bool
Glacier2::SessionHelper::operator==(const SessionHelper& other) const
{    
    return this == &other;
}

bool
Glacier2::SessionHelper::operator!=(const SessionHelper& other) const
{    
    return this != &other;
}

Ice::ObjectAdapterPtr
Glacier2::SessionHelper::internalObjectAdapter()
{
    if(!_router)
    {
        throw SessionNotExistException();
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

class ConnectStrategySecureConnection : public Glacier2::ConnectStrategy
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

class ConnectStrategyUserPassword : public Glacier2::ConnectStrategy
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
Glacier2::SessionHelper::connect(const map<string, string>& context)
{
    connectImpl(new ConnectStrategySecureConnection(context));
}

void
Glacier2::SessionHelper::connect(const string& user, const string& password, const map<string, string>& context)
{
    connectImpl(new ConnectStrategyUserPassword(user, password, context));
}

void
Glacier2::SessionHelper::destroyInternal(const Ice::DispatcherCallPtr& disconnected)
{
    assert(_destroy);
    Ice::CommunicatorPtr communicator;
    Glacier2::RouterPrx router;
    Glacier2::SessionRefreshThreadPtr refreshThread;
    {
        IceUtil::Mutex::Lock sync(_mutex);
        router = _router;
        _router = 0;
        _connected = false;

        refreshThread = _refreshThread;
        _refreshThread = 0;
        
        communicator = _communicator;
        _communicator = 0;
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
        catch(const Ice::Exception& ex)
        {
            //
            // Not expected.
            //
            ostringstream os;
            os << "SessionHelper: unexpected exception when destroying the session:\n";
            os << ex;
            if(communicator)
            {
                communicator->getLogger()->warning(os.str());
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
        _ex.reset(dynamic_cast<Ice::Exception*>(ex.ice_clone()));
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
    std::auto_ptr<Ice::Exception> _ex;
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

}

class Glacier2::ConnectThread : public IceUtil::Thread
{

public:
    
    ConnectThread(const Glacier2::SessionCallbackPtr& callback, const Glacier2::SessionHelperPtr& session,
                  const Glacier2::ConnectStrategyPtr& factory, const Ice::CommunicatorPtr& communicator) :
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
                    _communicator->destroy();
                }
                catch(...)
                {
                }

                _session->dispatchCallback(new ConnectFailed(_callback, _session, ex), 0);
            }
    }
    
private:
    
    const Glacier2::SessionCallbackPtr _callback;
    const Glacier2::SessionHelperPtr _session;
    const Glacier2::ConnectStrategyPtr _factory;
    const Ice::CommunicatorPtr _communicator;
};

void
Glacier2::SessionHelper::connectImpl(const ConnectStrategyPtr& factory)
{
    assert(!_destroy);

    try
    {
        _communicator = Ice::initialize(_initData);
    }
    catch(const Ice::LocalException& ex)
    {
        _destroy = true;
        dispatchCallback(new ConnectFailed(_callback, this, ex), 0);
        return;
    }

    IceUtil::ThreadPtr connectThread = new ConnectThread(_callback, this, factory, _communicator);
    connectThread->start();
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
Glacier2::SessionHelper::connected(const Glacier2::RouterPrx& router, const Glacier2::SessionPrx& session)
{
    //
    // Remote invocation should be done without acquire a mutex lock.
    //
    assert(router);
    Ice::ConnectionPtr conn = router->ice_getCachedConnection();
    string category = router->getCategoryForClient();
    long timeout = router->getSessionTimeout();
    
    {
        IceUtil::Mutex::Lock sync(_mutex);
        _router = router;

        if(_destroy)
        {
            //
            // Run the destroyInternal in a thread. This is because it
            // destroyInternal makes remote invocations.
            //
            IceUtil::ThreadPtr thread = new Glacier2::DestroyInternal(this, _callback);
            thread->start();
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
Glacier2::SessionHelper::dispatchCallback(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& conn)
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
Glacier2::SessionHelper::dispatchCallbackAndWait(const Ice::DispatcherCallPtr& call, const Ice::ConnectionPtr& conn)
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
