//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/Ice.h>
#include <IceGrid/IceGrid.h>
#include <Glacier2/Router.h>
#include <TestHelper.h>
#include <Test.h>

#include <random>

using namespace std;
using namespace Test;
using namespace IceGrid;

class Callback
{
public:

    Callback() : _response(false), _exception(false)
    {
    }

    void
    response(shared_ptr<Ice::ObjectPrx> obj)
    {
        {
            lock_guard<mutex> lg(_mutex);
            _response = true;
            _obj = move(obj);
        }

        _condVar.notify_one();
    }

    void
    exception(exception_ptr)
    {
        {
            lock_guard<mutex> lg(_mutex);
            _exception = true;
        }
        _condVar.notify_one();
    }

    void
    waitResponse(const char*, int)
    {
        unique_lock<mutex> lock(_mutex);
        _condVar.wait(lock, [&] { return _response || _exception; });
    }

    bool
    hasResponse(shared_ptr<Ice::ObjectPrx>& obj)
    {
        lock_guard<mutex> lg(_mutex);
        obj = _obj;
        return _response;
    }

    bool
    hasException()
    {
        lock_guard<mutex> lg(_mutex);
        return _exception;
    }

private:

    bool _response;
    bool _exception;
    mutex _mutex;
    condition_variable _condVar;
    shared_ptr<Ice::ObjectPrx> _obj;
};

class StressClient
{
public:

    StressClient(int id, shared_ptr<RegistryPrx> registry, bool destroySession) :
        _id(id),
        _registry(move(registry)),
        _notified(false),
        _terminated(false),
        _destroySession(destroySession)
    {
    }

    StressClient(int id, shared_ptr<SessionPrx> session) :
        _id(id),
        _session(move(session)),
        _notified(false),
        _terminated(false),
        _destroySession(false)
    {
    }

    void run()
    {
        {
            unique_lock<mutex> lock(_mutex);
            _condVar.wait(lock, [&] { return _notified == true; });
        }

        shared_ptr<SessionPrx> session;
        while(true)
        {
            {
                lock_guard<mutex> lg(_mutex);
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
                    session->setAllocationTimeout(static_cast<int>(_rd() % 200)); // 200ms timeout
                }
            }

            assert(session);
            session->keepAlive();

            shared_ptr<Ice::ObjectPrx> object;
            switch( _rd() % (_destroySession ? 4 : 2))
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
                this_thread::sleep_for(20ms);
                switch(_rd() % (_destroySession ? 2 : 1))
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
    allocate(const shared_ptr<SessionPrx>& session)
    {
        ostringstream os;
        os << "stress-" << (_rd() % 6) + 1;
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

    shared_ptr<Ice::ObjectPrx>
    allocateByType(const shared_ptr<SessionPrx>& session)
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
    allocateAndDestroy(const shared_ptr<SessionPrx>& session)
    {
        ostringstream os;
        os << "stress-" << (_rd() % 3);

        auto asyncCB = make_shared<Callback>();
        session->allocateObjectByIdAsync(Ice::stringToIdentity(os.str()),
                                         [asyncCB](shared_ptr<Ice::ObjectPrx> o) { asyncCB->response(move(o)); },
                                         [asyncCB](exception_ptr e) { asyncCB->exception(e); });
        session->destroy();
    }

    void
    allocateByTypeAndDestroy(const shared_ptr<SessionPrx>& session)
    {
        auto asyncCB = make_shared<Callback>();
        session->allocateObjectByTypeAsync("::StressTest",
                                          [asyncCB](shared_ptr<Ice::ObjectPrx> o) { asyncCB->response(move(o)); },
                                          [asyncCB](exception_ptr e) { asyncCB->exception(e); });
        session->destroy();
    }

    void
    notifyThread()
    {
        {
            lock_guard<mutex> lg(_mutex);
            _notified = true;
        }
        _condVar.notify_one();
    }

    void
    terminate()
    {
        {
            lock_guard<mutex> lg(_mutex);
            _terminated = true;
        }
        _condVar.notify_one();
    }

protected:

    const int _id;
    const shared_ptr<RegistryPrx> _registry;
    const shared_ptr<SessionPrx> _session;
    bool _notified;
    bool _terminated;
    const bool _destroySession;
    mutex _mutex;
    condition_variable _condVar;
    random_device _rd;
};

void
allTests(Test::TestHelper* helper)
{
    auto communicator = helper->communicator();
    auto registry = Ice::checkedCast<RegistryPrx>(
        communicator->stringToProxy(communicator->getDefaultLocator()->ice_getIdentity().category + "/Registry"));
    test(registry);
    auto session = registry->createAdminSession("foo", "bar");
    session->ice_getConnection()->setACM(registry->getACMTimeout(),
                                         Ice::nullopt,
                                         Ice::ACMHeartbeat::HeartbeatAlways);

    auto admin = session->getAdmin();
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

    shared_ptr<Ice::ObjectPrx> obj;
    shared_ptr<Ice::ObjectPrx> dummy;

    try
    {
        cout << "testing create session... " << flush;
        auto session1 = registry->createSession("Client1", "");
        auto session2 = registry->createSession("Client2", "");
        cout << "ok" << endl;

        cout << "testing allocate object by identity... " << flush;

        auto allocatable = Ice::stringToIdentity("allocatable");
        auto allocatablebis = Ice::stringToIdentity("allocatablebis");

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

        auto asyncCB1 = make_shared<Callback>();
        session2->allocateObjectByIdAsync(allocatable,
                                          [&asyncCB1](shared_ptr<Ice::ObjectPrx> o) { asyncCB1->response(o); },
                                          [&asyncCB1](exception_ptr e) { asyncCB1->exception(e); });
        this_thread::sleep_for(500ms);
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
        asyncCB1 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(allocatable,
                                          [&asyncCB1](shared_ptr<Ice::ObjectPrx> o) { asyncCB1->response(o); },
                                          [&asyncCB1](exception_ptr e) { asyncCB1->exception(e); });
        this_thread::sleep_for(500ms);
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
        auto asyncCB3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::Test",
                                          [&asyncCB3](shared_ptr<Ice::ObjectPrx> o) { asyncCB3->response(o); },
                                          [&asyncCB3](exception_ptr e) { asyncCB3->exception(e); });
        this_thread::sleep_for(500ms);
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
                this_thread::sleep_for(100ms);
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

        auto time = chrono::system_clock::now();
        session2->setAllocationTimeout(500);
        try
        {
            session2->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
            test(time + 100ms < chrono::system_clock::now());
        }
        time = chrono::system_clock::now();
        try
        {
            session2->allocateObjectById(allocatable);
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        test(time + 100ms < chrono::system_clock::now());
        time = chrono::system_clock::now();
        try
        {
            session2->allocateObjectByType("::Test");
            test(false);
        }
        catch(const AllocationTimeoutException&)
        {
        }
        test(time + 100ms < chrono::system_clock::now());

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
        asyncCB1 = make_shared<Callback>();
        session2->allocateObjectByIdAsync(allocatable3,
                                          [&asyncCB1](shared_ptr<Ice::ObjectPrx> o) { asyncCB1->response(o); },
                                          [&asyncCB1](exception_ptr e) { asyncCB1->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB1->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        test(!asyncCB1->hasResponse(dummy));
        session1->releaseObject(allocatable4);
        asyncCB1->waitResponse(__FILE__, __LINE__);
        test(asyncCB1->hasResponse(dummy));
        session2->releaseObject(allocatable3);

        session1->setAllocationTimeout(allocationTimeout);
        test(session2->allocateObjectByType("::TestServer1"));
        asyncCB3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::TestServer2",
                                          [&asyncCB3](shared_ptr<Ice::ObjectPrx> o) { asyncCB3->response(o); },
                                          [&asyncCB3](exception_ptr e) { asyncCB3->exception(e); });
        this_thread::sleep_for(500ms);
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

        auto obj1 = session1->allocateObjectByType("::TestMultipleServer");
        test(obj1);
        auto obj2 = session2->allocateObjectByType("::TestMultipleServer");
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
                this_thread::sleep_for(100ms);
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
        auto asyncCB11 = make_shared<Callback>();
        auto asyncCB12 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(allocatable,
                                          [&asyncCB11](shared_ptr<Ice::ObjectPrx> o) { asyncCB11->response(o); },
                                          [&asyncCB11](exception_ptr e) { asyncCB11->exception(e); });
        session1->allocateObjectByIdAsync(allocatable,
                                          [&asyncCB12](shared_ptr<Ice::ObjectPrx> o) { asyncCB12->response(o); },
                                          [&asyncCB12](exception_ptr e) { asyncCB12->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB11->hasResponse(dummy));
        test(!asyncCB12->hasResponse(dummy));
        session2->releaseObject(allocatable);
        asyncCB11->waitResponse(__FILE__, __LINE__);
        asyncCB12->waitResponse(__FILE__, __LINE__);
        test(asyncCB11->hasResponse(dummy) ? asyncCB12->hasException() : asyncCB12->hasResponse(dummy));
        test(asyncCB12->hasResponse(dummy) ? asyncCB11->hasException() : asyncCB11->hasResponse(dummy));
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable);
        auto asyncCB31 = make_shared<Callback>();
        auto asyncCB32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::Test",
                                            [&asyncCB31](shared_ptr<Ice::ObjectPrx> o) { asyncCB31->response(o); },
                                            [&asyncCB31](exception_ptr e) { asyncCB31->exception(e); });
        session1->allocateObjectByTypeAsync("::Test",
                                            [&asyncCB32](shared_ptr<Ice::ObjectPrx> o) { asyncCB32->response(o); },
                                            [&asyncCB32](exception_ptr e) { asyncCB32->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session2->releaseObject(allocatable);
        this_thread::sleep_for(300ms);
        do
        {
            this_thread::sleep_for(200ms);
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable);
        this_thread::sleep_for(300ms);
        auto asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable3);
        asyncCB11 = make_shared<Callback>();
        asyncCB12 = make_shared<Callback>();
        session1->allocateObjectByIdAsync(allocatable3,
                                          [&asyncCB11](shared_ptr<Ice::ObjectPrx> o) { asyncCB11->response(o); },
                                          [&asyncCB11](exception_ptr e) { asyncCB11->exception(e); });
        session1->allocateObjectByIdAsync(allocatable3,
                                          [&asyncCB12](shared_ptr<Ice::ObjectPrx> o) { asyncCB12->response(o); },
                                          [&asyncCB12](exception_ptr e) { asyncCB12->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB11->hasResponse(dummy));
        test(!asyncCB12->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        asyncCB11->waitResponse(__FILE__, __LINE__);
        asyncCB12->waitResponse(__FILE__, __LINE__);
        test(asyncCB11->hasResponse(dummy) ? asyncCB12->hasException() : asyncCB12->hasResponse(dummy));
        test(asyncCB12->hasResponse(dummy) ? asyncCB11->hasException() : asyncCB11->hasResponse(dummy));
        session1->releaseObject(allocatable3);

        session2->allocateObjectById(allocatable3);
        asyncCB31 = make_shared<Callback>();
        asyncCB32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::TestServer1",
                                            [&asyncCB31](shared_ptr<Ice::ObjectPrx> o) { asyncCB31->response(o); },
                                            [&asyncCB31](exception_ptr e) { asyncCB31->exception(e); });
        session1->allocateObjectByTypeAsync("::TestServer1",
                                            [&asyncCB32](shared_ptr<Ice::ObjectPrx> o) { asyncCB32->response(o); },
                                            [&asyncCB32](exception_ptr e) { asyncCB32->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session2->releaseObject(allocatable3);
        this_thread::sleep_for(300ms);
        do
        {
            this_thread::sleep_for(200ms);
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        this_thread::sleep_for(300ms);
        asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        session1->allocateObjectById(allocatable3);
        asyncCB31 = make_shared<Callback>();
        asyncCB32 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::TestServer1",
                                            [&asyncCB31](shared_ptr<Ice::ObjectPrx> o) { asyncCB31->response(o); },
                                            [&asyncCB31](exception_ptr e) { asyncCB31->exception(e); });
        session1->allocateObjectByTypeAsync("::TestServer1",
                                            [&asyncCB32](shared_ptr<Ice::ObjectPrx> o) { asyncCB32->response(o); },
                                            [&asyncCB32](exception_ptr e) { asyncCB32->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB31->hasResponse(dummy));
        test(!asyncCB32->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        this_thread::sleep_for(300ms);
        do
        {
            this_thread::sleep_for(200ms);
        }
        while(!asyncCB31->hasResponse(dummy) && !asyncCB32->hasResponse(dummy));
        test((asyncCB31->hasResponse(dummy) && dummy && !asyncCB32->hasResponse(dummy)) ||
             (asyncCB32->hasResponse(dummy) && dummy && !asyncCB31->hasResponse(dummy)));
        session1->releaseObject(allocatable3);
        this_thread::sleep_for(300ms);
        asyncCB33 = asyncCB31->hasResponse(dummy) ? asyncCB32 : asyncCB31;
        asyncCB33->waitResponse(__FILE__, __LINE__);
        test(asyncCB33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        cout << "ok" << endl;

        cout << "testing session destroy... " << flush;

        obj = session2->allocateObjectByType("::Test"); // Allocate the object
        test(obj && obj->ice_getIdentity().name == "allocatable");

        session1->setAllocationTimeout(allocationTimeout);
        asyncCB3 = make_shared<Callback>();
        session1->allocateObjectByTypeAsync("::Test",
                                            [&asyncCB3](shared_ptr<Ice::ObjectPrx> o) { asyncCB3->response(o); },
                                            [&asyncCB3](exception_ptr e) { asyncCB3->exception(e); });
        this_thread::sleep_for(500ms);
        test(!asyncCB3->hasResponse(dummy));
        session2->destroy();
        asyncCB3->waitResponse(__FILE__, __LINE__);
        test(asyncCB3->hasResponse(obj));
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

            auto objectAllocOriginal = admin->getServerInfo("ObjectAllocation").descriptor;
            auto objectAllocUpdate = dynamic_pointer_cast<ServerDescriptor>(objectAllocOriginal->ice_clone());

            auto serverAllocOriginal = admin->getServerInfo("ServerAllocation").descriptor;
            auto serverAllocUpdate = dynamic_pointer_cast<ServerDescriptor>(serverAllocOriginal->ice_clone());

            NodeUpdateDescriptor nodeUpdate;
            nodeUpdate.name = "localnode";
            nodeUpdate.servers.push_back(objectAllocUpdate);
            nodeUpdate.servers.push_back(serverAllocUpdate);

            ApplicationUpdateDescriptor appUpdate;
            appUpdate.name = "Test";
            appUpdate.nodes.push_back(nodeUpdate);

            {
                session1->allocateObjectById(allocatable3);
                auto r2 = session2->allocateObjectByIdAsync(allocatable4);

                session1->allocateObjectById(allocatable4);
                session1->releaseObject(allocatable4);
                test(r2.wait_for(chrono::milliseconds(0)) != future_status::ready);

                serverAllocUpdate->allocatable = false;
                admin->updateApplication(appUpdate);

                test(r2.wait_for(chrono::milliseconds(0)) != future_status::ready);

                session1->releaseObject(allocatable3);
                r2.get();
                session2->releaseObject(allocatable4);

                serverAllocUpdate->allocatable = true;
                admin->updateApplication(appUpdate);
            }

            {
                session1->allocateObjectById(allocatable);
                auto r2 = session2->allocateObjectByIdAsync(allocatable);

                objectAllocUpdate->deactivationTimeout = "23";
                admin->updateApplication(appUpdate);

                session1->releaseObject(allocatable);
                r2.get();
                session2->releaseObject(allocatable);
            }

            {
                session1->allocateObjectById(allocatable);
                auto r2 = session2->allocateObjectByIdAsync(allocatable);

                vector<ObjectDescriptor> allocatables = objectAllocUpdate->adapters[0].allocatables;
                objectAllocUpdate->adapters[0].allocatables.clear(); // Remove the allocatable object
                admin->updateApplication(appUpdate);

                try
                {
                    r2.get();
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
        auto routerBase = communicator->stringToProxy("Glacier2/router:default -p 12347");
        auto router1 = Ice::checkedCast<Glacier2::RouterPrx>(routerBase->ice_connectionId("client1"));
        test(router1);

        auto sessionBase = router1->createSession("test1", "abc123");
        try
        {
            session1 = Ice::checkedCast<SessionPrx>(sessionBase->ice_connectionId("client1")->ice_router(router1));
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

        auto stressSession = registry->createSession("StressSession", "");

        const int nClients = 10;
        map<shared_ptr<StressClient>, future<void>> clients;
        random_device rd;

        for(int i = 0; i < nClients - 2; ++i)
        {
            shared_ptr<StressClient> client;
            if(rd() % 2 == 1)
            {
                client = make_shared<StressClient>(i, registry, false);
            }
            else
            {
                client = make_shared<StressClient>(i, stressSession);
            }

            clients.insert(make_pair(client, async(launch::async, [=] { client->run(); })));
        }

        {
            auto client8 = make_shared<StressClient>(8, registry, true);
            clients.insert(make_pair(client8, async(launch::async, [=] { client8->run(); })));
            auto client9 = make_shared<StressClient>(9, registry, true);
            clients.insert(make_pair(client9, async(launch::async, [=] { client9->run(); })));
        }

        for(const auto& c : clients)
        {
            c.first->notifyThread();
        }

        //
        // Let the stress client run for a bit.
        //
        this_thread::sleep_for(8s);

        //
        // Terminate the stress clients.
        //
        for(auto& c : clients)
        {
            c.first->terminate();
            c.second.get();
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
