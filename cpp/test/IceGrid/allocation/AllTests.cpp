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

class AllocateObjectCallback : public AMI_Session_allocateObject, public Callback
{
public:

    virtual void ice_response() { response(0); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<AllocateObjectCallback> AllocateObjectCallbackPtr;

class FindObjectByIdCallback : public AMI_Query_findObjectById, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<FindObjectByIdCallback> FindObjectByIdCallbackPtr;

class FindObjectByTypeCallback : public AMI_Query_findObjectByType, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<FindObjectByTypeCallback> FindObjectByTypeCallbackPtr;

class FindObjectByTypeOnLeastLoadedNodeCallback : public AMI_Query_findObjectByTypeOnLeastLoadedNode, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { exception(); }
};
typedef IceUtil::Handle<FindObjectByTypeOnLeastLoadedNodeCallback> FindObjectByTypeOnLeastLoadedNodeCallbackPtr;

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

	cout << "testing allocate object... " << flush;

	Ice::ObjectPrx allocatable = communicator->stringToProxy("allocatable");
	Ice::ObjectPrx allocatablebis = communicator->stringToProxy("allocatablebis");

	try
	{
	    session1->allocateObject(communicator->stringToProxy("dummy"));
	}
	catch(const ObjectNotRegisteredException&)
	{
	}
	try
	{
	    session1->releaseObject(communicator->stringToProxy("dummy"));
	}
	catch(const ObjectNotRegisteredException&)
	{
	}

	try
	{
	    session1->allocateObject(communicator->stringToProxy("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session2->allocateObject(communicator->stringToProxy("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session1->releaseObject(communicator->stringToProxy("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}
	try
	{
	    session2->releaseObject(communicator->stringToProxy("nonallocatable"));
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}

	session1->allocateObject(allocatable);
	try
	{
	    session1->allocateObject(allocatable);
	    test(false);
	}
	catch(const AllocationException& ex)
	{
	}

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	try
	{
	    session2->allocateObject(allocatable);
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

	session1->allocateObject(allocatablebis);
	try
	{
	    session2->allocateObject(allocatablebis);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->releaseObject(allocatablebis);
	session2->allocateObject(allocatablebis);
	try
	{
	    session1->allocateObject(allocatablebis);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session2->releaseObject(allocatablebis);
    
	session2->setAllocationTimeout(_allocationTimeout);
	AllocateObjectCallbackPtr cb1 = new AllocateObjectCallback();
	session2->allocateObject_async(cb1, allocatable);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable);
	cb1->waitResponse(__FILE__, __LINE__);
	test(cb1->hasResponse(dummy));

	session1->setAllocationTimeout(0);
	try
	{
	    session1->allocateObject(allocatable);
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
	cb1 = new AllocateObjectCallback();
	session1->allocateObject_async(cb1, allocatable);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb1->hasResponse(dummy));
	session2->releaseObject(allocatable);
	cb1->waitResponse(__FILE__, __LINE__);
	test(cb1->hasResponse(dummy));

	session1->releaseObject(allocatable);

	//
	// TODO: XXX test replicated proxy
	//

	cout << "ok" << endl;

	cout << "testing object allocation with findObjectById... " << flush;
    
	IceGrid::QueryPrx query1 = session1->getQuery();
	IceGrid::QueryPrx query2 = session2->getQuery();
    
	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	obj = query1->findObjectById(Ice::stringToIdentity("nonallocatable"));
	test(!obj);
	obj = query2->findObjectById(Ice::stringToIdentity("nonallocatable"));
	test(!obj);

	obj = query1->findObjectById(Ice::stringToIdentity("allocatable")); // Allocate the object
	test(obj);
	test(!query1->findObjectById(Ice::stringToIdentity("allocatable")));
	test(!query2->findObjectById(Ice::stringToIdentity("allocatable")));
	try
	{
	    session2->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	session1->releaseObject(obj);
	try
	{
	    session1->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	obj = query2->findObjectById(Ice::stringToIdentity("allocatable")); // Allocate the object
	test(!query2->findObjectById(Ice::stringToIdentity("allocatable")));
	test(!query1->findObjectById(Ice::stringToIdentity("allocatable")));

	query1->findObjectById(allocatablebis->ice_getIdentity());
	test(!query2->findObjectById(allocatablebis->ice_getIdentity()));
	session1->releaseObject(allocatablebis);
	query2->findObjectById(allocatablebis->ice_getIdentity());
	test(!query1->findObjectById(allocatablebis->ice_getIdentity()));
	session2->releaseObject(allocatablebis);

	session1->setAllocationTimeout(_allocationTimeout);
	FindObjectByIdCallbackPtr cb2 = new FindObjectByIdCallback();
	query1->findObjectById_async(cb2, Ice::stringToIdentity("allocatable"));
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb2->hasResponse(dummy));
	session2->releaseObject(allocatable);
	cb2->waitResponse(__FILE__, __LINE__);
	test(cb2->hasResponse(dummy));

	session1->releaseObject(allocatable);

	cout << "ok" << endl;

	cout << "testing object allocation with findObjectByType... " << flush;
    
	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	obj = query1->findObjectByType("::Test");
	test(obj && obj->ice_getIdentity().name == "allocatable");
	test(!query1->findObjectByType("::Test"));
	test(!query2->findObjectByType("::Test"));
	try
	{
	    session2->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	session1->releaseObject(obj);
	try
	{
	    session1->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	obj = query2->findObjectByType("::Test"); // Allocate the object
	test(obj && obj->ice_getIdentity().name == "allocatable");
	test(!query2->findObjectByType("::Test"));
	test(!query1->findObjectByType("::Test"));

	query1->findObjectByType("::TestBis");
	test(!query2->findObjectByType("::TestBis"));
	session1->releaseObject(allocatablebis);
	query2->findObjectByType("::TestBis");
	test(!query1->findObjectByType("::TestBis"));
	session2->releaseObject(allocatablebis);

	session1->setAllocationTimeout(_allocationTimeout);
	FindObjectByTypeCallbackPtr cb3 = new FindObjectByTypeCallback();
	query1->findObjectByType_async(cb3, "::Test");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->releaseObject(obj);
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(obj));

	session1->releaseObject(obj);
    
	cout << "ok" << endl;

	cout << "testing object allocation with findObjectByTypeOnLeastLoadedNode... " << flush;
    
	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	obj = query1->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1);
	test(obj && obj->ice_getIdentity().name == "allocatable");
	test(!query1->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1));
	test(!query2->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1));
	try
	{
	    session2->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	session1->releaseObject(obj);
	try
	{
	    session1->releaseObject(obj);
	}
	catch(const AllocationException&)
	{
	}

	obj = query2->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1); // Allocate the object
	test(obj && obj->ice_getIdentity().name == "allocatable");
	test(!query2->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1));
	test(!query1->findObjectByTypeOnLeastLoadedNode("::Test", LoadSample1));

	query1->findObjectByTypeOnLeastLoadedNode("::TestBis", LoadSample1);
	test(!query2->findObjectByTypeOnLeastLoadedNode("::TestBis", LoadSample1));
	session1->releaseObject(allocatablebis);
	query2->findObjectByTypeOnLeastLoadedNode("::TestBis", LoadSample1);
	test(!query1->findObjectByTypeOnLeastLoadedNode("::TestBis", LoadSample1));
	session2->releaseObject(allocatablebis);

	session1->setAllocationTimeout(_allocationTimeout);
	FindObjectByTypeOnLeastLoadedNodeCallbackPtr cb4 = new FindObjectByTypeOnLeastLoadedNodeCallback();
	query1->findObjectByTypeOnLeastLoadedNode_async(cb4, "::Test", LoadSample1);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb4->hasResponse(dummy));
	session2->releaseObject(obj);
	cb4->waitResponse(__FILE__, __LINE__);
	test(cb4->hasResponse(obj));

	session1->releaseObject(obj);
    
	cout << "ok" << endl;

	cout << "testing object allocation with Ice::Locator... " << flush;
    
	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	Ice::LocatorPrx locator1 = session1->getLocator();
	Ice::LocatorPrx locator2 = session2->getLocator();    

	communicator->stringToProxy("nonallocatable")->ice_locator(locator1)->ice_ping();
	communicator->stringToProxy("nonallocatable")->ice_locator(locator2)->ice_ping();

	Ice::ObjectPrx obj1 = allocatable->ice_locator(locator1);
	Ice::ObjectPrx obj2 = allocatable->ice_locator(locator2);

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
	    session2->releaseObject(obj2);
	}
	catch(const AllocationException&)
	{
	}

	session1->releaseObject(obj1);
	try
	{
	    session1->releaseObject(obj1);
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
	session2->releaseObject(obj2);

	cout << "ok" << endl;

	cout << "testing object allocation timeout... " << flush;    

	session1->allocateObject(allocatable);
	IceUtil::Time time = IceUtil::Time::now();
	session2->setAllocationTimeout(500);
	try
	{
	    session2->allocateObject(allocatable);
	    test(false);
	}
	catch(const AllocationTimeoutException& ex)
	{
	    test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
	}
	time = IceUtil::Time::now();
	test(!query2->findObjectById(Ice::stringToIdentity("allocatable")));
	test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());
	time = IceUtil::Time::now();
	test(!query2->findObjectByType("::Test"));
	test(time + IceUtil::Time::milliSeconds(100) < IceUtil::Time::now());

	session1->releaseObject(allocatable);
	session2->setAllocationTimeout(0);

	cout << "ok" << endl;

	cout << "testing adapter allocation... " << flush;

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	Ice::ObjectPrx allocatable1 = communicator->stringToProxy("allocatable1");
	Ice::ObjectPrx allocatable2 = communicator->stringToProxy("allocatable2");

	session1->allocateObject(allocatable1);
	try
	{
	    session2->allocateObject(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session2->allocateObject(allocatable2);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->allocateObject(allocatable2);
	session1->releaseObject(allocatable1);
	try
	{
	    session2->allocateObject(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->releaseObject(allocatable2);
	session2->allocateObject(allocatable1);
	try
	{
	    session1->allocateObject(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session1->allocateObject(allocatable2);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session2->allocateObject(allocatable2);
	session2->releaseObject(allocatable1);
	try
	{
	    session1->allocateObject(allocatable1);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	test(!query1->findObjectByType("::TestAdapter1"));
	test(!query1->findObjectByType("::TestAdapter2"));
	test(query2->findObjectByType("::TestAdapter1"));
	test(!query2->findObjectByType("::TestAdapter1"));
	test(!query2->findObjectByType("::TestAdapter2"));
	session2->releaseObject(allocatable1);
	session2->releaseObject(allocatable2);

	session1->allocateObject(allocatable1);
	session1->allocateObject(allocatable2);

	session2->setAllocationTimeout(_allocationTimeout);
	cb1 = new AllocateObjectCallback();
	session2->allocateObject_async(cb1, allocatable1);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable1);
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable2);
	cb1->waitResponse(__FILE__, __LINE__);
	test(cb1->hasResponse(dummy));
	session2->releaseObject(allocatable1);

	session1->setAllocationTimeout(_allocationTimeout);
	test(query2->findObjectByType("::TestAdapter1"));
	cb3 = new FindObjectByTypeCallback();
	query1->findObjectByType_async(cb3, "::TestAdapter2");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->releaseObject(allocatable1);
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(dummy));
	session1->releaseObject(allocatable2);    

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);
	test(query1->findObjectByType("::TestMultipleByAdapter"));
	test(!query2->findObjectByType("::TestMultipleByAdapter"));
	test(query1->findObjectByType("::TestMultipleByAdapter"));
	session1->releaseObject(communicator->stringToProxy("allocatable11"));
	session1->releaseObject(communicator->stringToProxy("allocatable21"));
	test(query2->findObjectByType("::TestMultipleByAdapter"));
	test(!query1->findObjectByType("::TestMultipleByAdapter"));
	test(query2->findObjectByType("::TestMultipleByAdapter"));
	session2->releaseObject(communicator->stringToProxy("allocatable11"));
	session2->releaseObject(communicator->stringToProxy("allocatable21"));

	cout << "ok" << endl;

	cout << "testing server allocation... " << flush;

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);

	Ice::ObjectPrx allocatable3 = communicator->stringToProxy("allocatable3");
	Ice::ObjectPrx allocatable4 = communicator->stringToProxy("allocatable4");

	session1->allocateObject(allocatable3);
	try
	{
	    session2->allocateObject(allocatable3);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session2->allocateObject(allocatable4);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->allocateObject(allocatable4);
	session1->releaseObject(allocatable3);
	try
	{
	    session2->allocateObject(allocatable3);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session1->releaseObject(allocatable4);
	session2->allocateObject(allocatable3);
	try
	{
	    session1->allocateObject(allocatable3);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	try
	{
	    session1->allocateObject(allocatable4);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	session2->allocateObject(allocatable4);
	session2->releaseObject(allocatable3);
	try
	{
	    session1->allocateObject(allocatable3);
	    test(false);
	}
	catch(const AllocationTimeoutException&)
	{
	}
	test(!query1->findObjectByType("::TestServer1"));
	test(!query1->findObjectByType("::TestServer2"));
	test(query2->findObjectByType("::TestServer1"));
	test(!query2->findObjectByType("::TestServer1"));
	test(!query2->findObjectByType("::TestServer2"));
	session2->releaseObject(allocatable3);
	session2->releaseObject(allocatable4);

	session1->allocateObject(allocatable3);
	session1->allocateObject(allocatable4);

	session2->setAllocationTimeout(_allocationTimeout);
	cb1 = new AllocateObjectCallback();
	session2->allocateObject_async(cb1, allocatable3);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable3);
	test(!cb1->hasResponse(dummy));
	session1->releaseObject(allocatable4);
	cb1->waitResponse(__FILE__, __LINE__);
	test(cb1->hasResponse(dummy));
	session2->releaseObject(allocatable3);

	session1->setAllocationTimeout(_allocationTimeout);
	test(query2->findObjectByType("::TestServer1"));
	cb3 = new FindObjectByTypeCallback();
	query1->findObjectByType_async(cb3, "::TestServer2");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->releaseObject(allocatable3);
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(dummy));
	session1->releaseObject(allocatable4);    

	session1->setAllocationTimeout(0);
	session2->setAllocationTimeout(0);
	test(query1->findObjectByType("::TestMultipleByServer"));
	test(!query2->findObjectByType("::TestMultipleByServer"));
	test(query1->findObjectByType("::TestMultipleByServer"));
	session1->releaseObject(communicator->stringToProxy("allocatable31"));
	session1->releaseObject(communicator->stringToProxy("allocatable41"));
	test(query2->findObjectByType("::TestMultipleByServer"));
	test(!query1->findObjectByType("::TestMultipleByServer"));
	test(query2->findObjectByType("::TestMultipleByServer"));
	session2->releaseObject(communicator->stringToProxy("allocatable31"));
	session2->releaseObject(communicator->stringToProxy("allocatable41"));

	cout << "ok" << endl;

	cout << "testing concurrent allocations... " << flush;

	session1->setAllocationTimeout(_allocationTimeout);
	session2->setAllocationTimeout(_allocationTimeout);

	session2->allocateObject(allocatable);
	AllocateObjectCallbackPtr cb11 = new AllocateObjectCallback();
	AllocateObjectCallbackPtr cb12 = new AllocateObjectCallback();
	session1->allocateObject_async(cb11, allocatable);
	session1->allocateObject_async(cb12, allocatable);
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb11->hasResponse(dummy));
	test(!cb12->hasResponse(dummy));
	session2->releaseObject(allocatable);
	cb11->waitResponse(__FILE__, __LINE__);
	cb12->waitResponse(__FILE__, __LINE__);
	test(cb11->hasResponse(dummy) ? cb12->hasException() : cb12->hasResponse(dummy));
	test(cb12->hasResponse(dummy) ? cb11->hasException() : cb11->hasResponse(dummy));
	session1->releaseObject(allocatable);

	session2->allocateObject(allocatable);
	FindObjectByIdCallbackPtr cb21 = new FindObjectByIdCallback();
	FindObjectByIdCallbackPtr cb22 = new FindObjectByIdCallback();
	query1->findObjectById_async(cb21, Ice::stringToIdentity("allocatable"));
	query1->findObjectById_async(cb22, Ice::stringToIdentity("allocatable"));
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb21->hasResponse(dummy));
	test(!cb22->hasResponse(dummy));
	session2->releaseObject(allocatable);
	cb21->waitResponse(__FILE__, __LINE__);
	cb22->waitResponse(__FILE__, __LINE__);
	Ice::ObjectPrx dummy1;
	test(cb21->hasResponse(dummy1));
	Ice::ObjectPrx dummy2;
	test(cb22->hasResponse(dummy2));
	test(dummy1 && !dummy2 || dummy2);
	session1->releaseObject(allocatable);

	session2->allocateObject(allocatable);
	FindObjectByTypeCallbackPtr cb31 = new FindObjectByTypeCallback();
	FindObjectByTypeCallbackPtr cb32 = new FindObjectByTypeCallback();
	query1->findObjectByType_async(cb31, "::Test");
	query1->findObjectByType_async(cb32, "::Test");
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
	FindObjectByTypeCallbackPtr cb33 = cb31->hasResponse(dummy) ? cb32 : cb31;
	cb33->waitResponse(__FILE__, __LINE__);
	test(cb33->hasResponse(dummy) && dummy);
	session1->releaseObject(allocatable);

	cout << "ok" << endl;

	cout << "testing session destroy... " << flush;

	obj = query2->findObjectByType("::Test"); // Allocate the object
	test(obj && obj->ice_getIdentity().name == "allocatable");

	session1->setAllocationTimeout(_allocationTimeout);
	cb3 = new FindObjectByTypeCallback();
	query1->findObjectByType_async(cb3, "::Test");
	IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
	test(!cb3->hasResponse(dummy));
	session2->destroy();
	cb3->waitResponse(__FILE__, __LINE__);
	test(cb3->hasResponse(obj));
	session1->destroy();

	session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
	session2->setAllocationTimeout(0);
	session2->allocateObject(allocatable);
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
