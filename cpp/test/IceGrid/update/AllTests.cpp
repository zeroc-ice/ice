// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Thread.h>
#include <Ice/Ice.h>
#include <IceGrid/Observer.h>
#include <TestCommon.h>
#include <Test.h>

using namespace std;
using namespace Test;
using namespace IceGrid;

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
		    catch(const Ice::Exception& ex)
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

class RegistryObserverI : public RegistryObserver, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RegistryObserverI() : _updated(false)
    {
    }

    virtual void 
    init(int serial, const ApplicationDescriptorSeq& apps, const Ice::StringSeq& nodes, const Ice::Current&)
    {
	Lock sync(*this);
	for(ApplicationDescriptorSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
	{
	    this->applications.insert(make_pair((*p)->name, *p));
	}
	this->nodes = nodes;
	updated(serial);
    }

    virtual void
    applicationAdded(int serial, const ApplicationDescriptorPtr& app, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications.insert(make_pair(app->name, app));
	updated(serial);
    }

    virtual void 
    applicationRemoved(int serial, const std::string& name, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications.erase(name);
	updated(serial);
    }

    virtual void 
    applicationUpdated(int serial, const ApplicationUpdateDescriptor& desc, const Ice::Current&)
    {
	Lock sync(*this);
	for(map<string, string>::const_iterator p = desc.variables.begin(); p != desc.variables.end(); ++p)
	{
	    this->applications[desc.name]->variables[p->first] = p->second;
	}
	updated(serial);
    }

    virtual void 
    applicationSynced(int serial, const ApplicationDescriptorPtr& app, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications[app->name] = app;
	updated(serial);
    }

    virtual void
    nodeUp(const string& name, const Ice::Current& current)
    {
    }

    virtual void
    nodeDown(const string& name, const Ice::Current& current)
    {
    }

    void
    waitForUpdate(const char* file, int line)
    {
	Lock sync(*this);
	while(!_updated)
	{
	    if(!timedWait(IceUtil::Time::seconds(10)))
	    {
		cerr << "wait timed out " << file << ":" << line << endl;
		test(false); // Timeout
	    }
	}
	_updated = false;
    }

    int serial;
    map<string, ApplicationDescriptorPtr> applications;
    Ice::StringSeq nodes;

private:

    void
    updated(int serial)
    {
	this->serial = serial;
	_updated = true;
	notifyAll();
    }

    bool _updated;
};

class NodeObserverI : public NodeObserver
{
public:

    virtual void 
    init(const NodeDynamicInfoSeq& info, const Ice::Current&)
    {
    }

    virtual void 
    initNode(const NodeDynamicInfo& info, const Ice::Current&)
    {
    }

    virtual void
    updateServer(const string& node, const ServerDynamicInfo& info, const Ice::Current&)
    {
    }

    virtual void
    updateAdapter(const string& node, const AdapterDynamicInfo& info, const Ice::Current& current)
    {
    }
};

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    SessionManagerPrx manager = SessionManagerPrx::checkedCast(communicator->stringToProxy("IceGrid/SessionManager"));
    test(manager);

    SessionKeepAliveThreadPtr keepAlive;
    keepAlive = new SessionKeepAliveThread(communicator->getLogger(), IceUtil::Time::seconds(5));
    keepAlive->start();

    {
	cout << "testing sessions... " << flush;
	SessionPrx session1 = manager->createLocalSession("Observer1");
	SessionPrx session2 = manager->createLocalSession("Observer2");
	
	keepAlive->add(session1);
	keepAlive->add(session2);	
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1");
	RegistryObserverI* regObs1 = new RegistryObserverI();
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverI* nodeObs1 = new NodeObserverI();
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	manager->ice_connection()->setAdapter(adpt1);	
	session1->setObserversByIdentity(ro1->ice_getIdentity(), no1->ice_getIdentity());
	
	Ice::ObjectAdapterPtr adpt2 = communicator->createObjectAdapterWithEndpoints("Observer2", "default");
	RegistryObserverI* regObs2 = new RegistryObserverI();
 	Ice::ObjectPrx ro2 = adpt2->addWithUUID(regObs2);
 	NodeObserverI* nodeObs2 = new NodeObserverI();
 	Ice::ObjectPrx no2 = adpt2->addWithUUID(nodeObs2);
 	adpt2->activate();
 	session2->setObservers(RegistryObserverPrx::uncheckedCast(ro2), NodeObserverPrx::uncheckedCast(no2));
	
	regObs1->waitForUpdate(__FILE__, __LINE__);
	regObs2->waitForUpdate(__FILE__, __LINE__);

	int serial = regObs1->serial;
	test(serial == regObs2->serial);

	try
	{
	    session1->startUpdate(serial + 1);
	    test(false);
	}
	catch(const CacheOutOfDate&)
	{
	}
	catch(const AccessDenied&)
	{
	    test(false);
	}

	try
	{
	    session1->startUpdate(serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session2->startUpdate(regObs2->serial);
	    test(false);
	}
	catch(const AccessDenied& ex)
	{
	    test(ex.lockUserId == "Observer1");
	}

	try
	{
	    session1->finishUpdate();   
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session2->startUpdate(regObs2->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    ApplicationDescriptorPtr app = new ApplicationDescriptor();
	    app->name = "Application";
	    session2->addApplication(app);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session1->addApplication(new ApplicationDescriptor());
	    test(false);
	}
	catch(const AccessDenied& ex)
	{
	}

	try
	{
	    session2->finishUpdate();   
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	regObs1->waitForUpdate(__FILE__, __LINE__);
	regObs2->waitForUpdate(__FILE__, __LINE__);

	test(serial + 1 == regObs1->serial);
	test(serial + 1 == regObs2->serial);
	++serial;

	try
	{
	    session1->startUpdate(serial);
	    ApplicationUpdateDescriptor update;
	    update.name = "Application";
	    update.variables.insert(make_pair("test", "test"));
	    session1->updateApplication(update);
	    session1->finishUpdate();
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	regObs1->waitForUpdate(__FILE__, __LINE__);
	regObs2->waitForUpdate(__FILE__, __LINE__);

	test(serial + 1 == regObs1->serial);
	test(serial + 1 == regObs2->serial);
	++serial;

	try
	{
	    ApplicationUpdateDescriptor update;
	    update.name = "Application";
	    session1->updateApplication(update);
	    test(false);
	}
	catch(const AccessDenied&)
	{
	}

	try
	{
	    session2->startUpdate(serial);
	    session2->removeApplication("Application");
	    session2->finishUpdate();
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	regObs1->waitForUpdate(__FILE__, __LINE__);
	regObs2->waitForUpdate(__FILE__, __LINE__);

	test(serial + 1 == regObs1->serial);
	test(serial + 1 == regObs2->serial);
	++serial;
	
	try
	{
	    session1->startUpdate(serial);
	}
	catch(const Ice::UserException& ex)
	{
	    test(false);
	}
	session1->destroy();

	try
	{
	    session2->startUpdate(serial);
	    session2->finishUpdate();
	}
	catch(const Ice::UserException& ex)
	{
	    test(false);
	}
	session2->destroy();

	adpt1->deactivate();
	adpt2->deactivate();

	adpt1->waitForDeactivate();
	adpt2->waitForDeactivate();

	//
	// TODO: test session reaping?
	//

	cout << "ok" << endl;
    }

    {
	cout << "testing observers... " << flush;
	SessionPrx session1 = manager->createLocalSession("Observer1");
	
	keepAlive->add(session1);
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1.1");
	RegistryObserverI* regObs1 = new RegistryObserverI();
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverI* nodeObs1 = new NodeObserverI();
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	manager->ice_connection()->setAdapter(adpt1);	
	session1->setObserversByIdentity(ro1->ice_getIdentity(), no1->ice_getIdentity());
	
	regObs1->waitForUpdate(__FILE__, __LINE__);

	int serial = regObs1->serial;
	test(find(regObs1->nodes.begin(), regObs1->nodes.end(), "localnode") != regObs1->nodes.end());
	test(regObs1->applications.empty());	    

	try
	{
	    ApplicationDescriptorPtr app = new ApplicationDescriptor();
	    app->name = "Application";
	    session1->startUpdate(serial);
	    session1->addApplication(app);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications["Application"]);
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    ApplicationUpdateDescriptor update;
	    update.name = "Application";
	    update.variables.insert(make_pair("test", "test"));
	    session1->updateApplication(update);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications["Application"]);
	    test(regObs1->applications["Application"]->variables["test"] == "test");
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    ApplicationDescriptorPtr app; 
	    app = ApplicationDescriptorPtr::dynamicCast(regObs1->applications["Application"]->ice_clone());
	    app->variables.clear();
	    app->variables["test1"] = "test";
	    session1->syncApplication(app);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications["Application"]);
	    test(regObs1->applications["Application"]->variables.size() == 1);
	    test(regObs1->applications["Application"]->variables["test1"] == "test");
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session1->removeApplication("Application");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.empty());
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	session1->destroy();
	adpt1->deactivate();
	adpt1->waitForDeactivate();

	cout << "ok" << endl;
    }

    keepAlive->terminate();
    keepAlive->getThreadControl().join();
    keepAlive = 0;
}
