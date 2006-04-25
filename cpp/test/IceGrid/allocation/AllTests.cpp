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

    Callback() : _response(false)
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
    wait()
    {
	Lock sync(*this);
	while(!_response)
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

private:

    bool _response;
    Ice::ObjectPrx _obj;
};

class AllocateObjectCallback : public AMI_Session_allocateObject, public Callback
{
public:

    virtual void ice_response() { response(0); }
    virtual void ice_exception(const Ice::Exception&) { }
};
typedef IceUtil::Handle<AllocateObjectCallback> AllocateObjectCallbackPtr;

class FindObjectByIdCallback : public AMI_Query_findObjectById, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { }
};
typedef IceUtil::Handle<FindObjectByIdCallback> FindObjectByIdCallbackPtr;

class FindObjectByTypeCallback : public AMI_Query_findObjectByType, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { }
};
typedef IceUtil::Handle<FindObjectByTypeCallback> FindObjectByTypeCallbackPtr;

class FindObjectByTypeOnLeastLoadedNodeCallback : public AMI_Query_findObjectByTypeOnLeastLoadedNode, public Callback
{
public:

    virtual void ice_response(const Ice::ObjectPrx& obj) { response(obj); }
    virtual void ice_exception(const Ice::Exception&) { }
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

    Ice::ObjectPrx obj;
    Ice::ObjectPrx dummy;

    cout << "testing create session... " << flush;
    SessionPrx session1 = SessionPrx::uncheckedCast(manager->createLocalSession("Client1"));
    SessionPrx session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
	
    keepAlive->add(session1);
    keepAlive->add(session2);
    
    cout << "ok" << endl;

    cout << "testing allocate object... " << flush;

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

    session1->allocateObject(communicator->stringToProxy("allocatable"));
    try
    {
	session1->allocateObject(communicator->stringToProxy("allocatable"));
	test(false);
    }
    catch(const AllocationException& ex)
    {
    }
    session2->setAllocationTimeout(0);
    try
    {
	session2->allocateObject(communicator->stringToProxy("allocatable"));
	test(false);
    }
    catch(const AllocationTimeoutException& ex)
    {
    }
    try
    {
	session2->releaseObject(communicator->stringToProxy("allocatable"));
	test(false);
    }
    catch(const AllocationException& ex)
    {
    }

    session2->setAllocationTimeout(-1);
    AllocateObjectCallbackPtr cb1 = new AllocateObjectCallback();
    session2->allocateObject_async(cb1, communicator->stringToProxy("allocatable"));
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb1->hasResponse(dummy));
    session1->releaseObject(communicator->stringToProxy("allocatable"));
    cb1->wait();
    test(cb1->hasResponse(dummy));

    session1->setAllocationTimeout(0);
    try
    {
	session1->allocateObject(communicator->stringToProxy("allocatable"));
	test(false);
    }
    catch(const AllocationTimeoutException& ex)
    {
    }
    try
    {
	session1->releaseObject(communicator->stringToProxy("allocatable"));
	test(false);
    }
    catch(const AllocationException& ex)
    {
    }

    session1->setAllocationTimeout(-1);
    cb1 = new AllocateObjectCallback();
    session1->allocateObject_async(cb1, communicator->stringToProxy("allocatable"));
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb1->hasResponse(dummy));
    session2->releaseObject(communicator->stringToProxy("allocatable"));
    cb1->wait();
    test(cb1->hasResponse(dummy));

    session1->releaseObject(communicator->stringToProxy("allocatable"));

    //
    // TODO: XXX test replicated proxy
    //

    cout << "ok" << endl;

    cout << "testing allocation with findObjectById... " << flush;
    
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

    session1->setAllocationTimeout(-1);
    FindObjectByIdCallbackPtr cb2 = new FindObjectByIdCallback();
    query1->findObjectById_async(cb2, Ice::stringToIdentity("allocatable"));
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb2->hasResponse(dummy));
    session2->releaseObject(communicator->stringToProxy("allocatable"));
    cb2->wait();
    test(cb2->hasResponse(dummy));

    session1->releaseObject(communicator->stringToProxy("allocatable"));

    cout << "ok" << endl;

    cout << "testing allocation with findObjectByType... " << flush;
    
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

    session1->setAllocationTimeout(-1);
    FindObjectByTypeCallbackPtr cb3 = new FindObjectByTypeCallback();
    query1->findObjectByType_async(cb3, "::Test");
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb3->hasResponse(dummy));
    session2->releaseObject(obj);
    cb3->wait();
    test(cb3->hasResponse(obj));

    session1->releaseObject(obj);
    
    cout << "ok" << endl;

    cout << "testing allocation with findObjectByTypeOnLeastLoadedNode... " << flush;
    
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

    session1->setAllocationTimeout(-1);
    FindObjectByTypeOnLeastLoadedNodeCallbackPtr cb4 = new FindObjectByTypeOnLeastLoadedNodeCallback();
    query1->findObjectByTypeOnLeastLoadedNode_async(cb4, "::Test", LoadSample1);
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb4->hasResponse(dummy));
    session2->releaseObject(obj);
    cb4->wait();
    test(cb4->hasResponse(obj));

    session1->releaseObject(obj);
    
    cout << "ok" << endl;

    cout << "testing allocation with Ice::Locator... " << flush;
    
    session1->setAllocationTimeout(0);
    session2->setAllocationTimeout(0);

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

    cout << "testing allocation timeout... " << flush;    

    session1->allocateObject(communicator->stringToProxy("allocatable"));
    IceUtil::Time time = IceUtil::Time::now();
    session2->setAllocationTimeout(500);
    try
    {
	session2->allocateObject(communicator->stringToProxy("allocatable"));
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

    session1->releaseObject(communicator->stringToProxy("allocatable"));
    session2->setAllocationTimeout(0);

    cout << "ok" << endl;

    cout << "testing session destroy... " << flush;

    obj = query2->findObjectByType("::Test"); // Allocate the object
    test(obj && obj->ice_getIdentity().name == "allocatable");

    session1->setAllocationTimeout(-1);
    cb3 = new FindObjectByTypeCallback();
    query1->findObjectByType_async(cb3, "::Test");
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb3->hasResponse(dummy));
    session2->destroy();
    cb3->wait();
    test(cb3->hasResponse(obj));
    session1->destroy();

    session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
    session2->setAllocationTimeout(0);
    session2->allocateObject(communicator->stringToProxy("allocatable"));
    session2->destroy();

    cout << "ok" << endl;
}
