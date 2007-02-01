// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Registry.h>
#include <IceUtil/Thread.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

void
waitForServerState(const IceGrid::AdminPrx& admin, const std::string& server, IceGrid::ServerState state)
{
    int nRetry = 0;
    while(admin->getServerState(server) != state && nRetry < 15)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        ++nRetry;
    }
    if(admin->getServerState(server) != state)
    {
        cerr << "server state change timed out:" << endl;
        cerr << "server: " << server << endl;
        cerr << "state: " << state << endl;
    }
}

class PingThread : public IceUtil::Thread, IceUtil::Monitor<IceUtil::Mutex>
{
public:
    
    PingThread(const Ice::ObjectPrx& proxy, int nRepetitions) :
        _proxy(proxy), _finished(false), _nRepetitions(nRepetitions)
    {
    }

    virtual void run()
    {
        for(int i = 0; i < _nRepetitions; ++i)
        {
            try
            {
                _proxy->ice_ping();
            }
            catch(const Ice::LocalException& ex)
            {
                _exception.reset(dynamic_cast<Ice::LocalException*>(ex.ice_clone()));
            }
            catch(...)
            {
                assert(false);
            }
        }

        Lock sync(*this);
        _finished = true;
        notifyAll();
    }

    auto_ptr<Ice::LocalException>
    waitUntilFinished()
    {
        Lock sync(*this);
        while(!_finished)
        {
            wait();
        }
        return _exception;
    }

private:
    
    Ice::ObjectPrx _proxy;
    auto_ptr<Ice::LocalException> _exception;
    bool _finished;
    int _nRepetitions;
};
typedef IceUtil::Handle<PingThread> PingThreadPtr;

class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionKeepAliveThread(const IceGrid::AdminSessionPrx& session, long timeout) :
        _session(session),
        _timeout(IceUtil::Time::seconds(timeout)),
        _destroy(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_destroy)
        {
            timedWait(_timeout);
            if(_destroy)
            {
                break;
            }
            try
            {
                _session->keepAlive();
            }
            catch(const Ice::Exception&)
            {
                break;
            }
        }
    }

    void
    destroy()
    {
        Lock sync(*this);
        _destroy = true;
        notify();
    }

private:

    IceGrid::AdminSessionPrx _session;
    const IceUtil::Time _timeout;
    bool _destroy;
};

typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(communicator->stringToProxy("IceGrid/Registry"));
    test(registry);
    IceGrid::AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(session, registry->getSessionTimeout()/2);
    keepAlive->start();
    
    IceGrid::AdminPrx admin = session->getAdmin();
    test(admin);

    admin->startServer("node-1");
    admin->startServer("node-2");

    int nRetry = 0;
    while(!admin->pingNode("node-1") && nRetry < 15)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        ++nRetry;
    }

    nRetry = 0;
    while(!admin->pingNode("node-2") && nRetry < 15)
    {
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        ++nRetry;
    }

    cout << "testing on-demand activation... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::Inactive);
        TestIntfPrx obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server"));
        waitForServerState(admin, "server", IceGrid::Active);
        obj->shutdown();
        waitForServerState(admin, "server", IceGrid::Inactive);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server", IceGrid::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing manual activation... " << flush;
    try
    {
        test(admin->getServerState("server-manual") == IceGrid::Inactive);
        TestIntfPrx obj;
        try
        {
            obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server-manual"));
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::Inactive);
        admin->startServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::Active);
        obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server-manual"));   
        test(admin->getServerState("server-manual") == IceGrid::Active);
        obj->shutdown();
        waitForServerState(admin, "server-manual", IceGrid::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing always activation... " << flush;
    try
    {
        waitForServerState(admin, "server-always", IceGrid::Active);
        TestIntfPrx obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server-always"));
        admin->stopServer("server-always");
        waitForServerState(admin, "server-always", IceGrid::Active);
        obj->shutdown();
        waitForServerState(admin, "server-always", IceGrid::Active);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        waitForServerState(admin, "server-always", IceGrid::Active);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing session activation... " << flush;
    try
    {
        IceGrid::SessionPrx session = registry->createSession("test", "");

        test(admin->getServerState("server-session") == IceGrid::Inactive);
        TestIntfPrx obj = TestIntfPrx::uncheckedCast(communicator->stringToProxy("server-session"));
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        session->allocateObjectById(obj->ice_getIdentity());
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::Active);
        obj->shutdown();
        waitForServerState(admin, "server-session", IceGrid::Inactive);
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::Active);
        nRetry = 4;
        while(--nRetry > 0)
        {
            obj->shutdown();
        }
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::Active);
        session->releaseObject(obj->ice_getIdentity());
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        waitForServerState(admin, "server-session", IceGrid::Inactive);

        session->allocateObjectById(obj->ice_getIdentity());
        obj->ice_ping();
        waitForServerState(admin, "server-session", IceGrid::Active);
        session->destroy();
        try
        {
            obj->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        waitForServerState(admin, "server-session", IceGrid::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing server disable... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::Inactive);
        admin->enableServer("server", false);
        try
        {
            communicator->stringToProxy("server")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server") == IceGrid::Inactive);

        test(admin->getServerState("server-manual") == IceGrid::Inactive);
        admin->enableServer("server-manual", false);
        try
        {
            communicator->stringToProxy("server-manual")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-manual");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::Inactive);

        test(admin->getServerState("server-always") == IceGrid::Active);
        admin->enableServer("server-always", false);
        admin->stopServer("server-always");
        test(admin->getServerState("server-always") == IceGrid::Inactive);
        try
        {
            communicator->stringToProxy("server-always")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        try
        {
            admin->startServer("server-always");
            test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-always") == IceGrid::Inactive);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;
    

    cout << "testing server enable... " << flush;
    try
    {
        test(admin->getServerState("server") == IceGrid::Inactive);
        admin->enableServer("server", true);
        communicator->stringToProxy("server")->ice_ping();
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::Inactive);
        admin->startServer("server");
        test(admin->getServerState("server") == IceGrid::Active);
        admin->stopServer("server");
        test(admin->getServerState("server") == IceGrid::Inactive);

        test(admin->getServerState("server-manual") == IceGrid::Inactive);
        admin->enableServer("server-manual", true);
        try
        {
            communicator->stringToProxy("server-manual")->ice_ping();
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }
        test(admin->getServerState("server-manual") == IceGrid::Inactive);
        admin->startServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::Active);
        admin->stopServer("server-manual");
        test(admin->getServerState("server-manual") == IceGrid::Inactive);

        test(admin->getServerState("server-always") == IceGrid::Inactive);
        admin->enableServer("server-always", true);
        waitForServerState(admin, "server-always", IceGrid::Active);
        admin->stopServer("server-always");
        try
        {
            admin->startServer("server-always");
//          test(false);
        }
        catch(const IceGrid::ServerStartException&)
        {
        }
        test(admin->getServerState("server-always") == IceGrid::Active);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;       

    cout << "testing activation failure... " << flush;
    try
    {
        int i;
        const int nThreads = 3;
        Ice::ObjectPrx invalid = communicator->stringToProxy("invalid-exe");

        vector<PingThreadPtr> threads;
        threads.reserve(nThreads);
        vector<PingThreadPtr>::const_iterator p;
        for(i = 0; i < nThreads; i++)
        {
            threads.push_back(new PingThread(invalid, 10));
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            (*p)->start();
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            auto_ptr<Ice::LocalException> ex((*p)->waitUntilFinished());
            test(dynamic_cast<Ice::NoEndpointException*>(ex.get()));
        }
        threads.resize(0);

        invalid = communicator->stringToProxy("invalid-pwd");
        for(i = 0; i < nThreads; i++)
        {
            threads.push_back(new PingThread(invalid, 10));
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            (*p)->start();
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            auto_ptr<Ice::LocalException> ex((*p)->waitUntilFinished());
            test(dynamic_cast<Ice::NoEndpointException*>(ex.get()));
        }
        threads.resize(0);

        invalid = communicator->stringToProxy("fail-on-startup");
        for(i = 0; i < nThreads; i++)
        {
            threads.push_back(new PingThread(invalid, 5));
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            (*p)->start();
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            auto_ptr<Ice::LocalException> ex((*p)->waitUntilFinished());
            test(dynamic_cast<Ice::NoEndpointException*>(ex.get()));
        }
        threads.resize(0);

    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    cout << "testing activation timeout... " << flush;
    try
    {
        test(admin->getServerState("server-activation-timeout") == IceGrid::Inactive);
        const int nThreads = 5;
        Ice::ObjectPrx proxy = communicator->stringToProxy("server-activation-timeout");
        vector<PingThreadPtr> threads;
        threads.reserve(nThreads);
        vector<PingThreadPtr>::const_iterator p;
        int i;
        for(i = 0; i < nThreads; i++)
        {
            threads.push_back(new PingThread(proxy, 1));
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            (*p)->start();
        }
        for(p = threads.begin(); p != threads.end(); ++p)
        {
            auto_ptr<Ice::LocalException> ex((*p)->waitUntilFinished());
            test(dynamic_cast<Ice::NoEndpointException*>(ex.get()));
        }
        admin->stopServer("server-activation-timeout");
    }
    catch(const IceGrid::ServerStopException& ex)
    {
        cerr << ex << ": " << ex.reason << endl;
        test(false);
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;       

    cout << "testing deactivation timeout... " << flush;
    try
    {
        test(admin->getServerState("server-deactivation-timeout") == IceGrid::Inactive);
        communicator->stringToProxy("server-deactivation-timeout")->ice_ping();
        admin->stopServer("server-deactivation-timeout");
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;       

    cout << "testing permanent disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server1") == IceGrid::Inactive);
        TestIntfPrx obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server1"));
        waitForServerState(admin, "server1", IceGrid::Active);
        obj->fail();
        waitForServerState(admin, "server1", IceGrid::Inactive);
        try
        {
            obj->ice_ping();    
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }       
        test(!admin->isServerEnabled("server1"));

        test(admin->getServerState("server1-manual") == IceGrid::Inactive);
        admin->startServer("server1-manual");
        test(admin->getServerState("server1-manual") == IceGrid::Active);
        obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server1-manual"));  
        test(admin->getServerState("server1-manual") == IceGrid::Active);
        obj->fail();
        waitForServerState(admin, "server1-manual", IceGrid::Inactive);
        test(!admin->isServerEnabled("server1-manual"));

        test(admin->getServerState("server1-always") == IceGrid::Active);
        obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server1-always"));
        obj->fail();
        waitForServerState(admin, "server1-always", IceGrid::Inactive);
        test(!admin->isServerEnabled("server1-always"));
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;       


    cout << "testing temporary disable on failure... " << flush;
    try
    {
        test(admin->getServerState("server2") == IceGrid::Inactive);
        TestIntfPrx obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server2"));
        waitForServerState(admin, "server2", IceGrid::Active);
        obj->fail();
        waitForServerState(admin, "server2", IceGrid::Inactive);
        try
        {
            obj->ice_ping();    
            test(false);
        }
        catch(const Ice::NoEndpointException&)
        {
        }       
        test(!admin->isServerEnabled("server2"));
        nRetry = 0;
        while(!admin->isServerEnabled("server2") && nRetry < 15)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
            ++nRetry;
            try
            {
                obj->ice_ping();    
            }
            catch(const Ice::NoEndpointException&)
            {
            }
        }
        test(admin->isServerEnabled("server2"));
        waitForServerState(admin, "server2", IceGrid::Active);
        obj->ice_ping();
        admin->stopServer("server2");

        test(admin->getServerState("server2-manual") == IceGrid::Inactive);
        admin->startServer("server2-manual");
        test(admin->getServerState("server2-manual") == IceGrid::Active);
        obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server2-manual"));
        obj->fail();
        waitForServerState(admin, "server2-manual", IceGrid::Inactive);
        test(!admin->isServerEnabled("server2-manual"));
        admin->startServer("server2-manual");
        test(admin->isServerEnabled("server2-manual"));
        test(admin->getServerState("server2-manual") == IceGrid::Active);
        admin->stopServer("server2-manual");

        test(admin->getServerState("server2-always") == IceGrid::Active);
        obj = TestIntfPrx::checkedCast(communicator->stringToProxy("server2-always"));
        obj->fail();
        waitForServerState(admin, "server2-always", IceGrid::Inactive);
        test(!admin->isServerEnabled("server2-always"));
        nRetry = 0;
        while((!admin->isServerEnabled("server2-always") ||
               admin->getServerState("server2-always") != IceGrid::Active) &&
              nRetry < 15)
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
            ++nRetry;
        }
        test(admin->isServerEnabled("server2-always") && admin->getServerState("server2-always") == IceGrid::Active);
        obj->ice_ping();
    }
    catch(const Ice::LocalException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("node-1");
    admin->stopServer("node-2");
    
    keepAlive->destroy();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    session->destroy();
}

