// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <IceUtil/Random.h>
#include <Ice/Ice.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Query.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

class Callback : public IceUtil::Monitor<IceUtil::Mutex>
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
    exception()
    {
        Lock sync(*this);
        _exception = true;
        notify();
    }

    void
    waitResponse(const char* file, int line)
    {
        Lock sync(*this);
        while(!_response && !_exception)
        {
            if(!timedWait(IceUtil::Time::seconds(3)))
            {
                cerr << "timeout: " << file << ":" << line << endl;
                test(false); // Timeout
            }
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

class AllocateObjectByIdCallback : public AMI_Session_allocateObjectById, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<AllocateObjectByIdCallback> AllocateObjectByIdCallbackPtr;

class AllocateObjectByTypeCallback : public AMI_Session_allocateObjectByType, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<AllocateObjectByTypeCallback> AllocateObjectByTypeCallbackPtr;

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
                    session->setAllocationTimeout(IceUtil::random(200)); // 200ms timeout
                }
            }

            assert(session);
            session->keepAlive();

            Ice::ObjectPrx object;
            switch(IceUtil::random(_destroySession ? 4 : 2))
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
                IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(IceUtil::random(20)));
                switch(IceUtil::random(_destroySession ? 2 : 1))
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
        os << "stress-" << IceUtil::random(6) + 1;
        try
        {
            return session->allocateObjectById(_communicator->stringToIdentity(os.str()));
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
        os << "stress-" << IceUtil::random(3);
        session->allocateObjectById_async(new AllocateObjectByIdCallback(), _communicator->stringToIdentity(os.str()));
        session->destroy();
    }

    void
    allocateByTypeAndDestroy(const SessionPrx& session)
    {
        session->allocateObjectByType_async(new AllocateObjectByTypeCallback(), "::StressTest");
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

class SessionKeepAliveThread : public IceUtil::Thread, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    SessionKeepAliveThread(const Ice::LoggerPtr& logger, const IceUtil::Time& timeout) :
        _logger(logger),
        _timeout(timeout),
        _terminated(false)
    {
    }

    virtual void
    run()
    {
        Lock sync(*this);
        while(!_terminated)
        {
            timedWait(_timeout);
            if(!_terminated)
            {
                vector<SessionPrx>::iterator p = _sessions.begin();
                while(p != _sessions.end())
                {
                    try
                    {
                        (*p)->keepAlive();
                        ++p;
                    }
                    catch(const Ice::Exception&)
                    {
                        p = _sessions.erase(p);
                    }
                }

                vector<AdminSessionPrx>::iterator q = _adminSessions.begin();
                while(q != _adminSessions.end())
                {
                    try
                    {
                        (*q)->keepAlive();
                        ++q;
                    }
                    catch(const Ice::Exception&)
                    {
                        q = _adminSessions.erase(q);
                    }
                }
            }
        }
    }

    void 
    add(const SessionPrx& session)
    {
        Lock sync(*this);
        _sessions.push_back(session);
    }

    void 
    add(const AdminSessionPrx& session)
    {
        Lock sync(*this);
        _adminSessions.push_back(session);
    }

    void
    terminate()
    {
        Lock sync(*this);
        _terminated = true;
        notify();
    }

private:

    const Ice::LoggerPtr _logger;
    vector<SessionPrx> _sessions;
    vector<AdminSessionPrx> _adminSessions;
    const IceUtil::Time _timeout;
    bool _terminated;
};
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    SessionKeepAliveThreadPtr keepAlive = new SessionKeepAliveThread(
        communicator->getLogger(), IceUtil::Time::seconds(5));
    keepAlive->start();

    RegistryPrx registry = IceGrid::RegistryPrx::checkedCast(communicator->stringToProxy("IceGrid/Registry"));
    test(registry);
    AdminSessionPrx session = registry->createAdminSession("foo", "bar");

    keepAlive->add(session);

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
        
        keepAlive->add(session1);
        keepAlive->add(session2);
    
        cout << "ok" << endl;

        cout << "testing allocate object by identity... " << flush;

        Ice::Identity allocatable = communicator->stringToIdentity("allocatable");
        Ice::Identity allocatablebis = communicator->stringToIdentity("allocatablebis");

        try
        {
            session1->allocateObjectById(communicator->stringToIdentity("dummy"));
        }
        catch(const ObjectNotRegisteredException&)
        {
        }
        try
        {
            session1->releaseObject(communicator->stringToIdentity("dummy"));
        }
        catch(const ObjectNotRegisteredException&)
        {
        }

        try
        {
            session1->allocateObjectById(communicator->stringToIdentity("nonallocatable"));
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
            session2->allocateObjectById(communicator->stringToIdentity("nonallocatable"));
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
            session1->releaseObject(communicator->stringToIdentity("nonallocatable"));
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
            session2->releaseObject(communicator->stringToIdentity("nonallocatable"));
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
        AllocateObjectByIdCallbackPtr cb1 = new AllocateObjectByIdCallback();
        session2->allocateObjectById_async(cb1, allocatable);
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
        cb1 = new AllocateObjectByIdCallback();
        session1->allocateObjectById_async(cb1, allocatable);
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
        AllocateObjectByTypeCallbackPtr cb3 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb3, "::Test");
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb3->hasResponse(dummy));
        session2->releaseObject(obj->ice_getIdentity());
        cb3->waitResponse(__FILE__, __LINE__);
        test(cb3->hasResponse(obj));

        session1->releaseObject(obj->ice_getIdentity());
    
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

        Ice::Identity allocatable3 = communicator->stringToIdentity("allocatable3");
        Ice::Identity allocatable4 = communicator->stringToIdentity("allocatable4");

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
        catch(AllocationException&)
        {
        }
        try
        {
            session1->allocateObjectByType("::TestServer2");
            test(false);
        }
        catch(AllocationException&)
        {
        }
        test(session2->allocateObjectByType("::TestServer1"));
        try
        {
            session2->allocateObjectByType("::TestServer1");
            test(false);
        }
        catch(AllocationException&)
        {
        }
        try
        {
            session2->allocateObjectByType("::TestServer2");
            test(false);
        }
        catch(AllocationException&)
        {
        }
        session2->releaseObject(allocatable3);
        session2->releaseObject(allocatable4);

        session1->allocateObjectById(allocatable3);
        session1->allocateObjectById(allocatable4);

        session2->setAllocationTimeout(allocationTimeout);
        cb1 = new AllocateObjectByIdCallback();
        session2->allocateObjectById_async(cb1, allocatable3);
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
        cb3 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb3, "::TestServer2");
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
        catch(AllocationException&)
        {
        }
        test(session1->allocateObjectByType("::TestMultipleByServer"));
        session1->releaseObject(communicator->stringToIdentity("allocatable31"));
        session1->releaseObject(communicator->stringToIdentity("allocatable41"));
        test(session2->allocateObjectByType("::TestMultipleByServer"));
        try
        {
            session1->allocateObjectByType("::TestMultipleByServer");
            test(false);
        }
        catch(AllocationException&)
        {
        }
        test(session2->allocateObjectByType("::TestMultipleByServer"));
        session2->releaseObject(communicator->stringToIdentity("allocatable31"));
        session2->releaseObject(communicator->stringToIdentity("allocatable41"));

        Ice::ObjectPrx obj1 = session1->allocateObjectByType("::TestMultipleServer");
        test(obj1);
        Ice::ObjectPrx obj2 = session2->allocateObjectByType("::TestMultipleServer");
        test(obj2);
        try
        {
            session1->allocateObjectByType("::TestMultipleServer");
            test(false);
        }
        catch(AllocationTimeoutException&)
        {
        }
        try
        {
            session2->allocateObjectByType("::TestMultipleServer");
            test(false);
        }
        catch(AllocationTimeoutException&)
        {
        }       
        session1->releaseObject(obj1->ice_getIdentity());
        obj1 = session2->allocateObjectByType("::TestMultipleServer");
        session2->releaseObject(obj1->ice_getIdentity());
        session2->releaseObject(obj2->ice_getIdentity());

        cout << "ok" << endl;

        cout << "testing concurrent allocations... " << flush;

        session1->setAllocationTimeout(allocationTimeout);
        session2->setAllocationTimeout(allocationTimeout);

        session2->allocateObjectById(allocatable);
        AllocateObjectByIdCallbackPtr cb11 = new AllocateObjectByIdCallback();
        AllocateObjectByIdCallbackPtr cb12 = new AllocateObjectByIdCallback();
        session1->allocateObjectById_async(cb11, allocatable);
        session1->allocateObjectById_async(cb12, allocatable);
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
        AllocateObjectByTypeCallbackPtr cb31 = new AllocateObjectByTypeCallback();
        AllocateObjectByTypeCallbackPtr cb32 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb31, "::Test");
        session1->allocateObjectByType_async(cb32, "::Test");
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
        test(cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy) ||
             cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy));
        session1->releaseObject(allocatable);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        AllocateObjectByTypeCallbackPtr cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
        cb33->waitResponse(__FILE__, __LINE__);
        test(cb33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable);

        session2->allocateObjectById(allocatable3);
        cb11 = new AllocateObjectByIdCallback();
        cb12 = new AllocateObjectByIdCallback();
        session1->allocateObjectById_async(cb11, allocatable3);
        session1->allocateObjectById_async(cb12, allocatable3);
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
        cb31 = new AllocateObjectByTypeCallback();
        cb32 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb31, "::TestServer1");
        session1->allocateObjectByType_async(cb32, "::TestServer1");
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
        test(cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy) ||
             cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy));
        session1->releaseObject(allocatable3);
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(300));
        cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
        cb33->waitResponse(__FILE__, __LINE__);
        test(cb33->hasResponse(dummy) && dummy);
        session1->releaseObject(allocatable3);

        session1->allocateObjectById(allocatable3);
        cb31 = new AllocateObjectByTypeCallback();
        cb32 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb31, "::TestServer1");
        session1->allocateObjectByType_async(cb32, "::TestServer1");
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
        test(cb31->hasResponse(dummy) && dummy && !cb32->hasResponse(dummy) ||
             cb32->hasResponse(dummy) && dummy && !cb31->hasResponse(dummy));
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
        cb3 = new AllocateObjectByTypeCallback();
        session1->allocateObjectByType_async(cb3, "::Test");
        IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
        test(!cb3->hasResponse(dummy));
        session2->destroy();    
        cb3->waitResponse(__FILE__, __LINE__);
        test(cb3->hasResponse(obj));
        session1->destroy();

        session2 = SessionPrx::uncheckedCast(registry->createSession("Client2", ""));
        session2->setAllocationTimeout(0);
        session2->allocateObjectById(allocatable);
        session2->destroy();

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
        keepAlive->add(stressSession);

        const int nClients = 10;
        int i;
        vector<StressClientPtr> clients;
        for(i = 0; i < nClients - 2; ++i)
        {
            if(IceUtil::random(2) == 1)
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

    cout << "shutting down router... " << flush;
    admin->stopServer("Glacier2");
    cout << "ok" << endl;

    keepAlive->terminate();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    session->destroy();
}
