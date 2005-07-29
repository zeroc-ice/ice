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
#include <IceGrid/Admin.h>
#include <TestCommon.h>

using namespace std;
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
    init(int serial, const ApplicationDescriptorSeq& apps, const Ice::Current&)
    {
	Lock sync(*this);
	for(ApplicationDescriptorSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
	{
	    this->applications.insert(make_pair((*p).name, *p));
	}
	updated(serial);
    }

    virtual void
    applicationAdded(int serial, const ApplicationDescriptor& app, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications.insert(make_pair(app.name, app));
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
	for(Ice::StringSeq::const_iterator q = desc.removeVariables.begin(); q != desc.removeVariables.end(); ++q)
	{
	    this->applications[desc.name].variables.erase(*q);
	}
	for(map<string, string>::const_iterator p = desc.variables.begin(); p != desc.variables.end(); ++p)
	{
	    this->applications[desc.name].variables[p->first] = p->second;
	}
	updated(serial);
    }

    void
    waitForUpdate(const char* file, int line)
    {
	Lock sync(*this);
	while(!_updated)
	{
	    if(!timedWait(IceUtil::Time::seconds(10)))
	    {
		cerr << "timeout: " << file << ":" << line << endl;
		test(false); // Timeout
	    }
	}
	_updated = false;
    }

    int serial;
    map<string, ApplicationDescriptor> applications;

private:

    void
    updated(int serial = -1)
    {
	if(serial != -1)
	{
	    this->serial = serial;
	}
	_updated = true;
	notifyAll();
    }

    bool _updated;
};

class NodeObserverI : public NodeObserver, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeObserverI() : _updated(0)
    {
    }

    virtual void 
    init(const NodeDynamicInfoSeq& info, const Ice::Current& current)
    {
	Lock sync(*this);
	for(NodeDynamicInfoSeq::const_iterator p = info.begin(); p != info.end(); ++p)
	{
	    this->nodes[p->name] = *p;
	}
	updated(current);
    }

    virtual void
    nodeUp(const NodeDynamicInfo& info, const Ice::Current& current)
    {
	Lock sync(*this);
	this->nodes[info.name] = info;
	updated(current);
    }

    virtual void
    nodeDown(const string& name, const Ice::Current& current)
    {
	Lock sync(*this);
	this->nodes.erase(name);
	updated(current);
    }

    virtual void
    updateServer(const string& node, const ServerDynamicInfo& info, const Ice::Current& current)
    {
	Lock sync(*this);
//	cerr << node << " " << info.name << " " << info.state << " " << info.pid << endl;
	ServerDynamicInfoSeq& servers = this->nodes[node].servers;
	ServerDynamicInfoSeq::iterator p;
	for(p = servers.begin(); p != servers.end(); ++p)
	{
	    if(p->name == info.name)
	    {
		if(info.state == Destroyed)
		{
		    servers.erase(p);
		}
		else
		{
		    *p = info;
		}
		break;
	    }
	}
	if(info.state != Destroyed && p == servers.end())
	{
	    servers.push_back(info);
	}
	updated(current);
    }

    virtual void
    updateAdapter(const string& node, const AdapterDynamicInfo& info, const Ice::Current& current)
    {
	Lock sync(*this);
//  	cerr << "update adapter: " << info.id << " " << (info.proxy ? "active" : "inactive") << endl;
	AdapterDynamicInfoSeq& adapters = this->nodes[node].adapters;
	AdapterDynamicInfoSeq::iterator p;
	for(p = adapters.begin(); p != adapters.end(); ++p)
	{
	    if(p->id == info.id)
	    {
		if(info.proxy)
		{
		    *p = info;
		}
		else
		{
		    adapters.erase(p);
		}
		break;
	    }
	}
	if(info.proxy && p == adapters.end())
	{
	    adapters.push_back(info);
	}

	updated(current);
    }

    void
    waitForUpdate(const char* file, int line)
    {
	Lock sync(*this);
	while(!_updated)
	{
	    if(!timedWait(IceUtil::Time::seconds(10)))
	    {
		cerr << "timeout: " << file << ":" << line << endl;
		test(false); // Timeout
	    }
	}
	--_updated;
    }

    map<string, NodeDynamicInfo> nodes;

private:

    void
    updated(const Ice::Current& current)
    {
	++_updated;
//	cerr << "updated: " << current.operation << " " << _updated << endl;
	notifyAll();
    }

    int _updated;
};

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    SessionManagerPrx manager = SessionManagerPrx::checkedCast(communicator->stringToProxy("IceGrid/SessionManager"));
    test(manager);

    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
    test(admin);

    Ice::PropertiesPtr properties = communicator->getProperties();

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
	    ApplicationDescriptor app;
	    app.name = "Application";
	    session2->addApplication(app);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session1->addApplication(ApplicationDescriptor());
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
	cout << "testing registry observer... " << flush;
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
	test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
	test(regObs1->applications.empty());	    

	try
	{
	    ApplicationDescriptor app;
	    app.name = "Application";
	    session1->startUpdate(serial);
	    session1->addApplication(app);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.find("Application") != regObs1->applications.end());
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	try
	{
	    ApplicationUpdateDescriptor update;
	    update.name = "Application";
	    update.variables.insert(make_pair("test", "test"));
	    session1->updateApplication(update);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.find("Application") != regObs1->applications.end());
	    test(regObs1->applications["Application"].variables["test"] == "test");
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	try
	{
	    ApplicationDescriptor app; 
	    app = regObs1->applications["Application"];
	    app.variables.clear();
	    app.variables["test1"] = "test";
	    session1->syncApplication(app);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.find("Application") != regObs1->applications.end());
	    test(regObs1->applications["Application"].variables.size() == 1);
	    test(regObs1->applications["Application"].variables["test1"] == "test");
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	try
	{
	    session1->removeApplication("Application");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.empty());
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	//
	// Setup a descriptor to deploy a node on the node.
	//
	ApplicationDescriptor nodeApp;
	nodeApp.name = "NodeApp";
	ServerDescriptorPtr server = new ServerDescriptor();
	server->id = "node-1";
	server->exe = properties->getProperty("IceDir") + "/bin/icegridnode";
	AdapterDescriptor adapter;
	adapter.name = "IceGrid.Node";
	adapter.id = "IceGrid.Node.node-1";
	adapter.registerProcess = true;
	adapter.waitForActivation = false;
	server->adapters.push_back(adapter);
	PropertyDescriptor prop;
	prop.name = "IceGrid.Node.Name";
	prop.value = "node-1";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.Data";
	prop.value = properties->getProperty("TestDir") + "/db/node-1";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.Endpoints";
	prop.value = "default";
	server->properties.push_back(prop);
	NodeDescriptor node;
	node.servers.push_back(server);
	nodeApp.nodes["localnode"] = node;
	
	try
	{
	    session1->startUpdate(serial);
	    session1->addApplication(nodeApp);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.find("NodeApp") != regObs1->applications.end());
	    test(++serial == regObs1->serial);
	}
	catch(const DeploymentException& ex)
	{
	    cerr << ex.reason << endl;
	    test(false);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // init

	admin->startServer("node-1");
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
	test(nodeObs1->nodes.find("node-1") != nodeObs1->nodes.end());

	admin->stopServer("node-1");
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
	test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	try
	{
	    session1->removeApplication("NodeApp");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->applications.empty());
	    test(++serial == regObs1->serial);
	}
	catch(const DeploymentException& ex)
	{
	    cerr << ex.reason << endl;
	    test(false);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	session1->destroy();
	adpt1->deactivate();
	adpt1->waitForDeactivate();

	cout << "ok" << endl;
    }

    {
	cout << "testing node observer... " << flush;
	SessionPrx session1 = manager->createLocalSession("Observer1");
	
	keepAlive->add(session1);
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1.2");
	RegistryObserverI* regObs1 = new RegistryObserverI();
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverI* nodeObs1 = new NodeObserverI();
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	manager->ice_connection()->setAdapter(adpt1);	
	session1->setObserversByIdentity(ro1->ice_getIdentity(), no1->ice_getIdentity());
	
	regObs1->waitForUpdate(__FILE__, __LINE__);
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // init

	test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
	test(regObs1->applications.empty());

	ApplicationDescriptor nodeApp;
	nodeApp.name = "NodeApp";
	ServerDescriptorPtr server = new ServerDescriptor();
	server->id = "node-1";
	server->exe = properties->getProperty("IceDir") + "/bin/icegridnode";
	AdapterDescriptor adapter;
	adapter.name = "IceGrid.Node";
	adapter.id = "IceGrid.Node.node-1";
	adapter.registerProcess = true;
	server->adapters.push_back(adapter);
	PropertyDescriptor prop;
	prop.name = "IceGrid.Node.Name";
	prop.value = "node-1";
	server->properties.push_back(prop);
	prop.name = "IceGrid.Node.Data";
	prop.value = properties->getProperty("TestDir") + "/db/node-1";
	server->properties.push_back(prop);	
	prop.name = "IceGrid.Node.Endpoints";
	prop.value = "default";
	server->properties.push_back(prop);	
	NodeDescriptor node;
	node.servers.push_back(server);
	nodeApp.nodes["localnode"] = node;

	admin->addApplication(nodeApp);
	regObs1->waitForUpdate(__FILE__, __LINE__);

	admin->startServer("node-1");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
	test(nodeObs1->nodes.find("node-1") != nodeObs1->nodes.end());
	test(nodeObs1->nodes["localnode"].servers.size() == 1);
	test(nodeObs1->nodes["localnode"].servers[0].state == Active);
	admin->stopServer("node-1");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Deactivating)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Inactive)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
	test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);

	admin->removeApplication("NodeApp");
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)

	regObs1->waitForUpdate(__FILE__, __LINE__);
	test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	ApplicationDescriptor testApp;
	testApp.name = "TestApp";
	
	server = new ServerDescriptor();
	server->id = "Server";
	server->exe = properties->getProperty("TestDir") + "/server";
	adapter.name = "Server";
	adapter.id = "ServerAdapter";
	adapter.registerProcess = true;
	adapter.waitForActivation = true;
	server->adapters.push_back(adapter);
	prop.name = "Server.Endpoints";
	prop.value = "default";
	server->properties.push_back(prop);
	node = NodeDescriptor();
	node.servers.push_back(server);
	testApp.nodes["localnode"] = node;

	admin->addApplication(testApp);
	regObs1->waitForUpdate(__FILE__, __LINE__);

	admin->startServer("Server");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // adapterUpdate

	test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
	test(nodeObs1->nodes["localnode"].servers.size() == 1);
	test(nodeObs1->nodes["localnode"].servers[0].state == Active);
	test(nodeObs1->nodes["localnode"].adapters.size() == 1);
	test(nodeObs1->nodes["localnode"].adapters[0].proxy);

	admin->stopServer("Server");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // adapterUpdate

	test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
	test(nodeObs1->nodes["localnode"].servers.size() == 1);
	test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);
	test(nodeObs1->nodes["localnode"].adapters.empty());

	admin->removeApplication("TestApp");	

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	test(nodeObs1->nodes["localnode"].servers.empty());

	regObs1->waitForUpdate(__FILE__, __LINE__);

	cout << "ok" << endl;
    }

    keepAlive->terminate();
    keepAlive->getThreadControl().join();
    keepAlive = 0;
}
