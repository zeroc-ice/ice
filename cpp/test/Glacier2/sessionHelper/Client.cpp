// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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

class Dispatcher : public Ice::Dispatcher, public IceUtil::Thread
{

public:

    Dispatcher():
        _destroyed(false)
    {
    }

    virtual void
    dispatch(const Ice::DispatcherCallPtr& call,
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
            Ice::DispatcherCallPtr item = nextItem();
            if(!item)
            {
                break;
            }
            item->run();
        }
    }

private:

    Ice::DispatcherCallPtr
    nextItem()
    {
        IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
        while(_queue.empty())
        {
            if(_destroyed)
            {
                return 0;
            }
            _monitor.wait();
        }
        Ice::DispatcherCallPtr item = _queue.front();
        _queue.pop_front();
        return item;
    }


    IceUtil::Monitor<IceUtil::Mutex> _monitor;
    list<Ice::DispatcherCallPtr> _queue;
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
        string protocol = communicator()->getProperties()->getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        string host = communicator()->getProperties()->getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");
        _initData.properties = Ice::createProperties(argc, argv, communicator()->getProperties());
        _initData.properties->setProperty("Ice.Default.Router", "Glacier2/router:default -p 12347");

        DispatcherPtr dispatcher = new Dispatcher();
        dispatcher->start();
        _initData.dispatcher = dispatcher;

        _factory = new Glacier2::SessionFactoryHelper(_initData, new FailSessionCallback());

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
        _factory = new Glacier2::SessionFactoryHelper(_initData, new InterruptConnectCallback());

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect interrupt... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(12011);
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

        _factory = new Glacier2::SessionFactoryHelper(_initData, new SuccessSessionCallback());

        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(12347);
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
            Ice::ObjectPrx base = _session->communicator()->stringToProxy("callback:default -p 12010");
            cout << "ok" << endl;

            cout << "pinging server after session creation... " << flush;
            base->ice_ping();
            cout << "ok" << endl;

            cout << "testing checked cast for server object... " << flush;
            CallbackPrx twoway = CallbackPrx::checkedCast(base);
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

            Ice::ObjectPrx processBase;
            {
                cout << "testing stringToProxy for process object... " << flush;
                processBase = communicator()->stringToProxy("Glacier2/admin -f Process:default -h \"" + host + "\" -p 12348");
                cout << "ok" << endl;
            }


            Ice::ProcessPrx process;
            {
                cout << "testing checked cast for admin object... " << flush;
                process = Ice::ProcessPrx::checkedCast(processBase);
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

        _factory = new Glacier2::SessionFactoryHelper(_initData, new AfterShutdownSessionCallback());

        //
        // Wait a bit to ensure glaci2router has been shutdown.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));


        {
            IceUtil::Monitor<IceUtil::Mutex>::Lock lock(_monitor);
            cout << "testing SessionHelper connect after router shutdown... " << flush;
            _factory->setRouterHost(host);
            _factory->setPort(12347);
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
#ifdef ICE_STATIC_LIBS
    Ice::registerIceSSL();
#endif

    SessionHelperClient c;
    return c.main(argc, argv);
}
