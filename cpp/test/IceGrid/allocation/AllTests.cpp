// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/Session.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Query.h>
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
    waitResponse(char* file, int line)
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
    terminate()
    {
	Lock sync(*this);
	_terminated = true;
	notify();
    }

private:

    const Ice::LoggerPtr _logger;
    vector<SessionPrx> _sessions;
    const IceUtil::Time _timeout;
    bool _terminated;
};
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    SessionManagerPrx manager = SessionManagerPrx::checkedCast(communicator->stringToProxy("IceGrid/SessionManager"));
    test(manager);

    SessionKeepAliveThreadPtr keepAlive;
    keepAlive = new SessionKeepAliveThread(communicator->getLogger(), IceUtil::Time::seconds(5));
    keepAlive->start();

    const int _allocationTimeout = 5000;

    Ice::ObjectPrx obj;
    Ice::ObjectPrx dummy;

    try
    {
	cout << "testing create session... " << flush;
	SessionPrx session1 = SessionPrx::uncheckedCast(manager->createLocalSession("Client1"));
	SessionPrx session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
	
	keepAlive->add(session1);
	keepAlive->add(session2);
    
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
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session2->allocateObjectById(Ice::stringToIdentity("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session1->releaseObject(Ice::stringToIdentity("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session2->releaseObject(Ice::stringToIdentity("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}

	session1->allocateObjectById(allocatable);
	try
	{
	    session1->allocateObjectById(allocatable);
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	try
	{
	    session2->allocateObjectById(allocatable);
	    test(false);
	}
	catch(const AllocationTimeoutException& ex)
	{
	}
	try
	{
	    session2->releaseObject(allocatable);
	    test(false);
	}
	catch(const AllocationException& ex)
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
    
	session2->setAllocationTimeout(_allocationTimeout);
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
	catch(const AllocationTimeoutException& ex)
	{
	}
	try
	{
	    session1->releaseObject(allocatable);
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}

	session1->setAllocationTimeout(_allocationTimeout);
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
	session2->setAllocationTimeout(obj);

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
	catch(const AllocationException&)
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
	catch(const AllocationException&)
	{
	}
	session1->allocateObjectByType("::TestBis");
	try
	{
	    session2->allocateObjectByType("::TestBis");
	    test(false);
	}
	catch(const AllocationException&)
	{
	}
	session1->releaseObject(allocatablebis);
	session2->allocateObjectByType("::TestBis");
	try
	{
	    session1->allocateObjectByType("::TestBis");
	    test(false);
	}
	catch(const AllocationException&)
	{
	}
	session2->releaseObject(allocatablebis);

	session1->setAllocationTimeout(_allocationTimeout);
	AllocateObjectByTypeCallbackPtr cb3 = new AllocateObjectByTypeCallback();
	session1->allocateObjectByType_async(cb3, "::Test");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->releaseObject(obj->ice_getIdentity());
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(obj));

	session1->releaseObject(obj->ice_getIdentity());
    
	cout << "ok" << endl;

	cout << "testing object allocation with Ice::Locator... " << flush;
    
	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	communicator->stringToProxy("nonallocatable")->ice_ping();
	try
	{
	    communicator->stringToProxy("allocatable")->ice_ping();
	}
	catch(const Ice::NoEndpointException& ex)
	{
	}

	Ice::LocatorPrx locator1 = session1->getLocator();
	Ice::LocatorPrx locator2 = session2->getLocator();    

	communicator->stringToProxy("nonallocatable")->ice_locator(locator1)->ice_ping();
	communicator->stringToProxy("nonallocatable")->ice_locator(locator2)->ice_ping();

	Ice::ObjectPrx obj1 = communicator->stringToProxy("allocatable")->ice_locator(locator1);
	Ice::ObjectPrx obj2 = communicator->stringToProxy("allocatable")->ice_locator(locator2);

	obj1->ice_ping(); // Allocate the object
	obj1->ice_locatorCacheTimeout(0)->ice_ping();
	try
	{
	    obj2->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}
	try
	{
	    session2->releaseObject(obj2->ice_getIdentity());
	}
	catch(const AllocationException&)
	{
	}

	session1->releaseObject(obj1->ice_getIdentity());
	try
	{
	    session1->releaseObject(obj1->ice_getIdentity());
	}
	catch(const AllocationException&)
	{
	}

	obj2->ice_ping(); // Allocate the object
	obj2->ice_locatorCacheTimeout(0)->ice_ping();
	try
	{
	    obj1->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}
	session2->releaseObject(obj2->ice_getIdentity());

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
	catch(const AllocationTimeoutException& ex)
	{
	    test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
	}
	time = IceUtil::Time::now();
	try
	{
	    session2->allocateObjectById(allocatable);
	    test(false);
	}
	catch(const AllocationException&)
	{
	}
	test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
	time = IceUtil::Time::now();
	try
	{
	    session2->allocateObjectByType("::Test");
	    test(false);
	}
	catch(const AllocationException&)
	{
	}
	test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());

	session1->releaseObject(allocatable);
	session2->setAllocationTimeout(0);

	cout << "ok" << endl;

	cout << "testing adapter allocation... " << flush;

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	Ice::Identity allocatable1 = Ice::stringToIdentity("allocatable1");
	Ice::Identity allocatable2 = Ice::stringToIdentity("allocatable2");

	session1->allocateObjectById(allocatable1);
	try
	{
	    session2->allocateObjectById(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session2->allocateObjectById(allocatable2);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}

	Ice::ObjectPrx session1obj1 = communicator->stringToProxy("allocatable1@AdapterAlloc")->ice_locator(locator1);
	Ice::ObjectPrx session1obj2 = communicator->stringToProxy("allocatable2@AdapterAlloc")->ice_locator(locator1);
	Ice::ObjectPrx session2obj1 = communicator->stringToProxy("allocatable1@AdapterAlloc")->ice_locator(locator2);
	Ice::ObjectPrx session2obj2 = communicator->stringToProxy("allocatable2@AdapterAlloc")->ice_locator(locator2);
	session1obj1->ice_locatorCacheTimeout(0)->ice_ping();
	try
	{
	    session2obj1->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}
	try
	{
	    session2obj2->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}

	session1->allocateObjectById(allocatable2);
	session1->releaseObject(allocatable1);
	try
	{
	    session2->allocateObjectById(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->releaseObject(allocatable2);
	session2->allocateObjectById(allocatable1);
	try
	{
	    session1->allocateObjectById(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session1->allocateObjectById(allocatable2);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session2->allocateObjectById(allocatable2);
	session2->releaseObject(allocatable1);
	try
	{
	    session1->allocateObjectById(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session1->allocateObjectByType("::TestAdapter1");
	    test(false);
	}
	catch(const AllocationException&)
	{
	}
	try
	{
	    session1->allocateObjectByType("::TestAdapter2");
	    test(false);
	}
	catch(AllocationException&)
	{
	}
	test(session2->allocateObjectByType("::TestAdapter1"));
	try
	{
	    session2->allocateObjectByType("::TestAdapter1");
	    test(false);
	}
	catch(AllocationException&)
	{
	}
	try
	{
	    session2->allocateObjectByType("::TestAdapter2");
	    test(false);
	}
	catch(AllocationException&)
	{
	}
	session2->releaseObject(allocatable1);
	session2->releaseObject(allocatable2);

	session1->allocateObjectById(allocatable1);
	session1->allocateObjectById(allocatable2);

	session2->setAllocationTimeout(_allocationTimeout);
	cb1 = new AllocateObjectByIdCallback();
	session2->allocateObjectById_async(cb1, allocatable1);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable1);
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable2);
	cb1->waitResponse(__FILE__, __LINE__);
	test(cb1->hasResponse(dummy));
	session2->releaseObject(allocatable1);

	session1->setAllocationTimeout(_allocationTimeout);
	test(session2->allocateObjectByType("::TestAdapter1"));
	cb3 = new AllocateObjectByTypeCallback();
	session1->allocateObjectByType_async(cb3, "::TestAdapter2");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->releaseObject(allocatable1);
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(dummy));
	session1->releaseObject(allocatable2);    

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);
	test(session1->allocateObjectByType("::TestMultipleByAdapter"));
	try
	{
	    session2->allocateObjectByType("::TestMultipleByAdapter");
	    test(false);
	}
	catch(AllocationException&)
	{
	}
	test(session1->allocateObjectByType("::TestMultipleByAdapter"));
	session1->releaseObject(Ice::stringToIdentity("allocatable11"));
	session1->releaseObject(Ice::stringToIdentity("allocatable21"));
	test(session2->allocateObjectByType("::TestMultipleByAdapter"));
	try
	{
	    session1->allocateObjectByType("::TestMultipleByAdapter");
	    test(false);
	}
	catch(AllocationException&)
	{
	}
	test(session2->allocateObjectByType("::TestMultipleByAdapter"));
	session2->releaseObject(Ice::stringToIdentity("allocatable11"));
	session2->releaseObject(Ice::stringToIdentity("allocatable21"));

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

	Ice::ObjectPrx session1obj3 = communicator->stringToProxy("allocatable3@ServerAlloc")->ice_locator(locator1);
	Ice::ObjectPrx session1obj4 = communicator->stringToProxy("allocatable4@ServerAlloc")->ice_locator(locator1);
	Ice::ObjectPrx session2obj3 = communicator->stringToProxy("allocatable3@ServerAlloc")->ice_locator(locator2);
	Ice::ObjectPrx session2obj4 = communicator->stringToProxy("allocatable4@ServerAlloc")->ice_locator(locator2);
	session1obj3->ice_locatorCacheTimeout(0)->ice_ping();
	try
	{
	    session2obj3->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
	{
	}
	try
	{
	    session2obj4->ice_locatorCacheTimeout(0)->ice_ping();
	    test(false);
	}
	catch(const Ice::NoEndpointException&)
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

	session2->setAllocationTimeout(_allocationTimeout);
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

	session1->setAllocationTimeout(_allocationTimeout);
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
	session1->releaseObject(Ice::stringToIdentity("allocatable31"));
	session1->releaseObject(Ice::stringToIdentity("allocatable41"));
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
	session2->releaseObject(Ice::stringToIdentity("allocatable31"));
	session2->releaseObject(Ice::stringToIdentity("allocatable41"));

	cout << "ok" << endl;

	cout << "testing concurrent allocations... " << flush;

	session1->setAllocationTimeout(_allocationTimeout);
	session2->setAllocationTimeout(_allocationTimeout);

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

	cout << "ok" << endl;

	cout << "testing session destroy... " << flush;

	obj = session2->allocateObjectByType("::Test"); // Allocate the object
	test(obj && obj->ice_getIdentity().name == "allocatable");

	session1->setAllocationTimeout(_allocationTimeout);
	cb3 = new AllocateObjectByTypeCallback();
	session1->allocateObjectByType_async(cb3, "::Test");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->destroy();
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(obj));
	session1->destroy();

	session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
	session2->setAllocationTimeout(0);
	session2->allocateObjectById(allocatable);
	session2->destroy();

	cout << "ok" << endl;
    }
    catch(const NotAllocatableException& ex)
    {
	cerr << ex << endl;
	test(false);
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
}
