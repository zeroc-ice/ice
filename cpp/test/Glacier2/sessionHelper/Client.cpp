// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/IceUtil.h>
#include <Ice/Ice.h>
#include <Glacier2/Glacier2.h>

#include <TestCommon.h>

#include <iostream>
#include <iomanip>
#include <list>

#include <Callback.h>

using namespace std;
using namespace Test;

namespace
{

class Notify
{
public:

    virtual ~Notify()
    {
    }

    virtual void notify() = 0;
};

Notify* instance = 0;

class Dispatcher : 
#ifndef ICE_CPP11_MAPPING
    public Ice::Dispatcher,
#endif
    public IceUtil::Thread
{

public:

    Dispatcher():
        _destroyed(false)
    {
    }

    virtual void dispatch(
#ifdef ICE_CPP11_MAPPING
        std::function<void()> call,
#else
        const Ice::DispatcherCallPtr& call,
#endif
        const Ice::ConnectionPtr&)
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        if(_queue.empty())
        {
            _monitor.notify();
        }
        _queue.push_back(call);
    }

    virtual void
    destroy()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        _destroyed = true;
        _monitor.notify();
    }

    virtual void
    run()
    {
        while(1)
        {
#ifdef ICE_CPP11_MAPPING
            auto item = nextItem();
            if (!item)
            {
                break;
            }
            item();
#else
            Ice::DispatcherCallPtr item = nextItem();
            if(!item)
            {
                break;
            }
            item->run();
#endif
        }
    }

private:

#ifdef ICE_CPP11_MAPPING
    std::function<void()>
#else
    Ice::DispatcherCallPtr
#endif
    nextItem()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        while(_queue.empty())
        {
            if(_destroyed)
            {
                return ICE_NULLPTR;
            }
            _monitor.wait();
        }
#ifdef ICE_CPP11_MAPPING
        auto item = _queue.front();
#else
        Ice::DispatcherCallPtr item = _queue.front();
#endif
        _queue.pop_front();
        return item;
    }


    IceUtil::Monitor<IceUtil::Mutex> _monitor;
#ifdef ICE_CPP11_MAPPING
    list<std::function<void()>> _queue;
#else
    list<Ice::DispatcherCallPtr> _queue;
#endif
    bool _destroyed;
};
typedef IceUtil::Handle<Dispatcher> DispatcherPtr;

class SuccessSessionCallback : public Glacier2::SessionCallback
{

public:

    virtual void
    connected(const Glacier2::SessionHelperPtr&)
    {
        cout << "ok" << endl;
        instance->notify();
    }

    virtual void
    disconnected(const Glacier2::SessionHelperPtr&)
    {
        cout << "ok" << endl;
        instance->notify();
    }

    virtual void
    connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception&)
    {
        test(false);
    }

    virtual void
    createdCommunicator(const Glacier2::SessionHelperPtr& session)
    {
        test(session->communicator());
    }
};

class AfterShutdownSessionCallback : public Glacier2::SessionCallback
{

public:

    virtual void
    connected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    disconnected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();;
        }
        catch(const Ice::ConnectFailedException&)
        {
            cout << "ok" << endl;
            instance->notify();
        }
        catch(...)
        {
            test(false);
        }
    }

    virtual void
    createdCommunicator(const Glacier2::SessionHelperPtr& session)
    {
        test(session->communicator());
    }
};

class FailSessionCallback : public Glacier2::SessionCallback
{

public:

    virtual void
    connected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    disconnected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Glacier2::PermissionDeniedException&)
        {
            cout << "ok" << endl;
            instance->notify();
        }
        catch(const Ice::LocalException&)
        {
            test(false);
        }
    }

    virtual void
    createdCommunicator(const Glacier2::SessionHelperPtr& session)
    {
        test(session->communicator());
    }
};

class InterruptConnectCallback : public Glacier2::SessionCallback
{

public:

    virtual void
    connected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    disconnected(const Glacier2::SessionHelperPtr&)
    {
        test(false);
    }

    virtual void
    connectFailed(const Glacier2::SessionHelperPtr&, const Ice::Exception& ex)
    {
        try
        {
            ex.ice_throw();
        }
        catch(const Ice::CommunicatorDestroyedException&)
        {
            cout << "ok" << endl;
            instance->notify();
        }
        catch(...)
        {
            test(false);
        }
    }

    virtual void
    createdCommunicator(const Glacier2::SessionHelperPtr& session)
    {
        test(session->communicator());
    }
};

class SessionHelperClient : public Ice::Application, public Notify
{
public:

    int run(int argc, char* argv[])
    {
        instance = this;
        string protocol = getTestProtocol(communicator()->getProperties());
        string host = getTestHost(communicator()->getProperties());
        _initData.properties = Ice::createProperties(argc, argv, communicator()->getProperties());
        _initData.properties->setProperty("Ice.Default.Router", "Glacier2/router:" +
                                          getTestEndpoint(communicator(), 10));

        DispatcherPtr dispatcher = new Dispatcher();
        dispatcher->start();
#ifdef ICE_CPP11_MAPPING
        _initData.dispatcher = [dispatcher](std::function<void()> call, const std::shared_ptr<Ice::Connection>& conn)
            {
                dispatcher->dispatch(call, conn);
            };
#else
        _initData.dispatcher = dispatcher;
#endif
        _factory = ICE_MAKE_SHARED(Glacier2::SessionFactoryHelper, _initData, ICE_MAKE_SHARED(FailSessionCallback));

        //
        // Test to create a session with wrong userid/password
        //

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);

            cout << "testing SessionHelper connect with wrong userid/password... " << flush;

            _session = _factory->connect("userid", "xxx");
            //
            // Wait for connectFailed callback
            //
            _monitor.timedWait(IceUtil::Time::seconds(30));

            test(!_session->isConnected());
        }
        _factory->destroy();

        //
        // Test to interrupt connection establishment
        //

        _initData.properties->setProperty("Ice.Default.Router", "");
        _factory = ICE_MAKE_SHARED(Glacier2::SessionFactoryHelper, _initData, ICE_MAKE_SHARED(InterruptConnectCallback));

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect interrupt... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(getTestPort(_initData.properties, 1));
            _factory->setProtocol(protocol);
            _session = _factory->connect("userid", "abc123");

            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
            _session->destroy();

            //
            // Wait for connectFailed callback
            //
            _monitor.timedWait(IceUtil::Time::seconds(30));
            test(!_session->isConnected());
        }
        _factory->destroy();

        _factory = ICE_MAKE_SHARED(Glacier2::SessionFactoryHelper, _initData, ICE_MAKE_SHARED(SuccessSessionCallback));

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(getTestPort(_initData.properties, 10));
            _factory->setProtocol(protocol);
            _session = _factory->connect("userid", "abc123");

            //
            // Wait for connect callback
            //
            _monitor.timedWait(IceUtil::Time::seconds(30));

            cout << "testing SessionHelper isConnected after connect... " << flush;
            test(_session->isConnected());
            cout << "ok" << endl;

            cout << "testing SessionHelper categoryForClient after connect... " << flush;
            try
            {
                test(!_session->categoryForClient().empty());
            }
            catch(const Glacier2::SessionNotExistException&)
            {
                test(false);
            }
            cout << "ok" << endl;

//             try
//             {
//                 test(_session.session() != null);
//             }
//             catch(Glacier2.SessionNotExistException ex)
//             {
//                 test(false);
//             }

            cout << "testing stringToProxy for server object... " << flush;
            Ice::ObjectPrxPtr base = 
                _session->communicator()->stringToProxy("callback:" + getTestEndpoint(_session->communicator(), 0));
            cout << "ok" << endl;

            cout << "pinging server after session creation... " << flush;
            base->ice_ping();
            cout << "ok" << endl;

            cout << "testing checked cast for server object... " << flush;
            CallbackPrxPtr twoway = ICE_CHECKED_CAST(CallbackPrx, base);
            test(twoway);
            cout << "ok" << endl;

            cout << "testing server shutdown... " << flush;
            twoway->shutdown();
            cout << "ok" << endl;

            test(_session->communicator());
            cout << "testing SessionHelper destroy... " << flush;
            _session->destroy();

            //
            // Wait for disconnected callback
            //
            _monitor.wait();

            cout << "testing SessionHelper isConnected after destroy... " << flush;
            test(_session->isConnected() == false);
            cout << "ok" << endl;

            cout << "testing SessionHelper categoryForClient after destroy... " << flush;
            try
            {
                test(!_session->categoryForClient().empty());
                test(false);
            }
            catch(const Glacier2::SessionNotExistException&)
            {
            }
            cout << "ok" << endl;

//             cout << "testing SessionHelper session after destroy... " << flush;
//             try
//             {
//                 Glacier2::SessionPrx session = _session->session();
//                 test(false);
//             }
//             catch(const Glacier2::SessionNotExistException&)
//             {
//                 cout << "ok" << endl;
//             }
//             catch(const std::exception& ex)
//             {
//                 cout << ex.what() << endl;
//             }
//             catch(const std::string& msg)
//             {
//                 cout << msg << endl;
//             }
//             catch(...)
//             {
//                 test(false);
//             }

            cout << "testing SessionHelper communicator after destroy... " << flush;
            try
            {
                test(_session->communicator());
                _session->communicator()->stringToProxy("dummy");
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
            }
            cout << "ok" << endl;


            cout << "uninstalling router with communicator... " << flush;
            communicator()->setDefaultRouter(0);
            cout << "ok" << endl;

            Ice::ObjectPrxPtr processBase;
            {
                cout << "testing stringToProxy for process object... " << flush;
                processBase = communicator()->stringToProxy("Glacier2/admin -f Process:" +
                                                            getTestEndpoint(communicator(), 11));
                cout << "ok" << endl;
            }


            Ice::ProcessPrxPtr process;
            {
                cout << "testing checked cast for admin object... " << flush;
                process = ICE_CHECKED_CAST(Ice::ProcessPrx, processBase);
                test(process != 0);
                cout << "ok" << endl;
            }

            cout << "testing Glacier2 shutdown... " << flush;
            process->shutdown();
            try
            {
                process->ice_ping();
                test(false);
            }
            catch(const Ice::LocalException&)
            {
                cout << "ok" << endl;
            }
        }

        _factory->destroy();

        _factory = ICE_MAKE_SHARED(Glacier2::SessionFactoryHelper, _initData, ICE_MAKE_SHARED(AfterShutdownSessionCallback));

        //
        // Wait a bit to ensure glaci2router has been shutdown.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));


        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect after router shutdown... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(getTestPort(_initData.properties, 10));
            _factory->setProtocol(protocol);
            _session = _factory->connect("userid", "abc123");

            //
            // Wait for connectFailed callback
            //
            _monitor.wait();

            cout << "testing SessionHelper isConnect after connect failure... " << flush;
            test(_session->isConnected() == false);
            cout << "ok" << endl;

            cout << "testing SessionHelper communicator after connect failure... " << flush;
            try
            {
                test(_session->communicator());
                _session->communicator()->stringToProxy("dummy");
                test(false);
            }
            catch(const Ice::CommunicatorDestroyedException&)
            {
            }
            cout << "ok" << endl;

            cout << "testing SessionHelper destroy after connect failure... " << flush;
            _session->destroy();
            cout << "ok" << endl;
        }

        _factory->destroy();

        if(dispatcher)
        {
            dispatcher->destroy();
            dispatcher->getThreadControl().join();
        }

        return 0;
    }

    void
    notify()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        _monitor.notify();
    }

private:

    Glacier2::SessionHelperPtr _session;
    Glacier2::SessionFactoryHelperPtr _factory;
    Ice::InitializationData _initData;
    IceUtil::Monitor<IceUtil::Mutex> _monitor;
};

} // Anonymous namespace end


int
main(int argc, char* argv[])
{
    SessionHelperClient c;
    Ice::InitializationData initData = getTestInitData(argc, argv);
    return c.main(argc, argv, initData);
}
