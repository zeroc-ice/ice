// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceUtil/IceUtil.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Registry.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;

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
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    cout << "ok" << endl;

    cout << "shutting down server... " << flush;
    obj->shutdown();
    cout << "ok" << endl;
}

void
allTestsWithDeploy(const Ice::CommunicatorPtr& communicator)
{
    cout << "testing stringToProxy... " << flush;
    Ice::ObjectPrx base = communicator->stringToProxy("test @ TestAdapter");
    test(base);
    Ice::ObjectPrx base2 = communicator->stringToProxy("test");
    test(base2);
    cout << "ok" << endl;

    cout << "testing checked cast... " << flush;
    TestIntfPrx obj = TestIntfPrx::checkedCast(base);
    test(obj);
    test(obj == base);
    TestIntfPrx obj2 = TestIntfPrx::checkedCast(base2);
    test(obj2);
    test(obj2 == base2);
    cout << "ok" << endl;

    cout << "pinging server... " << flush;
    obj->ice_ping();
    obj2->ice_ping();
    cout << "ok" << endl;

    cout << "testing reference with unknown identity... " << flush;
    try
    {
        communicator->stringToProxy("unknown/unknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject == "object");
        test(ex.id == "unknown/unknown");
    }
    cout << "ok" << endl;

    cout << "testing reference with unknown adapter... " << flush;
    try
    {
        communicator->stringToProxy("test @ TestAdapterUnknown")->ice_ping();
        test(false);
    }
    catch (const Ice::NotRegisteredException& ex)
    {
        test(ex.kindOfObject == "object adapter");
        test(ex.id == "TestAdapterUnknown");
    }
    cout << "ok" << endl;

    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy("IceGrid/Registry"));
    test(registry);
    IceGrid::AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(session, registry->getSessionTimeout()/2);
    keepAlive->start();

    IceGrid::AdminPrx admin = session->getAdmin();
    test(admin);

    admin->enableServer("server", false);
    admin->stopServer("server");

    cout << "testing whether server is still reachable... " << flush;
    try
    {
        obj = TestIntfPrx::checkedCast(base);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
        test(false);
    }
    catch(const Ice::NoEndpointException&)
    {
    }
    
    admin->enableServer("server", true);

    try
    {
        obj = TestIntfPrx::checkedCast(base);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    try
    {
        obj2 = TestIntfPrx::checkedCast(base2);
    }
    catch(const Ice::NoEndpointException&)
    {
        test(false);
    }
    cout << "ok" << endl;

    admin->stopServer("server");

    keepAlive->destroy();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    session->destroy();
}
