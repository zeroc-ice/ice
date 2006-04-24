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

class AllocateObjectCallback : public AMI_Session_allocateObject, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    AllocateObjectCallback() : _response(false)
    {
    }

    void
    ice_response()
    {
	Lock sync(*this);
	_response = true;
	notify();
    }

    void
    ice_exception(const Ice::Exception&)
    {
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
    response()
    {
	Lock sync(*this);
	return _response;
    }

private:

    bool _response;
};
typedef IceUtil::Handle<AllocateObjectCallback> AllocateObjectCallbackPtr;

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

    cout << "testing create session... " << flush;
    SessionPrx session1 = SessionPrx::uncheckedCast(manager->createLocalSession("Client1"));
    SessionPrx session2 = SessionPrx::uncheckedCast(manager->createLocalSession("Client2"));
	
    keepAlive->add(session1);
    keepAlive->add(session2);
    
    cout << "ok" << endl;

    cout << "testing allocate object... " << flush;
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
    AllocateObjectCallbackPtr cb = new AllocateObjectCallback();
    session2->allocateObject_async(cb, communicator->stringToProxy("allocatable"));
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb->response());
    session1->releaseObject(communicator->stringToProxy("allocatable"));
    cb->wait();
    test(cb->response());

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
    cb = new AllocateObjectCallback();
    session1->allocateObject_async(cb, communicator->stringToProxy("allocatable"));
    IceUtil::ThreadControl::sleep(IceUtil::Time::milliSeconds(500));
    test(!cb->response());
    session2->releaseObject(communicator->stringToProxy("allocatable"));
    cb->wait();
    test(cb->response());

    session1->releaseObject(communicator->stringToProxy("allocatable"));
    //
    // TODO: XXX test replicated proxy
    //

    cout << "ok" << endl;

    cout << "testing allocation with query interface... " << flush;
    
    IceGrid::QueryPrx query1 = session1->getQuery();
    IceGrid::QueryPrx query2 = session2->getQuery();
    
    session1->setAllocationTimeout(0);
    session2->setAllocationTimeout(0);

    Ice::ObjectPrx obj;

    obj = query1->findObjectById(Ice::stringToIdentity("nonallocatable"));
    test(obj);
    obj = query2->findObjectById(Ice::stringToIdentity("nonallocatable"));
    test(obj);

    obj = query1->findObjectById(Ice::stringToIdentity("allocatable")); // Allocate the object
    test(obj);
    obj = query2->findObjectById(Ice::stringToIdentity("allocatable")); 
    test(!obj); // Object already allocated

    obj = query1->findObjectById(Ice::stringToIdentity("allocatable")); // Get the allocated object one more time
    test(obj);
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
    test(obj);
    obj = query1->findObjectById(Ice::stringToIdentity("allocatable")); 
    test(!obj); // Object already allocated
    obj = query2->findObjectById(Ice::stringToIdentity("allocatable"));
    session2->releaseObject(obj);

    //
    // TODO: XXX test other methods.
    //

    cout << "ok" << endl;
}
