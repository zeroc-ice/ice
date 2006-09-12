// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/Topics.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

template<class T>
class InitCB : public T
{
public:

    InitCB(const ObserverTopicPtr& topic, const Ice::ObjectPrx& observer, const string& name, int serial) : 
	_topic(topic),
	_observer(observer),
	_name(name),
	_serial(serial)
    {	
    }

    void
    ice_response()
    {
	_topic->subscribe(_observer, _serial);
    }

    void
    ice_exception(const Ice::Exception& ex)
    {
	Ice::Warning out(_observer->ice_getCommunicator()->getLogger());
	out << "couldn't initialize " << _name << " observer:\n" << ex;    
    }

private:

    const ObserverTopicPtr _topic;
    const Ice::ObjectPrx _observer;
    const string _name;
    const int _serial;
};

};

ObserverTopic::ObserverTopic(const IceStorm::TopicManagerPrx& topicManager, const string& name) :
    _serial(0)
{
    IceStorm::TopicPrx t;
    try
    {
	t = topicManager->create(name);
    }
    catch(const IceStorm::TopicExists&)
    {
	t = topicManager->retrieve(name);
    }

    //
    // NOTE: collocation optimization needs to be turned on for the
    // topic because the subscribe() method is given a fixed proxy
    // which can't be marshalled.
    //
    _topic = IceStorm::TopicPrx::uncheckedCast(t->ice_collocationOptimized(true));
    _basePublisher = _topic->getPublisher()->ice_collocationOptimized(false);
}

ObserverTopic::~ObserverTopic()
{
}

void
ObserverTopic::subscribe(const Ice::ObjectPrx& obsv, int serial)
{
    while(true)
    {
	if(serial == -1)
	{
	    initObserver(obsv);
	    return;
	}

	Lock sync(*this);
	if(serial != _serial)
	{
	    serial = -1;
	    continue;
	}

	subscribeImpl(obsv);
	break;
    }
}

void
ObserverTopic::subscribeAndWaitForSubscription(const Ice::ObjectPrx& obsv)
{
    {
	Lock sync(*this);
	_waitForSubscribe.insert(obsv->ice_getIdentity());
    }
    subscribe(obsv);
    {
	Lock sync(*this);
	while(_topic && _waitForSubscribe.find(obsv->ice_getIdentity()) != _waitForSubscribe.end())
	{
	    wait();
	}
    }
}
 
void
ObserverTopic::unsubscribe(const Ice::ObjectPrx& observer)
{
    Lock sync(*this);
    if(_topic)
    {
	_topic->unsubscribe(observer);
    }
}

void
ObserverTopic::destroy()
{
    Lock sync(*this);
    _topic = 0;
    notifyAll();
}

void 
ObserverTopic::subscribeImpl(const Ice::ObjectPrx& observer)
{
    // This must be called with the mutex locked.
    if(!_topic)
    {
	return;
    }

    IceStorm::QoS qos;
    qos["reliability"] = "twoway ordered";
    _topic->subscribe(qos, observer);

    _waitForSubscribe.erase(observer->ice_getIdentity());
    notifyAll();
}

void
ObserverTopic::updateSerial(int serial)
{
    //
    // This loop ensures that updates from the database are processed
    // sequentially.
    //
    assert(_serial < serial);
    while(_serial + 1 != serial)
    {
	wait();
    }
    _serial = serial;
    notifyAll();
}

Ice::Context
ObserverTopic::getContext(const string& name, int serial) const
{
    ostringstream os;
    os << serial;

    Ice::Context context;
    context[name] = os.str();
    return context;
}

RegistryObserverTopic::RegistryObserverTopic(const IceStorm::TopicManagerPrx& topicManager) : 
    ObserverTopic(topicManager, "RegistryObserver")
{
    const_cast<RegistryObserverPrx&>(_publisher) = RegistryObserverPrx::uncheckedCast(_basePublisher);
}

void
RegistryObserverTopic::registryUp(const RegistryInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(_serial + 1);
    _registries.insert(make_pair(info.name, info));
    try
    {
	_publisher->registryUp(info);
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `registryUp' update:\n" << ex;    
    }
}

void 
RegistryObserverTopic::registryDown(const string& name)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(_serial + 1);
    if(_registries.find(name) != _registries.end())
    {
	_registries.erase(name);
	try
	{
	    _publisher->registryDown(name);
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	    out << "unexpected exception while publishing `registryDown' update:\n" << ex;    
	}
    }
}

void
RegistryObserverTopic::initObserver(const Ice::ObjectPrx& obsv)
{
    RegistryObserverPrx observer = RegistryObserverPrx::uncheckedCast(obsv);
    RegistryInfoSeq registries;
    int serial;
    {
	Lock sync(*this);
	registries.reserve(_registries.size());
	for(map<string, RegistryInfo>::const_iterator p = _registries.begin(); p != _registries.end(); ++p)
	{
	    registries.push_back(p->second);
	}
	serial = _serial;
    }
    observer->registryInit_async(new InitCB<AMI_RegistryObserver_registryInit>(this, observer, "registry", serial),
				 registries);
}

NodeObserverTopic::NodeObserverTopic(const IceStorm::TopicManagerPrx& topicManager, 
				     const Ice::ObjectAdapterPtr& adapter) : 
    ObserverTopic(topicManager, "NodeObserver")
{
    const_cast<NodeObserverPrx&>(_publisher) = NodeObserverPrx::uncheckedCast(_basePublisher);
    try
    {
	const_cast<NodeObserverPrx&>(_externalPublisher) = NodeObserverPrx::uncheckedCast(adapter->addWithUUID(this));
    }
    catch(const Ice::LocalException&)
    {
    }
}

void
NodeObserverTopic::nodeInit(const NodeDynamicInfoSeq&, const Ice::Current&)
{
    assert(false);
}

void
NodeObserverTopic::nodeUp(const NodeDynamicInfo& info, const Ice::Current& current)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(_serial + 1);
    _nodes.insert(make_pair(info.info.name, info));
    try
    {
	_publisher->nodeUp(info);
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing 'nodeUp' update:\n" << ex;    
    }
}

void 
NodeObserverTopic::nodeDown(const string& name, const Ice::Current&)
{
    assert(false);
}

void 
NodeObserverTopic::updateServer(const string& node, const ServerDynamicInfo& server, const Ice::Current&)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }

    if(_nodes.find(node) == _nodes.end())
    {
	//
	// If the node isn't known anymore, we ignore the update.
	//
	return;
    }
    
    updateSerial(_serial + 1);

    ServerDynamicInfoSeq& servers = _nodes[node].servers;
    ServerDynamicInfoSeq::iterator p = servers.begin();
    while(p != servers.end())
    {
	if(p->id == server.id)
	{
	    if(server.state == Destroyed || server.state == Inactive)
	    {
		servers.erase(p);
	    }
	    else
	    {
		*p = server;
	    }
	    break;
	}
	++p;
    }
    if(server.state != Destroyed && server.state != Inactive && p == servers.end())
    {
	servers.push_back(server);
    }

    try
    {
	_publisher->updateServer(node, server);
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `updateServer' update:\n" << ex;    
    }
}

void 
NodeObserverTopic::updateAdapter(const string& node, const AdapterDynamicInfo& adapter, const Ice::Current&)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }

    if(_nodes.find(node) == _nodes.end())
    {
	//
	// If the node isn't known anymore, we ignore the update.
	//
	return;
    }

    updateSerial(_serial + 1);

    AdapterDynamicInfoSeq& adapters = _nodes[node].adapters;
    AdapterDynamicInfoSeq::iterator p = adapters.begin();
    while(p != adapters.end())
    {
	if(p->id == adapter.id)
	{
	    if(adapter.proxy)
	    {
		*p = adapter;
	    }
	    else
	    {
		adapters.erase(p);
	    }
	    break;
	}
	++p;
    }
    if(adapter.proxy && p == adapters.end())
    {
	adapters.push_back(adapter);
    }
    
    try
    {
	_publisher->updateAdapter(node, adapter);
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `updateAdapter' update:\n" << ex;    
    }
}

void 
NodeObserverTopic::nodeDown(const string& name)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }

    updateSerial(_serial + 1);

    if(_nodes.find(name) != _nodes.end())
    {
	_nodes.erase(name);
	try
	{
	    _publisher->nodeDown(name);
	}
	catch(const Ice::LocalException& ex)
	{
	    Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	    out << "unexpected exception while publishing `nodeDown' update:\n" << ex;    
	}
    }
}

void
NodeObserverTopic::initObserver(const Ice::ObjectPrx& obsv)
{
    NodeObserverPrx observer = NodeObserverPrx::uncheckedCast(obsv);
    int serial;
    NodeDynamicInfoSeq nodes;
    {
	Lock sync(*this);
	nodes.reserve(_nodes.size());
	for(map<string, NodeDynamicInfo>::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
	{
	    nodes.push_back(p->second);
	}
	serial = _serial;
    }
    observer->nodeInit_async(new InitCB<AMI_NodeObserver_nodeInit>(this, observer, "node", serial), nodes);
}

ApplicationObserverTopic::ApplicationObserverTopic(const IceStorm::TopicManagerPrx& topicManager,
						   const StringApplicationInfoDict& applications) :
    ObserverTopic(topicManager, "ApplicationObserver"),
    _applications(applications.begin(), applications.end())
{
    const_cast<ApplicationObserverPrx&>(_publisher) = ApplicationObserverPrx::uncheckedCast(_basePublisher);
}

void 
ApplicationObserverTopic::applicationInit(int serial, const ApplicationInfoSeq& apps)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _applications.clear();
    for(ApplicationInfoSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
    {
	_applications.insert(make_pair(p->descriptor.name, *p));
    }
    try
    {
	_publisher->applicationInit(serial, apps, getContext("application", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `applicationInit' update:\n" << ex;    
    }
}

void 
ApplicationObserverTopic::applicationAdded(int serial, const ApplicationInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _applications.insert(make_pair(info.descriptor.name, info));
    try
    {
	_publisher->applicationAdded(serial, info, getContext("application", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `applicationAdded' update:\n" << ex;    
    }
}

void 
ApplicationObserverTopic::applicationRemoved(int serial, const string& name)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _applications.erase(name);
    try
    {
	_publisher->applicationRemoved(serial, name, getContext("application", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `applicationRemoved' update:\n" << ex;    
    }
}

void 
ApplicationObserverTopic::applicationUpdated(int serial, const ApplicationUpdateInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }

    updateSerial(serial);
    try
    {
	map<string, ApplicationInfo>::iterator p = _applications.find(info.descriptor.name);
	if(p != _applications.end())
	{
	    ApplicationHelper helper(_publisher->ice_getCommunicator(), p->second.descriptor);
	    p->second.descriptor = helper.update(info.descriptor);
	    p->second.updateTime = info.updateTime;
	    p->second.updateUser = info.updateUser;
	    p->second.revision = info.revision;
	}
    }
    catch(const DeploymentException& ex)
    {
	cerr << ex.reason << endl;
	assert(false);
    }
    catch(const std::string& msg)
    {
	cerr << msg << endl;
	assert(false);
    }
    catch(const char* msg)
    {
	cerr << msg << endl;
	assert(false);
    }
    catch(...)
    {
	assert(false);
    }
    try
    {
	_publisher->applicationUpdated(serial, info, getContext("application", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `applicationUpdated' update:\n" << ex;    
    }
}

void 
ApplicationObserverTopic::initObserver(const Ice::ObjectPrx& obsv)
{
    ApplicationObserverPrx observer = ApplicationObserverPrx::uncheckedCast(obsv);
    int serial;
    ApplicationInfoSeq applications;
    {
	Lock sync(*this);
	serial = _serial;
	assert(serial != -1);
	for(map<string, ApplicationInfo>::const_iterator p = _applications.begin(); p != _applications.end(); ++p)
	{
	    applications.push_back(p->second);
	}
    }
    observer->applicationInit_async(new InitCB<AMI_ApplicationObserver_applicationInit>(this, observer, "application",
											serial), 
				    serial, applications);
}

AdapterObserverTopic::AdapterObserverTopic(const IceStorm::TopicManagerPrx& topicManager,
					   const StringAdapterInfoDict& adapters) :
    ObserverTopic(topicManager, "AdapterObserver"),
    _adapters(adapters.begin(), adapters.end())
{
    const_cast<AdapterObserverPrx&>(_publisher) = AdapterObserverPrx::uncheckedCast(_basePublisher);
}

void 
AdapterObserverTopic::adapterInit(int serial, const AdapterInfoSeq& adpts)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _adapters.clear();
    for(AdapterInfoSeq::const_iterator q = adpts.begin(); q != adpts.end(); ++q)
    {
	_adapters.insert(make_pair(q->id, *q));
    }
    try
    {
	_publisher->adapterInit(adpts, getContext("adapter", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `adapterInit' update:\n" << ex;    
    }
}

void 
AdapterObserverTopic::adapterAdded(int serial, const AdapterInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _adapters.insert(make_pair(info.id, info));
    try
    {
	_publisher->adapterAdded(info, getContext("adapter", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `adapterAdded' update:\n" << ex;    
    }
}

void 
AdapterObserverTopic::adapterUpdated(int serial, const AdapterInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _adapters[info.id] = info;
    try
    {
	_publisher->adapterUpdated(info, getContext("adapter", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `adapterUpdated' update:\n" << ex;    
    }
}

void
AdapterObserverTopic::adapterRemoved(int serial, const string& id)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _adapters.erase(id);
    try
    {
	_publisher->adapterRemoved(id, getContext("adapter", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `adapterRemoved' update:\n" << ex;    
    }
}

void 
AdapterObserverTopic::initObserver(const Ice::ObjectPrx& obsv)
{
    AdapterObserverPrx observer = AdapterObserverPrx::uncheckedCast(obsv);
    int serial;
    AdapterInfoSeq adapters;
    {
	Lock sync(*this);
	serial = _serial;
	assert(serial != -1);
	for(map<string, AdapterInfo>::const_iterator p = _adapters.begin(); p != _adapters.end(); ++p)
	{
	    adapters.push_back(p->second);
	}	
    }	    
    observer->adapterInit_async(new InitCB<AMI_AdapterObserver_adapterInit>(this, observer, "adapter", serial), 
				adapters);
}

ObjectObserverTopic::ObjectObserverTopic(const IceStorm::TopicManagerPrx& topicManager,
					 const IdentityObjectInfoDict& objects) :
    ObserverTopic(topicManager, "ObjectObserver"),
    _objects(objects.begin(), objects.end())
{
    const_cast<ObjectObserverPrx&>(_publisher) = ObjectObserverPrx::uncheckedCast(_basePublisher);
}

void 
ObjectObserverTopic::objectInit(int serial, const ObjectInfoSeq& objects)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _objects.clear();
    for(ObjectInfoSeq::const_iterator r = objects.begin(); r != objects.end(); ++r)
    {
	_objects.insert(make_pair(r->proxy->ice_getIdentity(), *r));
    }
    try
    {
	_publisher->objectInit(objects, getContext("object", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `objectInit' update:\n" << ex;    
    }
}

void 
ObjectObserverTopic::objectAdded(int serial, const ObjectInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));
    try
    {
	_publisher->objectAdded(info, getContext("object", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `objectAdded' update:\n" << ex;    
    }
}

void 
ObjectObserverTopic::objectUpdated(int serial, const ObjectInfo& info)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _objects[info.proxy->ice_getIdentity()] = info;
    try
    {
	_publisher->objectUpdated(info, getContext("object", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `objectUpdated' update:\n" << ex;    
    }
}

void
ObjectObserverTopic::objectRemoved(int serial, const Ice::Identity& id)
{
    Lock sync(*this);
    if(!_topic)
    {
	return;
    }
    updateSerial(serial);
    _objects.erase(id);
    try
    {
	_publisher->objectRemoved(id, getContext("object", serial));
    }
    catch(const Ice::LocalException& ex)
    {
	Ice::Warning out(_publisher->ice_getCommunicator()->getLogger());
	out << "unexpected exception while publishing `objectRemoved' update:\n" << ex;    
    }
}

void 
ObjectObserverTopic::initObserver(const Ice::ObjectPrx& obsv)
{
    ObjectObserverPrx observer = ObjectObserverPrx::uncheckedCast(obsv);
    int serial;
    ObjectInfoSeq objects;
    {
	Lock sync(*this);
	serial = _serial;
	for(map<Ice::Identity, ObjectInfo>::const_iterator p = _objects.begin(); p != _objects.end(); ++p)
	{
	    objects.push_back(p->second);
	}
    }	    
    observer->objectInit_async(new InitCB<AMI_ObjectObserver_objectInit>(this, observer, "object", serial), objects);
}


