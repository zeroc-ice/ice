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
#include <IceGrid/Registry.h>
#include <IceGrid/Query.h>
#include <IceGrid/Session.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Observer.h>
#include <Glacier2/Router.h>
#include <TestCommon.h>

using namespace std;
using namespace IceGrid;

void 
addProperty(const CommunicatorDescriptorPtr& communicator, const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    communicator->propertySet.properties.push_back(prop);
}

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
		vector<AdminSessionPrx>::iterator p = _sessions.begin();
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
    add(const AdminSessionPrx& session)
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
    vector<AdminSessionPrx> _sessions;
    const IceUtil::Time _timeout;
    bool _terminated;
};
typedef IceUtil::Handle<SessionKeepAliveThread> SessionKeepAliveThreadPtr;

class ObserverStackTracer
{
public:

    ObserverStackTracer(const string& name) : _name(name)
    {
	_observers.insert(make_pair(name, this));
    }

    virtual ~ObserverStackTracer()
    {
	_observers.erase(_name);
    }

    static void
    printStack()
    {
	map<string, ObserverStackTracer*>::const_iterator p;
	for(p = _observers.begin(); p != _observers.end(); ++p)
	{
	    vector<string>::const_iterator q = p->second->_stack.begin();
	    if(p->second->_stack.size() > 10)
	    {
		q = p->second->_stack.begin() + p->second->_stack.size() - 10;
	    }
	    cerr << "Last 10 updates of observer `" << p->second->_name << "':" << endl;
	    for(; q != p->second->_stack.end(); ++q)
	    {
		cerr << "  " << *q << endl;
	    }
	    p->second->_stack.clear();
	}
    }

    void
    trace(const string& msg)
    {
	_stack.push_back(msg);
    }

private:

    string _name;
    vector<string> _stack;
    
    static map<string, ObserverStackTracer*> _observers;
};
map<string, ObserverStackTracer*> ObserverStackTracer::_observers;

class RegistryObserverI : public RegistryObserver, public ObserverStackTracer, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    RegistryObserverI(const string& name) : ObserverStackTracer(name), _updated(0)
    {
    }
    
    virtual void 
    init(int serial, const ApplicationDescriptorSeq& apps, const AdapterInfoSeq& adapters, 
	 const ObjectInfoSeq& objects, const Ice::Current&)
    {
	Lock sync(*this);
	for(ApplicationDescriptorSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
	{
	    if(p->name != "Test") // Ignore the test application from application.xml!
	    {
		this->applications.insert(make_pair(p->name, *p));
	    }
	}
	for(AdapterInfoSeq::const_iterator q = adapters.begin(); q != adapters.end(); ++q)
	{
	    this->adapters.insert(make_pair(q->id, *q));
	}
	for(ObjectInfoSeq::const_iterator r = objects.begin(); r != objects.end(); ++r)
	{
	    this->objects.insert(make_pair(r->proxy->ice_getIdentity(), *r));
	}
	updated(serial, "init update");
    }

    virtual void
    applicationAdded(int serial, const ApplicationDescriptor& app, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications.insert(make_pair(app.name, app));
	updated(serial, "application added `" + app.name + "'");
    }

    virtual void 
    applicationRemoved(int serial, const std::string& name, const Ice::Current&)
    {
	Lock sync(*this);
	this->applications.erase(name);
	updated(serial, "application removed `" + name + "'");
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
	updated(serial, "application updated `" + desc.name + "'");
    }

    void
    adapterAdded(int serial, const AdapterInfo& info, const Ice::Current&)
    {
	Lock sync(*this);
	this->adapters.insert(make_pair(info.id, info));
	updated(serial, "adapter added `" + info.id + "'");
    }

    void
    adapterUpdated(int serial, const AdapterInfo& info, const Ice::Current&)
    {
	Lock sync(*this);
	this->adapters[info.id] = info;
	updated(serial, "adapter updated `" + info.id + "'");
    }

    void
    adapterRemoved(int serial, const string& id, const Ice::Current&)
    {
	Lock sync(*this);
	this->adapters.erase(id);
	updated(serial, "adapter removed `" + id + "'");
    }

    void
    objectAdded(int serial, const ObjectInfo& info, const Ice::Current&)
    {
	Lock sync(*this);
	this->objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
	updated(serial, "object added `" + info.proxy->ice_toString() + "'");
    }

    void
    objectUpdated(int serial, const ObjectInfo& info, const Ice::Current&)
    {
	Lock sync(*this);
	this->objects[info.proxy->ice_getIdentity()] = info;
	updated(serial, "object updated `" + info.proxy->ice_toString() + "'");
    }

    void
    objectRemoved(int serial, const Ice::Identity& id, const Ice::Current& current)
    {
	Lock sync(*this);
	this->objects.erase(id);
	updated(serial, "object removed `" + current.adapter->getCommunicator()->identityToString(id) + "'");
    }

    void
    waitForUpdate(const char* file, int line)
    {
	Lock sync(*this);

	ostringstream os;
	os << "wait for update from line " << line << " (serial = " << serial << ")";
	trace(os.str());

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

    int serial;
    map<string, ApplicationDescriptor> applications;
    map<string, AdapterInfo> adapters;
    map<Ice::Identity, ObjectInfo> objects;

private:

    void
    updated(int serial, const string& update)
    {
	ostringstream os;
	os  << update << " (serial = " << serial << ")";
	trace(os.str());

	if(serial != -1)
	{
	    this->serial = serial;
	}
	++_updated;
	notifyAll();
    }

    int _updated;
};
typedef IceUtil::Handle<RegistryObserverI> RegistryObserverIPtr;

class NodeObserverI : public NodeObserver, public ObserverStackTracer, public IceUtil::Monitor<IceUtil::Mutex>
{
public:

    NodeObserverI(const string& name) : ObserverStackTracer(name), _updated(0)
    {
    }

    virtual void 
    init(const NodeDynamicInfoSeq& info, const Ice::Current& current)
    {
	Lock sync(*this);
	for(NodeDynamicInfoSeq::const_iterator p = info.begin(); p != info.end(); ++p)
	{
	    this->nodes[p->name] = filter(*p);
	}
	updated(current, "init");
    }

    virtual void
    nodeUp(const NodeDynamicInfo& info, const Ice::Current& current)
    {
	Lock sync(*this);
	this->nodes[info.name] = filter(info);
	updated(current, "node `" + info.name + "' up");
    }

    virtual void
    nodeDown(const string& name, const Ice::Current& current)
    {
	Lock sync(*this);
	this->nodes.erase(name);
	updated(current, "node `" + name + "' down");
    }

    virtual void
    updateServer(const string& node, const ServerDynamicInfo& info, const Ice::Current& current)
    {
	if(info.id == "Glacier2" || info.id == "Glacier2Admin" || info.id == "PermissionsVerifierServer")
	{
	    return;
	}

	Lock sync(*this);
	//cerr << node << " " << info.id << " " << info.state << " " << info.pid << endl;
	ServerDynamicInfoSeq& servers = this->nodes[node].servers;
	ServerDynamicInfoSeq::iterator p;
	for(p = servers.begin(); p != servers.end(); ++p)
	{
	    if(p->id == info.id)
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

	ostringstream os;
	os << "server `" << info.id << "' on node `" << node << "' state updated: " << info.state
	   << " (pid = " << info.pid << ")";
	updated(current, os.str());
    }

    virtual void
    updateAdapter(const string& node, const AdapterDynamicInfo& info, const Ice::Current& current)
    {
	if(info.id == "PermissionsVerifierServer.Server")
	{
	    return;
	}

	Lock sync(*this);
  	//cerr << "update adapter: " << info.id << " " << (info.proxy ? "active" : "inactive") << endl;
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

	ostringstream os;
	os << "adapter `" << info.id << " on node `" << node << "' state updated: " 
	   << (info.proxy ? "active" : "inactive");
	updated(current, os.str());
    }

    NodeDynamicInfo
    filter(const NodeDynamicInfo& info)
    {
	if(info.name != "localnode")
	{
	    return info;
	}

	NodeDynamicInfo filtered;
	filtered.name = info.name;
	filtered.info = info.info;

	for(ServerDynamicInfoSeq::const_iterator p = info.servers.begin(); p != info.servers.end(); ++p)
	{
	    if(p->id == "Glacier2" || p->id == "Glacier2Admin" || p->id == "PermissionsVerifierServer")
	    {
		continue;
	    }
	    filtered.servers.push_back(*p);
	}

	for(AdapterDynamicInfoSeq::const_iterator a = info.adapters.begin(); a != info.adapters.end(); ++a)
	{
	    if(a->id == "PermissionsVerifierServer.Server")
	    {
		continue;
	    }
	    filtered.adapters.push_back(*a);
	}

	return filtered;
    }

    void
    waitForUpdate(const char* file, int line)
    {
	Lock sync(*this);

	ostringstream os;
	os << "wait for update from line " << line;
	trace(os.str());

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
    updated(const Ice::Current& current, const string& update)
    {
	trace(update);
	++_updated;
	//cerr << "updated: " << current.operation << " " << _updated << endl;
	notifyAll();
    }

    int _updated;
};
typedef IceUtil::Handle<NodeObserverI> NodeObserverIPtr;

void
testFailedAndPrintObservers(const char* expr, const char* file, unsigned int line)
{
    ObserverStackTracer::printStack();
    testFailed(expr, file, line);
}

#undef test
#define test(ex) ((ex) ? ((void)0) : testFailedAndPrintObservers(#ex, __FILE__, __LINE__))

void 
allTests(const Ice::CommunicatorPtr& communicator)
{
    RegistryPrx registry = RegistryPrx::checkedCast(communicator->stringToProxy("IceGrid/Registry"));
    test(registry);

    AdminPrx admin = AdminPrx::checkedCast(communicator->stringToProxy("IceGrid/Admin"));
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

    cout << "starting admin router... " << flush;
    try
    {
	admin->startServer("Glacier2Admin");
    }
    catch(const ServerStartException& ex)
    {
	cerr << ex.reason << endl;
	test(false);
    }
    cout << "ok" << endl;

    Ice::PropertiesPtr properties = communicator->getProperties();

    SessionKeepAliveThreadPtr keepAlive;
    keepAlive = new SessionKeepAliveThread(communicator->getLogger(), IceUtil::Time::seconds(5));
    keepAlive->start();

    IceGrid::RegistryPrx registry1 = IceGrid::RegistryPrx::uncheckedCast(registry->ice_connectionId("reg1"));
    IceGrid::RegistryPrx registry2 = IceGrid::RegistryPrx::uncheckedCast(registry->ice_connectionId("reg2"));

    Ice::ObjectPrx router = communicator->stringToProxy("Glacier2/router:default -p 12347 -h 127.0.0.1");
    Ice::ObjectPrx adminRouter = communicator->stringToProxy("Glacier2/router:default -p 12348 -h 127.0.0.1");

    Glacier2::RouterPrx router1 = Glacier2::RouterPrx::uncheckedCast(router->ice_connectionId("router1"));
    Glacier2::RouterPrx router2 = Glacier2::RouterPrx::uncheckedCast(router->ice_connectionId("router2"));

    Glacier2::RouterPrx adminRouter1 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter1"));
    Glacier2::RouterPrx adminRouter2 = Glacier2::RouterPrx::uncheckedCast(adminRouter->ice_connectionId("admRouter2"));

    { 
	cout << "testing username/password sessions... " << flush;

	SessionPrx session1, session2;

	session1 = SessionPrx::uncheckedCast(registry1->createSession("client1", "test1")->ice_connectionId("reg1"));
	session2 = SessionPrx::uncheckedCast(registry2->createSession("client2", "test2")->ice_connectionId("reg2"));
	try
	{
	    registry1->createSession("client3", "test1");
	}
	catch(const IceGrid::PermissionDeniedException&)
	{
	}

	session1->ice_ping();
	session2->ice_ping();

	try
	{
	    session1->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    session2->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    session1->ice_connectionId("reg2")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    session2->ice_connectionId("reg1")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}


	session1->destroy();
	session2->destroy();

	AdminSessionPrx adminSession1, adminSession2;

	adminSession1 = AdminSessionPrx::uncheckedCast(
	    registry1->createAdminSession("admin1", "test1")->ice_connectionId("reg1"));
	adminSession2 = AdminSessionPrx::uncheckedCast(
	    registry2->createAdminSession("admin2", "test2")->ice_connectionId("reg2"));
	try
	{
	    registry1->createAdminSession("admin3", "test1");
	}
	catch(const IceGrid::PermissionDeniedException&)
	{
	}

	adminSession1->ice_ping();
	adminSession2->ice_ping();

	try
	{
	    adminSession1->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    adminSession2->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	adminSession1->getAdmin()->ice_connectionId("reg1")->ice_ping();
	adminSession2->getAdmin()->ice_connectionId("reg2")->ice_ping();

	try
	{
	    adminSession1->getAdmin()->ice_connectionId("reg2")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    adminSession2->getAdmin()->ice_connectionId("reg1")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	adminSession1->destroy();
	adminSession2->destroy();
	
	cout << "ok" << endl;
    }

    if(properties->getProperty("Ice.Default.Protocol") == "ssl")
    { 
	cout << "testing sessions from secure connection... " << flush;

	SessionPrx session1, session2;

	session1 = SessionPrx::uncheckedCast(registry1->createSessionFromSecureConnection()->ice_connectionId("reg1"));
	session2 = SessionPrx::uncheckedCast(registry2->createSessionFromSecureConnection()->ice_connectionId("reg2"));

	session1->ice_ping();
	session2->ice_ping();

	try
	{
	    session1->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    session2->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	session1->destroy();
	session2->destroy();
	
	AdminSessionPrx adminSession1, adminSession2;

	adminSession1 = AdminSessionPrx::uncheckedCast(
	    registry1->createAdminSessionFromSecureConnection()->ice_connectionId("reg1"));
	adminSession2 = AdminSessionPrx::uncheckedCast(
	    registry2->createAdminSessionFromSecureConnection()->ice_connectionId("reg2"));

	adminSession1->ice_ping();
	adminSession2->ice_ping();

	try
	{
	    adminSession1->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    adminSession2->ice_connectionId("")->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	adminSession1->destroy();
	adminSession2->destroy();

	cout << "ok" << endl;
    }
    else
    {
	cout << "testing sessions from secure connection... " << flush;
	try
	{
	    registry1->createSessionFromSecureConnection();
	    test(false);
	}
	catch(const IceGrid::PermissionDeniedException&)
	{
	}
	try
	{
	    registry1->createAdminSessionFromSecureConnection();
	    test(false);
	}
	catch(const IceGrid::PermissionDeniedException&)
	{
	}
	cout << "ok" << endl;
    }

    {
	cout << "testing Glacier2 username/password sessions... " << flush;

	SessionPrx session1, session2;

	Glacier2::SessionPrx base;

	base = router1->createSession("client1", "test1");
	session1 = SessionPrx::uncheckedCast(base->ice_connectionId("router1")->ice_router(router1));

	base = router2->createSession("client2", "test2");
	session2 = SessionPrx::uncheckedCast(base->ice_connectionId("router2")->ice_router(router2));

	try
	{
	    router1->createSession("client3", "test1");
	}
	catch(const Glacier2::CannotCreateSessionException&)
	{
	}

	session1->ice_ping();
	session2->ice_ping();

	try
	{
	    session1->ice_connectionId("router2")->ice_router(router2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    session2->ice_connectionId("router1")->ice_router(router1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	Ice::ObjectPrx obj = communicator->stringToProxy("IceGrid/Query");
	obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
	obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();

	obj = communicator->stringToProxy("IceGrid/Registry");
	try
	{
	    obj->ice_connectionId("router1")->ice_router(router1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    obj->ice_connectionId("router2")->ice_router(router2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    router1->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}
	try
	{
	    router2->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}

	AdminSessionPrx admSession1, admSession2;

	base = adminRouter1->createSession("admin1", "test1");
	admSession1 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter1")->ice_router(adminRouter1));

	base = adminRouter2->createSession("admin2", "test2");
	admSession2 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter2")->ice_router(adminRouter2));

	try
	{
	    adminRouter1->createSession("client3", "test1");
	}
	catch(const Glacier2::CannotCreateSessionException&)
	{
	}

	admSession1->ice_ping();
	admSession2->ice_ping();

	Ice::ObjectPrx admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter1");
	Ice::ObjectPrx admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter2");

	admin1->ice_ping();
	admin2->ice_ping();

	try
	{
	    admSession1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    admSession2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    admin1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    admin2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	obj = communicator->stringToProxy("IceGrid/Query");
	try
	{
	    obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	obj = communicator->stringToProxy("IceGrid/Admin");
	try
	{
	    obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    adminRouter1->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}
	try
	{
	    adminRouter2->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}

	cout << "ok" << endl;
    }

    if(properties->getProperty("Ice.Default.Protocol") == "ssl")
    { 
	cout << "testing Glacier2 sessions from secure connection... " << flush;

	SessionPrx session1, session2;

	Glacier2::SessionPrx base;

	//
	// BUGFIX: We can't re-use the same router proxies because of bug 1034.
	//
	router1 = Glacier2::RouterPrx::uncheckedCast(router1->ice_connectionId("router11"));
	router2 = Glacier2::RouterPrx::uncheckedCast(router2->ice_connectionId("router21"));

	base = router1->createSessionFromSecureConnection();
	session1 = SessionPrx::uncheckedCast(base->ice_connectionId("router11")->ice_router(router1));

	base = router2->createSessionFromSecureConnection();
	session2 = SessionPrx::uncheckedCast(base->ice_connectionId("router21")->ice_router(router2));

	session1->ice_ping();
	session2->ice_ping();

	try
	{
	    session1->ice_connectionId("router21")->ice_router(router2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    session2->ice_connectionId("router11")->ice_router(router1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	Ice::ObjectPrx obj = communicator->stringToProxy("IceGrid/Query");
	obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
	obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();

	obj = communicator->stringToProxy("IceGrid/Registry");
	try
	{
	    obj->ice_connectionId("router11")->ice_router(router1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    obj->ice_connectionId("router21")->ice_router(router2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    router1->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}
	try
	{
	    router2->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}

	AdminSessionPrx admSession1, admSession2;

	base = adminRouter1->createSessionFromSecureConnection();
	admSession1 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter1")->ice_router(adminRouter1));

	base = adminRouter2->createSessionFromSecureConnection();
	admSession2 = AdminSessionPrx::uncheckedCast(base->ice_connectionId("admRouter2")->ice_router(adminRouter2));

	admSession1->ice_ping();
	admSession2->ice_ping();

	Ice::ObjectPrx admin1 = admSession1->getAdmin()->ice_router(adminRouter1)->ice_connectionId("admRouter1");
	Ice::ObjectPrx admin2 = admSession2->getAdmin()->ice_router(adminRouter2)->ice_connectionId("admRouter2");

	admin1->ice_ping();
	admin2->ice_ping();

	try
	{
	    admSession1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    admSession2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    admin1->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    admin2->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	obj = communicator->stringToProxy("IceGrid/Query");
	try
	{
	    obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	try
	{
	    obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}	    

	obj = communicator->stringToProxy("IceGrid/Admin");
	try
	{
	    obj->ice_connectionId("admRouter1")->ice_router(adminRouter1)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{ 
	}
	try
	{
	    obj->ice_connectionId("admRouter2")->ice_router(adminRouter2)->ice_ping();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}

	try
	{
	    adminRouter1->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}
	try
	{
	    adminRouter2->destroySession();
	}
	catch(const Ice::ConnectionLostException&)
	{
	}

	cout << "ok" << endl;
    }
    else
    {
	cout << "testing Glacier2 sessions from secure connection... " << flush;
	try
	{
	    router1->createSessionFromSecureConnection();
	    test(false);
	}
	catch(const Glacier2::PermissionDeniedException&)
	{
	}
	try
	{
	    adminRouter1->createSessionFromSecureConnection();
	    test(false);
	}
	catch(const Glacier2::PermissionDeniedException&)
	{
	}
	cout << "ok" << endl;
    }

    {
	cout << "testing updates with admin sessions... " << flush;
	AdminSessionPrx session1 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin1", "test1"));
	AdminSessionPrx session2 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin2", "test2"));
	
	keepAlive->add(session1);
	keepAlive->add(session2);	

	AdminPrx admin1 = session1->getAdmin();
	AdminPrx admin2 = session2->getAdmin();
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1");
	RegistryObserverIPtr regObs1 = new RegistryObserverI("regObs1");
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	registry->ice_getConnection()->setAdapter(adpt1);	
	session1->setObserversByIdentity(ro1->ice_getIdentity(), no1->ice_getIdentity());
	
	Ice::ObjectAdapterPtr adpt2 = communicator->createObjectAdapterWithEndpoints("Observer2", "default");
	RegistryObserverIPtr regObs2 = new RegistryObserverI("regObs2");
 	Ice::ObjectPrx ro2 = adpt2->addWithUUID(regObs2);
 	NodeObserverIPtr nodeObs2 = new NodeObserverI("nodeObs1");
 	Ice::ObjectPrx no2 = adpt2->addWithUUID(nodeObs2);
 	adpt2->activate();
 	session2->setObservers(RegistryObserverPrx::uncheckedCast(ro2), NodeObserverPrx::uncheckedCast(no2));
	
	regObs1->waitForUpdate(__FILE__, __LINE__);
	regObs2->waitForUpdate(__FILE__, __LINE__);

	int serial = regObs1->serial;
	test(serial == regObs2->serial);

	try
	{
	    session1->getAdmin()->ice_ping();
	}
	catch(const Ice::LocalException&)
	{
	    test(false);
	}

	try
	{
	    int s = session1->startUpdate();
	    test(s != serial + 1);
	}
	catch(const AccessDeniedException&)
	{
	    test(false);
	}

	try
	{
	    int s = session1->startUpdate();
	    test(s == serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    session2->startUpdate();
	    test(false);
	}
	catch(const AccessDeniedException& ex)
	{
	    test(ex.lockUserId == "admin1");
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
	    int s = session2->startUpdate();
	    test(s == regObs2->serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    ApplicationDescriptor app;
	    app.name = "Application";
	    admin2->addApplication(app);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}

	try
	{
	    admin1->addApplication(ApplicationDescriptor());
	    test(false);
	}
	catch(const AccessDeniedException&)
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
	    int s = session1->startUpdate();
	    test(s == serial);
	    ApplicationUpdateDescriptor update;
	    update.name = "Application";
	    update.variables.insert(make_pair(string("test"), string("test")));
	    admin1->updateApplication(update);
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
	    admin1->updateApplication(update);
	    test(false);
	}
	catch(const AccessDeniedException&)
	{
	}

	try
	{
	    int s = session2->startUpdate();
	    test(s == serial);
	    admin2->removeApplication("Application");
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
	    int s = session1->startUpdate();
	    test(s == serial);
	}
	catch(const Ice::UserException&)
	{
	    test(false);
	}
	session1->destroy();

	try
	{
	    int s = session2->startUpdate();
	    test(s == serial);
	    session2->finishUpdate();
	}
	catch(const Ice::UserException&)
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
	AdminSessionPrx session1 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin1", "test1"));
	AdminPrx admin1 = session1->getAdmin();

	keepAlive->add(session1);
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1.1");
	RegistryObserverIPtr regObs1 = new RegistryObserverI("regObs1");
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	registry->ice_getConnection()->setAdapter(adpt1);	
	session1->setObserversByIdentity(ro1->ice_getIdentity(), no1->ice_getIdentity());
	
	regObs1->waitForUpdate(__FILE__, __LINE__);

	int serial = regObs1->serial;
	test(regObs1->applications.empty());

	do
	{
	    nodeObs1->waitForUpdate(__FILE__, __LINE__);
	}
	while(nodeObs1->nodes.find("localnode") == nodeObs1->nodes.end());

	try
	{
	    ApplicationDescriptor app;
	    app.name = "Application";
	    int s = session1->startUpdate();
	    test(s == serial);
	    admin1->addApplication(app);
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
	    update.variables.insert(make_pair(string("test"), string("test")));
	    admin1->updateApplication(update);
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
	    admin1->syncApplication(app);
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
	    admin1->removeApplication("Application");
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
	// Test adapterAdded/adapterUpdated/adapterRemoved.
	//
	try
	{
	    Ice::ObjectPrx obj = communicator->stringToProxy("dummy:tcp -p 10000");

	    Ice::LocatorRegistryPrx locatorRegistry = communicator->getDefaultLocator()->getRegistry();
	    locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter") != regObs1->adapters.end());
	    test(regObs1->adapters["DummyAdapter"].proxy == obj);
	    test(++serial == regObs1->serial);
	    
	    obj = communicator->stringToProxy("dummy:tcp -p 10000 -h host");
	    locatorRegistry->setAdapterDirectProxy("DummyAdapter", obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter") != regObs1->adapters.end());
	    test(regObs1->adapters["DummyAdapter"].proxy == obj);
	    test(++serial == regObs1->serial);

	    obj = communicator->stringToProxy("dummy:tcp -p 10000 -h host");
	    locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter", "DummyReplicaGroup", obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter") != regObs1->adapters.end());
	    test(regObs1->adapters["DummyAdapter"].proxy == obj);
	    test(regObs1->adapters["DummyAdapter"].replicaGroupId == "DummyReplicaGroup");
	    test(++serial == regObs1->serial);

	    obj = communicator->stringToProxy("dummy:tcp -p 10000 -h host");
	    locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter1", "DummyReplicaGroup", obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter1") != regObs1->adapters.end());
	    test(regObs1->adapters["DummyAdapter1"].proxy == obj);
	    test(regObs1->adapters["DummyAdapter1"].replicaGroupId == "DummyReplicaGroup");
	    test(++serial == regObs1->serial);

	    obj = communicator->stringToProxy("dummy:tcp -p 10000 -h host");
	    locatorRegistry->setReplicatedAdapterDirectProxy("DummyAdapter2", "DummyReplicaGroup", obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter2") != regObs1->adapters.end());
	    test(regObs1->adapters["DummyAdapter2"].proxy == obj);
	    test(regObs1->adapters["DummyAdapter2"].replicaGroupId == "DummyReplicaGroup");
	    test(++serial == regObs1->serial);

	    admin->removeAdapter("DummyAdapter2");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter2") == regObs1->adapters.end());
	    test(++serial == regObs1->serial);

	    admin->removeAdapter("DummyReplicaGroup");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters["DummyAdapter"].replicaGroupId == "");
	    test(regObs1->adapters["DummyAdapter1"].replicaGroupId == "");
	    serial += 2;
	    test(serial == regObs1->serial);

	    locatorRegistry->setAdapterDirectProxy("DummyAdapter", 0);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->adapters.find("DummyAdapter") == regObs1->adapters.end());
	    test(++serial == regObs1->serial);
	}
	catch(const Ice::UserException& ex)
	{
	    cerr << ex << endl;
	    test(false);
	}

	//
	// Test objectAdded/objectUpdated/objectRemoved.
	//
	try
	{
	    Ice::ObjectPrx obj = communicator->stringToProxy("dummy:tcp -p 10000");

	    admin->addObjectWithType(obj, "::Dummy");
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->objects.find(communicator->stringToIdentity("dummy")) != regObs1->objects.end());
	    test(regObs1->objects[communicator->stringToIdentity("dummy")].type == "::Dummy");
	    test(regObs1->objects[communicator->stringToIdentity("dummy")].proxy == obj);
	    test(++serial == regObs1->serial);
	    
	    obj = communicator->stringToProxy("dummy:tcp -p 10000 -h host");
	    admin->updateObject(obj);
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->objects.find(communicator->stringToIdentity("dummy")) != regObs1->objects.end());
	    test(regObs1->objects[communicator->stringToIdentity("dummy")].type == "::Dummy");
	    test(regObs1->objects[communicator->stringToIdentity("dummy")].proxy == obj);
	    test(++serial == regObs1->serial);

	    admin->removeObject(obj->ice_getIdentity());
	    regObs1->waitForUpdate(__FILE__, __LINE__);
	    test(regObs1->objects.find(communicator->stringToIdentity("dummy")) == regObs1->objects.end());
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
	server->pwd = ".";
	AdapterDescriptor adapter;
	adapter.name = "IceGrid.Node";
	adapter.id = "IceGrid.Node.node-1";
	adapter.registerProcess = true;
	adapter.waitForActivation = false;
	server->adapters.push_back(adapter);
	addProperty(server, "IceGrid.Node.Name", "node-1");
	addProperty(server, "IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-1");
	addProperty(server, "IceGrid.Node.Endpoints", "default");
	NodeDescriptor node;
	node.servers.push_back(server);
	nodeApp.nodes["localnode"] = node;
	
	try
	{
	    int s = session1->startUpdate();
	    test(s == serial);
	    admin1->addApplication(nodeApp);
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

	try
	{
	    admin->startServer("node-1");
	}
	catch(const NodeUnreachableException& ex)
	{
	    cerr << ex << ":\n";
	    cerr << "node = " << ex.name << endl;
	    cerr << "reason = " << ex.reason << endl;
	}

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	do
	{
	    nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
	}
	while(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	try
	{
	    admin->stopServer("node-1");
	}
	catch(const NodeUnreachableException& ex)
	{
	    cerr << ex << ":\n";
	    cerr << "node = " << ex.name << endl;
	    cerr << "reason = " << ex.reason << endl;
	}
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // updateServer
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
	test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	try
	{
	    admin1->removeApplication("NodeApp");
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

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)

	session1->destroy();
	adpt1->deactivate();
	adpt1->waitForDeactivate();

	cout << "ok" << endl;
    }

    {
	cout << "testing node observer... " << flush;
	AdminSessionPrx session1 = AdminSessionPrx::uncheckedCast(registry->createAdminSession("admin1", "test1"));
	
	keepAlive->add(session1);
	
	Ice::ObjectAdapterPtr adpt1 = communicator->createObjectAdapter("Observer1.2");
	RegistryObserverIPtr regObs1 = new RegistryObserverI("regObs1");
	Ice::ObjectPrx ro1 = adpt1->addWithUUID(regObs1);
	NodeObserverIPtr nodeObs1 = new NodeObserverI("nodeObs1");
	Ice::ObjectPrx no1 = adpt1->addWithUUID(nodeObs1);
	adpt1->activate();
	registry->ice_getConnection()->setAdapter(adpt1);	
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
	server->pwd = ".";
	AdapterDescriptor adapter;
	adapter.name = "IceGrid.Node";
	adapter.id = "IceGrid.Node.node-1";
	adapter.registerProcess = true;
	adapter.waitForActivation = false;
	server->adapters.push_back(adapter);
	addProperty(server, "IceGrid.Node.Name", "node-1");
	addProperty(server, "IceGrid.Node.Data", properties->getProperty("TestDir") + "/db/node-1");
	addProperty(server, "IceGrid.Node.Endpoints", "default");
	NodeDescriptor node;
	node.servers.push_back(server);
	nodeApp.nodes["localnode"] = node;

	admin->addApplication(nodeApp);
	regObs1->waitForUpdate(__FILE__, __LINE__);

	admin->startServer("node-1");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	do
	{
 	    nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeUp
	}
	while(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	test(nodeObs1->nodes["localnode"].servers.size() == 1);
	test(nodeObs1->nodes["localnode"].servers[0].state == Active);
	admin->stopServer("node-1");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Deactivating)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Inactive)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // nodeDown
	test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);

	admin->removeApplication("NodeApp");
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)

	regObs1->waitForUpdate(__FILE__, __LINE__);
	test(nodeObs1->nodes.find("node-1") == nodeObs1->nodes.end());

	ApplicationDescriptor testApp;
	testApp.name = "TestApp";
	
	server = new ServerDescriptor();
	server->id = "Server";
	server->exe = properties->getProperty("TestDir") + "/server";
	server->pwd = ".";
	adapter.name = "Server";
	adapter.id = "ServerAdapter";
	adapter.registerProcess = true;
	adapter.waitForActivation = true;
	server->adapters.push_back(adapter);
	addProperty(server, "Server.Endpoints", "default");
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
	
	test(nodeObs1->nodes["localnode"].servers[0].enabled);
	admin->enableServer("Server", false);
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	test(!nodeObs1->nodes["localnode"].servers[0].enabled);
	admin->enableServer("Server", true);
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	test(nodeObs1->nodes["localnode"].servers[0].enabled);

	admin->stopServer("Server");

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // adapterUpdate

	test(nodeObs1->nodes.find("localnode") != nodeObs1->nodes.end());
	test(nodeObs1->nodes["localnode"].servers.size() == 1);
	test(nodeObs1->nodes["localnode"].servers[0].state == Inactive);
	test(nodeObs1->nodes["localnode"].adapters.empty());

	admin->removeApplication("TestApp");	

	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroying)
	nodeObs1->waitForUpdate(__FILE__, __LINE__); // serverUpdate(Destroyed)
	test(nodeObs1->nodes["localnode"].servers.empty());

	regObs1->waitForUpdate(__FILE__, __LINE__);

	cout << "ok" << endl;
    }

    keepAlive->terminate();
    keepAlive->getThreadControl().join();
    keepAlive = 0;

    admin->stopServer("PermissionsVerifierServer");

    cout << "shutting down admin router... " << flush;
    admin->stopServer("Glacier2Admin");
    cout << "ok" << endl;

    cout << "shutting down router... " << flush;
    admin->stopServer("Glacier2");
    cout << "ok" << endl;
}
