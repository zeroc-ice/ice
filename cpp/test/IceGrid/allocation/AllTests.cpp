// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

class Callback : public IceUtil::Monitor<IceUtil::Mutex>, public virtual IceUtil::Shared
{
public:

    Callback() : _response(false), _exception(false)
    {
    }

    void
    response(const Ice::ObjectPrx& obj)
    {
        Lock sync(*this);
        _response = true;
        _obj = obj;
        notify();
    }

    void
    exception(const Ice::Exception&)
    {
        Lock sync(*this);
        _exception = true;
        notify();
    }

    void
    waitResponse(const char*, int)
    {
        Lock sync(*this);
        while(!_response && !_exception)
        {
            wait();
        }
    }

    bool
    hasResponse(Ice::ObjectPrx& obj)
    {
        Lock sync(*this);
        obj = _obj;
        return _response;
    }

    bool
    hasException()
    {
        Lock sync(*this);
        return _exception;
    }

private:

    bool _response;
    bool _exception;
    Ice::ObjectPrx _obj;
};
typedef IceUtil::Handle<Callback> CallbackPtr;

class StressClient : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    StressClient(int id, const RegistryPrx& registry, bool destroySession) :
        _communicator(registry->ice_getCommunicator()),
        _id(id),
        _registry(registry),
        _notified(false),
        _terminated(false),
        _destroySession(destroySession)
    {
    }

    StressClient(int id, const SessionPrx& session) :
        _communicator(session->ice_getCommunicator()),
        _id(id),
        _session(session),
        _notified(false),
        _terminated(false),
        _destroySession(false)
    {
    }

    virtual
    void run()
    {
        {
            Lock sync(*this);
            while(!_notified)
            {
                wait();
            }
        }

        SessionPrx session;
        while(true)
        {
            {
                Lock sync(*this);
                if(_terminated)
                {
                    if(!_session && session)
                    {
                        session->destroy();
                    }
                    return;
                }
            }

            if(!session)
            {
                ostringstream os;
                os << "Client-" << _id;
                if(_session)
                {
                    session = _session;
                }
                else
                {
                    session = _registry->createSession(os.str(), "");
                    session->setAllocationTimeout(IceUtilInternal::random(200)); // 200ms timeout
                }
            }

            assert(session);
            session->keepAlive();

            Ice::ObjectPrx object;
            switch(IceUtilInternal::random(_destroySession ? 4 : 2))
            {
            case 0:
                object = allocate(session);
                break;
            case 1:
                object = allocateByType(session);
                break;
            case 2:
                assert(!_session);
                allocateAndDestroy(session);
                session = 0;
                break;
            case 3:
                assert(!_session);
                allocateByTypeAndDestroy(session);
                session = 0;
                break;
            }

            if(object)
            {
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(IceUtilInternal::random(20)));
                switch(IceUtilInternal::random(_destroySession ? 2 : 1))
                {
                case 0:
                    session->releaseObject(object->ice_getIdentity());
                    break;
                case 1:
                    assert(!_session);
                    session->destroy();
                    session = 0;
                    break;
                }
            }
        }
    }

    Ice::ObjectPrx
    allocate(const SessionPrx& session)
    {
        ostringstream os;
        os << "stress-" << IceUtilInternal::random(6) + 1;
        try
        {
            return session->allocateObjectById(Ice::stringToIdentity(os.str()));
        }
        catch(const AllocationTimeoutException&)
        {
        }
        catch(const AllocationException&)
        {
            // This can only happen if we are using the common session
            // and the object is already allocated.
            test(_session);
        }
        return 0;
    }

    Ice::ObjectPrx
    allocateByType(const SessionPrx& session)
    {
        try
        {
            return session->allocateObjectByType("::StressTest");
        }
        catch(const AllocationTimeoutException&)
        {
        }
        return 0;
    }

    void
    allocateAndDestroy(const SessionPrx& session)
    {
        ostringstream os;
        os << "stress-" << IceUtilInternal::random(3);
        CallbackPtr asyncCB = new Callback();
        IceGrid::Callback_Session_allocateObjectByIdPtr cb =
            IceGrid::newCallback_Session_allocateObjectById(asyncCB, &Callback::response, &Callback::exception);
        session->begin_allocateObjectById(Ice::stringToIdentity(os.str()), cb);
        session->destroy();
    }

    void
    allocateByTypeAndDestroy(const SessionPrx& session)
    {
        CallbackPtr asyncCB = new Callback();
        IceGrid::Callback_Session_allocateObjectByTypePtr cb =
            IceGrid::newCallback_Session_allocateObjectByType(asyncCB, &Callback::response, &Callback::exception);
        session->begin_allocateObjectByType("::StressTest", cb);
        session->destroy();
    }

    void
    notifyThread()
    {
        Lock sync(*this);
        _notified = true;
        notify();
    }

    void
    terminate()
    {
        Lock sync(*this);
        _terminated = true;
        notify();
    }

protected:

    const Ice::CommunicatorPtr _communicator;
    const int _id;
    const RegistryPrx _registry;
    const SessionPrx _session;
    bool _notified;
    bool _terminated;
    const bool _destroySession;
};
typedef IceUtil::Handle<StressClient> StressClientPtr;

void
allTests(const Ice::CommunicatorPtr& communicator)
{
    IceGrid::RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");
    session->ice_getConnection()->setACM(registry->getACMTimeout(), IceUtil::None, Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    AdminPrx admin = session->getAdmin();
    test(admin);

    cout << "starting router... " << flush;
    try
    {
        admin->startServer("Glacier2");
    }
    catch(const ServerStartException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    cout << "ok" << endl;

    const int allocationTimeout = 5000;

    Ice::ObjectPrx obj;
    Ice::ObjectPrx dummy;

    try
    {
        cout << "testing create session... " << flush;
        SessionPrx session1 = registry->createSession("Client1", "");
        SessionPrx session2 = registry->createSession("Client2", "");
        cout << "ok" << endl;

        cout << "testing allocate object by identity... " << flush;

        Ice::Identity allocatable = Ice::stringToIdentity("allocatable");
        Ice::Identity allocatablebis = Ice::stringToIdentity("allocatablebis");

        try
        {
            session1->allocateObjectById(Ice::stringToIdentity("dummy"));
        }
        catch(const ObjectNotRegisteredException&)
        {
        }
        try
        {
            session1->releaseObject(Ice::stringToIdentity("dummy"));
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        try
        {
            session1->allocateObjectById(Ice::stringToIdentity("nonallocatable"));
            test(false);
        }
        catch(const AllocationException&)
        {
            test(false);
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        try
        {
            session2->allocateObjectById(Ice::stringToIdentity("nonallocatable"));
            test(false);
        }
        catch(const AllocationException&)
        {
            test(false);
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        try
        {
            session1->releaseObject(Ice::stringToIdentity("nonallocatable"));
            test(false);
        }
        catch(const AllocationException&)
        {
            test(false);
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        try
        {
            session2->releaseObject(Ice::stringToIdentity("nonallocatable"));
            test(false);
        }
        catch(const AllocationException&)
        {
            test(false);
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        session1->allocateObjectById(allocatable);
        try
        {
            session1->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationException&)
        {
        }

        session1->setAllocationTimeout(0);
        session2->setAllocationTimeout(0);

        try
        {
            session2->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session2->releaseObject(allocatable);
            test(false);
        }
        catch(const AllocationException&)
        {
        }

        session1->allocateObjectById(allocatablebis);
        try
        {
            session2->allocateObjectById(allocatablebis);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session1->releaseObject(allocatablebis);
        session2->allocateObjectById(allocatablebis);
        try
        {
            session1->allocateObjectById(allocatablebis);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session2->releaseObject(allocatablebis);

        session2->setAllocationTimeout(allocationTimeout);

        CallbackPtr asyncCB1 = new Callback();
        IceGrid::Callback_Session_allocateObjectByIdPtr cb1 = IceGrid::newCallback_Session_allocateObjectById(asyncCB1,
                                                 &Callback::response,
                                                 &Callback::exception);

        session2->begin_allocateObjectById(allocatable, cb1);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB1->hasResponse(dummy));
        session1->releaseObject(allocatable);
        asyncCB1->waitResponse(__FILE__, __LINE__);
        test(asyncCB1->hasResponse(dummy));

        session1->setAllocationTimeout(0);
        try
        {
            session1->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session1->releaseObject(allocatable);
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        session1->setAllocationTimeout(allocationTimeout);
        asyncCB1 = new Callback();
        cb1 = IceGrid::newCallback_Session_allocateObjectById(asyncCB1,
                                                 &Callback::response,
                                                 &Callback::exception);
        session1->begin_allocateObjectById(allocatable, cb1);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB1->hasResponse(dummy));
        session2->releaseObject(allocatable);
        asyncCB1->waitResponse(__FILE__, __LINE__);
        test(asyncCB1->hasResponse(dummy));

        session1->releaseObject(allocatable);

        cout << "ok" << endl;

        cout << "testing allocate object by type... " << flush;

        session1->setAllocationTimeout(0);
        session2->setAllocationTimeout(0);

        try
        {
            obj = session1->allocateObjectByType("::Unknown");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
            test(false);
        }
        catch(const AllocationException&)
        {
        }

        try
        {
            obj = session1->allocateObjectByType("::NotAllocatable");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
            test(false);
        }
        catch(const AllocationException&)
        {
        }

        obj = session1->allocateObjectByType("::Test");
        test(obj && obj->ice_getIdentity().name == "allocatable");
        try
        {
            session1->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        try
        {
            session2->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session2->releaseObject(obj->ice_getIdentity());
        }
        catch(const AllocationException&)
        {
        }

        session1->releaseObject(obj->ice_getIdentity());
        try
        {
            session1->releaseObject(obj->ice_getIdentity());
        }
        catch(const AllocationException&)
        {
        }

        obj = session2->allocateObjectByType("::Test"); // Allocate the object
        test(obj && obj->ice_getIdentity().name == "allocatable");
        try
        {
            session2->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        try
        {
            session1->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session1->allocateObjectByType("::TestBis");
        try
        {
            session2->allocateObjectByType("::TestBis");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session1->releaseObject(allocatablebis);
        session2->allocateObjectByType("::TestBis");
        try
        {
            session1->allocateObjectByType("::TestBis");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session2->releaseObject(allocatablebis);

        session1->setAllocationTimeout(allocationTimeout);
        CallbackPtr asyncCB3 = new Callback();
        IceGrid::Callback_Session_allocateObjectByTypePtr cb3 =
            IceGrid::newCallback_Session_allocateObjectByType(asyncCB3, &Callback::response, &Callback::exception);

        session1->begin_allocateObjectByType("::Test", cb3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB3->hasResponse(dummy));
        session2->releaseObject(obj->ice_getIdentity());
        asyncCB3->waitResponse(__FILE__, __LINE__);
        test(asyncCB3->hasResponse(obj));

        session1->releaseObject(obj->ice_getIdentity());

        admin->enableServer("ObjectAllocation", false);
        try
        {
            while(true)
            {
                // The notification of the server being disabled is asynchronous and might
                // not be visible to the allocation system immediately.
                session1->allocateObjectByType("::Test");
                session1->releaseObject(obj->ice_getIdentity());
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
            }
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        admin->enableServer("ObjectAllocation", true);

        cout << "ok" << endl;

        cout << "testing object allocation timeout... " << flush;
        session1->allocateObjectById(allocatable);

        IceUtil::Time time = IceUtil::Time::now();
        session2->setAllocationTimeout(500);
        try
        {
            session2->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
            test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
        }
        time = IceUtil::Time::now();
        try
        {
            session2->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
        time = IceUtil::Time::now();
        try
        {
            session2->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());

        session1->releaseObject(allocatable);
        session2->setAllocationTimeout(0);

        cout << "ok" << endl;

        cout << "testing server allocation... " << flush;

        session1->setAllocationTimeout(0);
        session2->setAllocationTimeout(0);

        Ice::Identity allocatable3 = Ice::stringToIdentity("allocatable3");
        Ice::Identity allocatable4 = Ice::stringToIdentity("allocatable4");

        session1->allocateObjectById(allocatable3);
        try
        {
            session2->allocateObjectById(allocatable3);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session2->allocateObjectById(allocatable4);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }

        session1->allocateObjectById(allocatable4);
        session1->releaseObject(allocatable3);
        try
        {
            session2->allocateObjectById(allocatable3);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session1->releaseObject(allocatable4);
        session2->allocateObjectById(allocatable3);
        try
        {
            session1->allocateObjectById(allocatable3);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session1->allocateObjectById(allocatable4);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session2->allocateObjectById(allocatable4);
        session2->releaseObject(allocatable3);
        try
        {
            session1->allocateObjectById(allocatable3);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session1->allocateObjectByType("::TestServer1");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        try
        {
            session1->allocateObjectByType("::TestServer2");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        test(session2->allocateObjectByType("::TestServer1"));
        try
        {
            session2->allocateObjectByType("::TestServer1");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        try
        {
            session2->allocateObjectByType("::TestServer2");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        session2->releaseObject(allocatable3);
        session2->releaseObject(allocatable4);

        session1->allocateObjectById(allocatable3);
        session1->allocateObjectById(allocatable4);

        session2->setAllocationTimeout(allocationTimeout);
        asyncCB1 = new Callback();
        cb1 = IceGrid::newCallback_Session_allocateObjectById(asyncCB1, &Callback::response, &Callback::exception);
        session2->begin_allocateObjectById(allocatable3, cb1);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB1->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        test(!asyncCB1->hasResponse(dummy));
        session1->releaseObject(allocatable4);
        asyncCB1->waitResponse(__FILE__, __LINE__);
        test(asyncCB1->hasResponse(dummy));
        session2->releaseObject(allocatable3);

        session1->setAllocationTimeout(allocationTimeout);
        test(session2->allocateObjectByType("::TestServer1"));
        asyncCB3 = new Callback();
        cb3 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB3, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectByType("::TestServer2", cb3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB3->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        asyncCB3->waitResponse(__FILE__, __LINE__);
        test(asyncCB3->hasResponse(dummy));
        session1->releaseObject(allocatable4);

        session1->setAllocationTimeout(0);
        session2->setAllocationTimeout(0);
        test(session1->allocateObjectByType("::TestMultipleByServer"));
        try
        {
            session2->allocateObjectByType("::TestMultipleByServer");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        test(session1->allocateObjectByType("::TestMultipleByServer"));
        session1->releaseObject(Ice::stringToIdentity("allocatable31"));
        session1->releaseObject(Ice::stringToIdentity("allocatable41"));
        test(session2->allocateObjectByType("::TestMultipleByServer"));
        try
        {
            session1->allocateObjectByType("::TestMultipleByServer");
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        test(session2->allocateObjectByType("::TestMultipleByServer"));
        session2->releaseObject(Ice::stringToIdentity("allocatable31"));
        session2->releaseObject(Ice::stringToIdentity("allocatable41"));

        Ice::ObjectPrx obj1 = session1->allocateObjectByType("::TestMultipleServer");
        test(obj1);
        Ice::ObjectPrx obj2 = session2->allocateObjectByType("::TestMultipleServer");
        test(obj2);
        try
        {
            session1->allocateObjectByType("::TestMultipleServer");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        try
        {
            session2->allocateObjectByType("::TestMultipleServer");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        session1->releaseObject(obj1->ice_getIdentity());
        obj1 = session2->allocateObjectByType("::TestMultipleServer");
        session2->releaseObject(obj1->ice_getIdentity());
        session2->releaseObject(obj2->ice_getIdentity());

        admin->enableServer("ServerAllocation", false);
        try
        {
            while(true)
            {
                // The notification of the server being disabled is asynchronous and might
                // not be visible to the allocation system immediately.
                session1->allocateObjectByType("::TestServer1");
                session1->releaseObject(allocatable3);
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(100));
            }
            test(false);
        }
        catch(const AllocationException&)
        {
        }
        admin->enableServer("ServerAllocation", true);

        cout << "ok" << endl;

        cout << "testing concurrent allocations... " << flush;

        session1->setAllocationTimeout(allocationTimeout);
        session2->setAllocationTimeout(allocationTimeout);

        session2->allocateObjectById(allocatable);
        CallbackPtr asyncCB11 = new Callback();
        IceGrid::Callback_Session_allocateObjectByIdPtr cb11 =
            IceGrid::newCallback_Session_allocateObjectById(asyncCB11, &Callback::response, &Callback::exception);
        CallbackPtr asyncCB12 = new Callback();
        IceGrid::Callback_Session_allocateObjectByIdPtr cb12 =
            IceGrid::newCallback_Session_allocateObjectById(asyncCB12, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectById(allocatable, cb11);
        session1->begin_allocateObjectById(allocatable, cb12);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB11->hasResponse(dummy));
        test(!asyncCB12->hasResponse(dummy));
        session2->releaseObject(allocatable);
        asyncCB11->waitResponse(__FILE__, __LINE__);
        asyncCB12->waitResponse(__FILE__, __LINE__);
        test(asyncCB11->hasResponse(dummy) ? asyncCB12->hasException() : asyncCB12->hasResponse(dummy));
        test(asyncCB12->hasResponse(dummy) ? asyncCB11->hasException() : asyncCB11->hasResponse(dummy));
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable);
        CallbackPtr asyncCB31 = new Callback();
        IceGrid::Callback_Session_allocateObjectByTypePtr cb31 =
            IceGrid::newCallback_Session_allocateObjectByType(asyncCB31, &Callback::response, &Callback::exception);
        CallbackPtr asyncCB32 = new Callback();
        IceGrid::Callback_Session_allocateObjectByTypePtr cb32 =
            IceGrid::newCallback_Session_allocateObjectByType(asyncCB32, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectByType("::Test", cb31);
        session1->begin_allocateObjectByType("::Test", cb32);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session2->releaseObject(allocatable);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        CallbackPtr asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable3);
        asyncCB11 = new Callback();
        cb11 = IceGrid::newCallback_Session_allocateObjectById(asyncCB11, &Callback::response, &Callback::exception);
        asyncCB12 = new Callback();
        cb12 = IceGrid::newCallback_Session_allocateObjectById(asyncCB12, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectById(allocatable3, cb11);
        session1->begin_allocateObjectById(allocatable3, cb12);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB11->hasResponse(dummy));
        test(!asyncCB12->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        asyncCB11->waitResponse(__FILE__, __LINE__);
        asyncCB12->waitResponse(__FILE__, __LINE__);
        test(asyncCB11->hasResponse(dummy) ? asyncCB12->hasException() : asyncCB12->hasResponse(dummy));
        test(asyncCB12->hasResponse(dummy) ? asyncCB11->hasException() : asyncCB11->hasResponse(dummy));
        session1->releaseObject(allocatable3);

        session2->allocateObjectById(allocatable3);
        asyncCB31 = new Callback();
        cb31 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB31, &Callback::response, &Callback::exception);
        asyncCB32 = new Callback();
        cb32 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB32, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectByType("::TestServer1", cb31);
        session1->begin_allocateObjectByType("::TestServer1", cb32);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        session1->allocateObjectById(allocatable3);
        asyncCB31 = new Callback();
        cb31 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB31, &Callback::response, &Callback::exception);
        asyncCB32 = new Callback();
        cb32 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB32, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectByType("::TestServer1", cb31);
        session1->begin_allocateObjectByType("::TestServer1", cb32);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        cout << "ok" << endl;

        cout << "testing session destroy... " << flush;

        obj = session2->allocateObjectByType("::Test"); // Allocate the object
        test(obj && obj->ice_getIdentity().name == "allocatable");

        session1->setAllocationTimeout(allocationTimeout);
        asyncCB3 = new Callback();
        cb3 = IceGrid::newCallback_Session_allocateObjectByType(asyncCB3, &Callback::response, &Callback::exception);
        session1->begin_allocateObjectByType("::Test", cb3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!asyncCB3->hasResponse(dummy));
        session2->destroy();
        asyncCB3->waitResponse(__FILE__, __LINE__);
        test(asyncCB3->hasResponse(obj));
        session1->destroy();

        session2 = SessionPrx::uncheckedCast(registry->createSession("Client2", ""));
        session2->setAllocationTimeout(0);
        session2->allocateObjectById(allocatable);
        session2->destroy();

        cout << "ok" << endl;
        cout << "testing application updates with allocated objects... " << flush;
        {
            SessionPrx session1 = registry->createSession("Client1", "");
            SessionPrx session2 = registry->createSession("Client2", "");

            ServerDescriptorPtr objectAllocOriginal = admin->getServerInfo("ObjectAllocation").descriptor;
            ServerDescriptorPtr objectAllocUpdate = ServerDescriptorPtr::dynamicCast(objectAllocOriginal->ice_clone());

            ServerDescriptorPtr serverAllocOriginal = admin->getServerInfo("ServerAllocation").descriptor;
            ServerDescriptorPtr serverAllocUpdate = ServerDescriptorPtr::dynamicCast(serverAllocOriginal->ice_clone());

            NodeUpdateDescriptor nodeUpdate;
            nodeUpdate.name = "localnode";
            nodeUpdate.servers.push_back(objectAllocUpdate);
            nodeUpdate.servers.push_back(serverAllocUpdate);

            ApplicationUpdateDescriptor appUpdate;
            appUpdate.name = "Test";
            appUpdate.nodes.push_back(nodeUpdate);

            {
                session1->allocateObjectById(allocatable3);
                Ice::AsyncResultPtr r2 = session2->begin_allocateObjectById(allocatable4);

                session1->allocateObjectById(allocatable4);
                session1->releaseObject(allocatable4);
                test(!r2->isCompleted());

                serverAllocUpdate->allocatable = false;
                admin->updateApplication(appUpdate);

                test(!r2->isCompleted());

                session1->releaseObject(allocatable3);
                session2->end_allocateObjectById(r2);
                session2->releaseObject(allocatable4);

                serverAllocUpdate->allocatable = true;
                admin->updateApplication(appUpdate);
            }

            {
                session1->allocateObjectById(allocatable);
                Ice::AsyncResultPtr r2 = session2->begin_allocateObjectById(allocatable);

                objectAllocUpdate->deactivationTimeout = "23";
                admin->updateApplication(appUpdate);

                session1->releaseObject(allocatable);
                session2->end_allocateObjectById(r2);
                session2->releaseObject(allocatable);
            }

            {
                session1->allocateObjectById(allocatable);
                Ice::AsyncResultPtr r2 = session2->begin_allocateObjectById(allocatable);

                vector<ObjectDescriptor> allocatables = objectAllocUpdate->adapters[0].allocatables;
                objectAllocUpdate->adapters[0].allocatables.clear(); // Remove the allocatable object
                admin->updateApplication(appUpdate);

                try
                {
                    session2->end_allocateObjectById(r2);
                    test(false);
                }
                catch(const ObjectNotRegisteredException&)
                {
                }
                try
                {
                    session1->releaseObject(allocatable);
                    test(false);
                }
                catch(const ObjectNotRegisteredException&)
                {
                }

                objectAllocUpdate->adapters[0].allocatables = allocatables;
                admin->updateApplication(appUpdate);
            }

            session1->destroy();
            session2->destroy();
        }
        cout << "ok" << endl;

        cout << "testing allocation with Glacier2 session... " << flush;
        Ice::ObjectPrx routerBase = communicator->stringToProxy("Glacier2/router:default -p 12347");
        Glacier2::RouterPrx router1 = Glacier2::RouterPrx::checkedCast(routerBase->ice_connectionId("client1"));
        test(router1);

        Glacier2::SessionPrx sessionBase = router1->createSession("test1", "abc123");
        try
        {
            session1 = IceGrid::SessionPrx::checkedCast(sessionBase->ice_connectionId("client1")->ice_router(router1));
            test(session1);
            session1->ice_ping();

            Ice::ObjectPrx obj;
            obj = session1->allocateObjectById(allocatable)->ice_connectionId("client1")->ice_router(router1);
            obj->ice_ping();
            session1->releaseObject(allocatable);
            try
            {
                obj->ice_ping();
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }

            obj = session1->allocateObjectById(allocatable3)->ice_connectionId("client1")->ice_router(router1);
            obj->ice_ping();
            obj2 = communicator->stringToProxy("allocatable4")->ice_connectionId("client1")->ice_router(router1);
            obj2->ice_ping();
            session1->releaseObject(allocatable3);
            try
            {
                obj->ice_ping();
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }
            try
            {
                obj2->ice_ping();
            }
            catch(const Ice::ObjectNotExistException&)
            {
            }
            session1->destroy();
        }
        catch(const Ice::LocalException& ex)
        {
            cerr << ex << endl;
            test(false);
        }
        cout << "ok" << endl;

        cout << "stress test... " << flush;

        SessionPrx stressSession = registry->createSession("StressSession", "");

        const int nClients = 10;
        int i;
        vector<StressClientPtr> clients;
        for(i = 0; i < nClients - 2; ++i)
        {
            if(IceUtilInternal::random(2) == 1)
            {
                clients.push_back(new StressClient(i, registry, false));
            }
            else
            {
                clients.push_back(new StressClient(i, stressSession));
            }
            clients.back()->start();
        }
        clients.push_back(new StressClient(i++, registry, true));
        clients.back()->start();
        clients.push_back(new StressClient(i++, registry, true));
        clients.back()->start();

        for(vector<StressClientPtr>::const_iterator p = clients.begin(); p != clients.end(); ++p)
        {
            (*p)->notifyThread();
        }

        //
        // Let the stress client run for a bit.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(8));

        //
        // Terminate the stress clients.
        //
        for(vector<StressClientPtr>::const_iterator q = clients.begin(); q != clients.end(); ++q)
        {
            (*q)->terminate();
            (*q)->getThreadControl().join();
        }

        stressSession->destroy();

        cout << "ok" << endl;

    }
    catch(const AllocationTimeoutException& ex)
    {
        cerr << ex << endl;
        test(false);
    }
    catch(const AllocationException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }
    catch(const DeploymentException& ex)
    {
        cerr << ex.reason << endl;
        test(false);
    }

    cout << "shutting down router... " << flush;
    admin->stopServer("Glacier2");
    cout << "ok" << endl;

    session->destroy();
}
