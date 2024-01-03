//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Thread.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

class Callback
{
public:

    Callback() :
        _response(false),
        _exception(false)
    {
    }

    void response(const Ice::ObjectPrxPtr& obj)
    {
        unique_lock lock(_mutex);
        _response = true;
        _obj = obj;
        _condVar.notify_all();
    }

    void exception()
    {
        unique_lock lock(_mutex);
        _exception = true;
        _condVar.notify_all();
    }

    void waitResponse(const char*, int)
    {
        unique_lock lock(_mutex);
        while(!_response && !_exception)
        {
            _condVar.wait(lock);
        }
    }

    bool hasResponse(Ice::ObjectPrxPtr& obj)
    {
        unique_lock lock(_mutex);
        obj = _obj;
        return _response;
    }

    bool hasException()
    {
        unique_lock lock(_mutex);
        return _exception;
    }

private:

    bool _response;
    bool _exception;
    shared_ptr<Ice::ObjectPrx> _obj;
    std::mutex _mutex;
    std::condition_variable _condVar;
};

class StressClient
{
public:

    StressClient(int id, shared_ptr<RegistryPrx> registry, bool destroySession) :
        _communicator(registry->ice_getCommunicator()),
        _id(id),
        _registry(registry),
        _notified(false),
        _terminated(false),
        _destroySession(destroySession)
    {
    }

    StressClient(int id, shared_ptr<SessionPrx> session) :
        _communicator(session->ice_getCommunicator()),
        _id(id),
        _session(session),
        _notified(false),
        _terminated(false),
        _destroySession(false)
    {
    }

    virtual ~StressClient() {}

    virtual void run()
    {
        {
            unique_lock lock(_mutex);
            while(!_notified)
            {
                _condVar.wait(lock);
            }
        }

        shared_ptr<SessionPrx> session;
        while(true)
        {
            {
                unique_lock lock(_mutex);
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
                    session->setAllocationTimeout(static_cast<Ice::Int>(IceUtilInternal::random(200))); // 200ms timeout
                }
            }

            assert(session);
            session->keepAlive();

            Ice::ObjectPrxPtr object;
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

    shared_ptr<Ice::ObjectPrx>
    allocate(shared_ptr<SessionPrx> session)
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
        return nullptr;
    }

    shared_ptr<Ice::ObjectPrx> allocateByType(shared_ptr<SessionPrx> session)
    {
        try
        {
            return session->allocateObjectByType("::StressTest");
        }
        catch(const AllocationTimeoutException&)
        {
        }
        return nullptr;
    }

    void allocateAndDestroy(const SessionPrxPtr& session)
    {
        ostringstream os;
        os << "stress-" << IceUtilInternal::random(3);
        auto cb = make_shared<Callback>();
        session->allocateObjectByIdAsync(
            Ice::stringToIdentity(os.str()),
            [cb](shared_ptr<Ice::ObjectPrx> obj)
            {
                cb->response(obj);
            },
            [cb](exception_ptr)
            {
                cb->exception();
            });
        session->destroy();
    }

    void allocateByTypeAndDestroy(shared_ptr<SessionPrx> session)
    {
        auto cb = make_shared<Callback>();
        session->allocateObjectByTypeAsync(
            "::StressTest",
            [cb](shared_ptr<Ice::ObjectPrx> obj)
            {
                cb->response(obj);
            },
            [cb](exception_ptr)
            {
                cb->exception();
            });
        session->destroy();
    }

    void notifyThread()
    {
        unique_lock lock(_mutex);
        _notified = true;
        _condVar.notify_all();
    }

    void terminate()
    {
        unique_lock lock(_mutex);
        _terminated = true;
        _condVar.notify_all();
    }

protected:

    const Ice::CommunicatorPtr _communicator;
    const int _id;
    const shared_ptr<RegistryPrx>_registry;
    const shared_ptr<SessionPrx> _session;
    bool _notified;
    bool _terminated;
    const bool _destroySession;
    std::mutex _mutex;
    std::condition_variable _condVar;
};

void
allTests(Test::TestHelper* helper)
{
    Ice::CommunicatorPtr communicator = helper->communicator();
    IceGrid::RegistryPrxPtr registry = ICE_CHECKED_CAST(
        IceGrid::RegistryPrx,
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    AdminSessionPrxPtr session = registry->createAdminSession("foo", "bar");
    session->ice_getConnection()->setACM(registry->getACMTimeout(),
                                         IceUtil::None,
                                         Ice::ICE_ENUM(ACMHeartbeat, HeartbeatAlways));

    AdminPrxPtr admin = session->getAdmin();
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

    Ice::ObjectPrxPtr obj;
    Ice::ObjectPrxPtr dummy;

    try
    {
        cout << "testing create session... " << flush;
        SessionPrxPtr session1 = registry->createSession("Client1", "");
        SessionPrxPtr session2 = registry->createSession("Client2", "");
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

        auto cb1 = make_shared<Callback>();
        session2->allocateObjectByIdAsync(
            allocatable,
            [cb1](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb1->response(prx);
            },
            [cb1](exception_ptr)
            {
                cb1->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb1->hasResponse(dummy));
        session1->releaseObject(allocatable);
        cb1->waitResponse(__FILE__, __LINE__);
        test(cb1->hasResponse(dummy));

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
        cb1 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(
            allocatable,
            [cb1](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb1->response(prx);
            },
            [cb1](exception_ptr)
            {
                cb1->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb1->hasResponse(dummy));
        session2->releaseObject(allocatable);
        cb1->waitResponse(__FILE__, __LINE__);
        test(cb1->hasResponse(dummy));

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
        auto cb3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::Test",
            [cb3](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb3->response(prx);
            },
            [cb3](exception_ptr)
            {
                cb3->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb3->hasResponse(dummy));
        session2->releaseObject(obj->ice_getIdentity());
        cb3->waitResponse(__FILE__, __LINE__);
        test(cb3->hasResponse(obj));

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
        cb1 = make_shared<Callback>();
        session2->allocateObjectByIdAsync(
            allocatable3,
            [cb1](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb1->response(prx);
            },
            [cb1](exception_ptr)
            {
                cb1->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb1->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        test(!cb1->hasResponse(dummy));
        session1->releaseObject(allocatable4);
        cb1->waitResponse(__FILE__, __LINE__);
        test(cb1->hasResponse(dummy));
        session2->releaseObject(allocatable3);

        session1->setAllocationTimeout(allocationTimeout);
        test(session2->allocateObjectByType("::TestServer1"));
        cb3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::TestServer2",
            [cb3](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb3->response(prx);
            },
            [cb3](exception_ptr)
            {
                cb3->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb3->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        cb3->waitResponse(__FILE__, __LINE__);
        test(cb3->hasResponse(dummy));
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

        shared_ptr<Ice::ObjectPrx> obj1 = session1->allocateObjectByType("::TestMultipleServer");
        test(obj1);
        shared_ptr<Ice::ObjectPrx> obj2 = session2->allocateObjectByType("::TestMultipleServer");
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
        auto cb11 = make_shared<Callback>();
        auto cb12 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(
            allocatable,
            [cb11](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb11->response(prx);
            },
            [cb11](exception_ptr)
            {
                cb11->exception();
            });
        session1->allocateObjectByIdAsync(
            allocatable,
            [cb12](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb12->response(prx);
            },
            [cb12](exception_ptr)
            {
                cb12->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb11->hasResponse(dummy));
        test(!cb12->hasResponse(dummy));
        session2->releaseObject(allocatable);
        cb11->waitResponse(__FILE__, __LINE__);
        cb12->waitResponse(__FILE__, __LINE__);
        test(cb11->hasResponse(dummy) ? cb12->hasException() : cb12->hasResponse(dummy));
        test(cb12->hasResponse(dummy) ? cb11->hasException() : cb11->hasResponse(dummy));
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable);
        auto cb31 = make_shared<Callback>();
        auto cb32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::Test",
            [cb31](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb31->response(prx);
            },
            [cb31](exception_ptr)
            {
                cb31->exception();
            });
        session1->allocateObjectByTypeAsync(
            "::Test",
            [cb32](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb32->response(prx);
            },
            [cb32](exception_ptr)
            {
                cb32->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb31->hasResponse(dummy));
        test(!cb32->hasResponse(dummy));
        session2->releaseObject(allocatable);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!cb31->hasResponse(dummy) && !cb32->hasResponse(dummy));
        test((cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy)) ||
             (cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy)));
        session1->releaseObject(allocatable);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        shared_ptr<Callback> cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
        cb33->waitResponse(__FILE__, __LINE__);
        test(cb33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable3);
        cb11 = make_shared<Callback>();
        cb12 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(
            allocatable3,
            [cb11](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb11->response(prx);
            },
            [cb11](exception_ptr)
            {
                cb11->exception();
            });
        session1->allocateObjectByIdAsync(
            allocatable3,
            [cb12](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb12->response(prx);
            },
            [cb12](exception_ptr)
            {
                cb12->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb11->hasResponse(dummy));
        test(!cb12->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        cb11->waitResponse(__FILE__, __LINE__);
        cb12->waitResponse(__FILE__, __LINE__);
        test(cb11->hasResponse(dummy) ? cb12->hasException() : cb12->hasResponse(dummy));
        test(cb12->hasResponse(dummy) ? cb11->hasException() : cb11->hasResponse(dummy));
        session1->releaseObject(allocatable3);

        session2->allocateObjectById(allocatable3);
        cb31 = make_shared<Callback>();
        cb32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::TestServer1",
            [cb31](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb31->response(prx);
            },
            [cb31](exception_ptr)
            {
                cb31->exception();
            });
        session1->allocateObjectByTypeAsync(
            "::TestServer1",
            [cb32](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb32->response(prx);
            },
            [cb32](exception_ptr)
            {
                cb32->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb31->hasResponse(dummy));
        test(!cb32->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!cb31->hasResponse(dummy) && !cb32->hasResponse(dummy));
        test((cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy)) ||
             (cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
        cb33->waitResponse(__FILE__, __LINE__);
        test(cb33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        session1->allocateObjectById(allocatable3);
        cb31 = make_shared<Callback>();
        cb32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::TestServer1",
            [cb31](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb31->response(prx);
            },
            [cb31](exception_ptr)
            {
                cb31->exception();
            });
        session1->allocateObjectByTypeAsync(
            "::TestServer1",
            [cb32](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb32->response(prx);
            },
            [cb32](exception_ptr)
            {
                cb32->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb31->hasResponse(dummy));
        test(!cb32->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        do
        {
            IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(200));
        }
        while(!cb31->hasResponse(dummy) && !cb32->hasResponse(dummy));
        test((cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy)) ||
             (cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
        cb33->waitResponse(__FILE__, __LINE__);
        test(cb33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        cout << "ok" << endl;

        cout << "testing session destroy... " << flush;

        obj = session2->allocateObjectByType("::Test"); // Allocate the object
        test(obj && obj->ice_getIdentity().name == "allocatable");

        session1->setAllocationTimeout(allocationTimeout);
        cb3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync(
            "::Test",
            [cb3](shared_ptr<Ice::ObjectPrx> prx)
            {
                cb3->response(prx);
            },
            [cb3](exception_ptr)
            {
                cb3->exception();
            });
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb3->hasResponse(dummy));
        session2->destroy();
        cb3->waitResponse(__FILE__, __LINE__);
        test(cb3->hasResponse(obj));
        session1->destroy();

        session2 = Ice::uncheckedCast<SessionPrx>(registry->createSession("Client2", ""));
        session2->setAllocationTimeout(0);
        session2->allocateObjectById(allocatable);
        session2->destroy();

        cout << "ok" << endl;
        cout << "testing application updates with allocated objects... " << flush;
        {
            session1 = registry->createSession("Client1", "");
            session2 = registry->createSession("Client2", "");

            ServerDescriptorPtr objectAllocOriginal = admin->getServerInfo("ObjectAllocation").descriptor;
            ServerDescriptorPtr objectAllocUpdate = objectAllocOriginal->ice_clone();

            ServerDescriptorPtr serverAllocOriginal = admin->getServerInfo("ServerAllocation").descriptor;
            ServerDescriptorPtr serverAllocUpdate = serverAllocOriginal->ice_clone();

            NodeUpdateDescriptor nodeUpdate;
            nodeUpdate.name = "localnode";
            nodeUpdate.servers.push_back(objectAllocUpdate);
            nodeUpdate.servers.push_back(serverAllocUpdate);

            ApplicationUpdateDescriptor appUpdate;
            appUpdate.name = "Test";
            appUpdate.nodes.push_back(nodeUpdate);

            {
                session1->allocateObjectById(allocatable3);
                auto f2 =session2->allocateObjectByIdAsync(allocatable4);

                session1->allocateObjectById(allocatable4);
                session1->releaseObject(allocatable4);
                test(f2.wait_for(0s) != future_status::ready);

                serverAllocUpdate->allocatable = false;
                admin->updateApplication(appUpdate);

                test(f2.wait_for(0s) != future_status::ready);

                session1->releaseObject(allocatable3);
                f2.wait();
                session2->releaseObject(allocatable4);

                serverAllocUpdate->allocatable = true;
                admin->updateApplication(appUpdate);
            }

            {
                session1->allocateObjectById(allocatable);
                auto f2 = session2->allocateObjectByIdAsync(allocatable);

                objectAllocUpdate->deactivationTimeout = "23";
                admin->updateApplication(appUpdate);

                session1->releaseObject(allocatable);
                f2.wait();
                session2->releaseObject(allocatable);
            }

            {
                session1->allocateObjectById(allocatable);
                auto f2 = session2->allocateObjectByIdAsync(allocatable);

                vector<ObjectDescriptor> allocatables = objectAllocUpdate->adapters[0].allocatables;
                objectAllocUpdate->adapters[0].allocatables.clear(); // Remove the allocatable object
                admin->updateApplication(appUpdate);

                try
                {
                    f2.get();
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
        shared_ptr<Ice::ObjectPrx> routerBase = communicator->stringToProxy("Glacier2/router:default -p 12347");
        shared_ptr<Glacier2::RouterPrx> router1 =
            Ice::checkedCast<Glacier2::RouterPrx>(routerBase->ice_connectionId("client1"));
        test(router1);

        shared_ptr<Glacier2::SessionPrx> sessionBase = router1->createSession("test1", "abc123");
        try
        {
            session1 = Ice::checkedCast<IceGrid::SessionPrx>(
                sessionBase->ice_connectionId("client1")->ice_router(router1));
            test(session1);
            session1->ice_ping();

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

        shared_ptr<SessionPrx> stressSession = registry->createSession("StressSession", "");

        const int nClients = 10;
        int i;
        vector<pair<shared_ptr<StressClient>, thread>> clients;
        for(i = 0; i < nClients - 2; ++i)
        {
            shared_ptr<StressClient> client;
            if(IceUtilInternal::random(2) == 1)
            {
                client = make_shared<StressClient>(i, registry, false);
            }
            else
            {
                client = make_shared<StressClient>(i, stressSession);
            }

            std::thread t(
                [client]()
                {
                    client->run();
                });
            clients.push_back(make_pair(client, std::move(t)));
        }

        {
            shared_ptr<StressClient> client = make_shared<StressClient>(i++, registry, true);
            std::thread t = std::thread(
                [client]()
                {
                    client->run();
                });
            clients.push_back(make_pair(client, std::move(t)));
            client = make_shared<StressClient>(i++, registry, true);
            t = std::thread(
                [client]()
                {
                    client->run();
                });
            clients.push_back(make_pair(client, std::move(t)));
        }

        for (const auto& p : clients)
        {
            p.first->notifyThread();
        }

        //
        // Let the stress client run for a bit.
        //
        IceUtil::ThreadControl::sleep(IceUtil::Time::seconds(8));

        //
        // Terminate the stress clients.
        //
        for (auto& p : clients)
        {
            p.first->terminate();
            p.second.join();
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
