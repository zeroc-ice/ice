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

class RegistryInitCB : public AMI_RegistryObserver_init
{
public:

    RegistryInitCB(const RegistryObserverTopicPtr& topic, const RegistryObserverPrx& observer, int serial) : 
	_topic(topic),
	_observer(observer),
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
	out << "couldn't initialize registry observer:\n" << ex;    
    }

private:

    const RegistryObserverTopicPtr _topic;
    const RegistryObserverPrx _observer;
    const int _serial;
};

class NodeInitCB : public AMI_NodeObserver_init
{
public:

    NodeInitCB(const NodeObserverTopicPtr& topic, const NodeObserverPrx& observer, int serial) : 
	_topic(topic),
	_observer(observer),
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
	out << "couldn't initialize node observer:\n" << ex;    
    }
    
private:

    const NodeObserverTopicPtr _topic;
    const NodeObserverPrx _observer;
    const int _serial;
};


NodeObserverTopic::NodeObserverTopic(const IceStorm::TopicManagerPrx& topicManager) : _serial(0)
{
    IceStorm::TopicPrx t;
    try
    {
	t = topicManager->create("NodeObserver");
    }
    catch(const IceStorm::TopicExists&)
    {
	t = topicManager->retrieve("NodeObserver");
    }

    //
    // NOTE: collocation optimization needs to be turned on for the
    // topic because the subscribe() method is given a fixed proxy
    // which can't be marshalled.
    //
    const_cast<IceStorm::TopicPrx&>(_topic) = IceStorm::TopicPrx::uncheckedCast(t->ice_collocationOptimized(true));
    const_cast<NodeObserverPrx&>(_publisher) = NodeObserverPrx::uncheckedCast(_topic->getPublisher()->ice_oneway());
}

void
NodeObserverTopic::init(const NodeDynamicInfoSeq&, const Ice::Current&)
{
    assert(false);
}

void
NodeObserverTopic::nodeUp(const NodeDynamicInfo& info, const Ice::Current& current)
{
    Lock sync(*this);
    _nodes.insert(make_pair(info.name, info));
    _publisher->nodeUp(info);
}

void 
NodeObserverTopic::nodeDown(const string& name, const Ice::Current&)
{
    Lock sync(*this);
    if(_nodes.find(name) != _nodes.end())
    {
	_nodes.erase(name);
	_publisher->nodeDown(name);
    }
}

void 
NodeObserverTopic::updateServer(const string& node, const ServerDynamicInfo& server, const Ice::Current&)
{
    Lock sync(*this);
    if(_nodes.find(node) == _nodes.end())
    {
	//
	// If the node isn't known anymore, we ignore the update.
	//
	return;
    }
    
    ++_serial;

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
    
    _publisher->updateServer(node, server);
}

void 
NodeObserverTopic::updateAdapter(const string& node, const AdapterDynamicInfo& adapter, const Ice::Current&)
{
    Lock sync(*this);
    if(_nodes.find(node) == _nodes.end())
    {
	//
	// If the node isn't known anymore, we ignore the update.
	//
	return;
    }

    ++_serial;

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
    
    _publisher->updateAdapter(node, adapter);
}

void
NodeObserverTopic::subscribe(const NodeObserverPrx& observer, int serial)
{
    while(true)
    {
	if(serial == -1)
	{
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
	    observer->init_async(new NodeInitCB(this, observer, serial), nodes);
	    return;
	}

	Lock sync(*this);
	if(serial != _serial)
	{
	    serial = -1;
	    continue;
	}

	IceStorm::QoS qos;
	qos["reliability"] = "twoway ordered";
	_topic->subscribe(qos, observer);
	break;
    }
}

void
NodeObserverTopic::unsubscribe(const NodeObserverPrx& observer)
{
    _topic->unsubscribe(observer);
}

RegistryObserverTopic::RegistryObserverTopic(const IceStorm::TopicManagerPrx& topicManager) : _serial(0)
{
    IceStorm::TopicPrx t;
    try
    {
	t = topicManager->create("RegistryObserver");
    }
    catch(const IceStorm::TopicExists&)
    {
	t = topicManager->retrieve("RegistryObserver");
    }

    //
    // NOTE: collocation optimization needs to be turned on for the
    // topic because the subscribe() method is given a fixed proxy
    // which can't be marshalled.
    //
    const_cast<IceStorm::TopicPrx&>(_topic) = IceStorm::TopicPrx::uncheckedCast(t->ice_collocationOptimized(true));
    const_cast<RegistryObserverPrx&>(_publisher) = 
	RegistryObserverPrx::uncheckedCast(_topic->getPublisher()->ice_oneway());
}

void 
RegistryObserverTopic::init(int serial, 
			    const ApplicationDescriptorSeq& apps, 
			    const AdapterInfoSeq& adpts,
			    const ObjectInfoSeq& objects,
			    const Ice::Current&)
{
    Lock sync(*this);

    _serial = serial;

    for(ApplicationDescriptorSeq::const_iterator p = apps.begin(); p != apps.end(); ++p)
    {
	_applications.insert(make_pair(p->name, *p));
    }
    for(AdapterInfoSeq::const_iterator q = adpts.begin(); q != adpts.end(); ++q)
    {
	_adapters.insert(make_pair(q->id, *q));
    }
    for(ObjectInfoSeq::const_iterator r = objects.begin(); r != objects.end(); ++r)
    {
	_objects.insert(make_pair(r->proxy->ice_getIdentity(), *r));
    }

    _publisher->init(serial, apps, adpts, objects);
}

void 
RegistryObserverTopic::applicationAdded(int serial, const ApplicationDescriptor& desc, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _applications.insert(make_pair(desc.name, desc));

    _publisher->applicationAdded(serial, desc);
}

void 
RegistryObserverTopic::applicationRemoved(int serial, const string& name, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _applications.erase(name);

    _publisher->applicationRemoved(serial, name);
}

void 
RegistryObserverTopic::applicationUpdated(int serial, const ApplicationUpdateDescriptor& desc, const Ice::Current& c)
{
    Lock sync(*this);

    updateSerial(serial);
    try
    {
	map<string, ApplicationDescriptor>::iterator p = _applications.find(desc.name);
	if(p != _applications.end())
	{
	    ApplicationHelper helper(c.adapter->getCommunicator(), p->second);
	    helper.update(desc);
	    p->second = helper.getDescriptor();
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

    _publisher->applicationUpdated(serial, desc);
}

void 
RegistryObserverTopic::adapterAdded(int serial, const AdapterInfo& info, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _adapters.insert(make_pair(info.id, info));

    _publisher->adapterAdded(serial, info);
}

void 
RegistryObserverTopic::adapterUpdated(int serial, const AdapterInfo& info, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _adapters[info.id] = info;

    _publisher->adapterUpdated(serial, info);
}

void
RegistryObserverTopic::adapterRemoved(int serial, const string& id, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _adapters.erase(id);

    _publisher->adapterRemoved(serial, id);
}

void 
RegistryObserverTopic::objectAdded(int serial, const ObjectInfo& info, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _objects.insert(make_pair(info.proxy->ice_getIdentity(), info));

    _publisher->objectAdded(serial, info);
}

void 
RegistryObserverTopic::objectUpdated(int serial, const ObjectInfo& info, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _objects[info.proxy->ice_getIdentity()] = info;

    _publisher->objectUpdated(serial, info);
}

void
RegistryObserverTopic::objectRemoved(int serial, const Ice::Identity& id, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _objects.erase(id);

    _publisher->objectRemoved(serial, id);
}

void 
RegistryObserverTopic::subscribe(const RegistryObserverPrx& observer, int serial)
{
    while(true)
    {
	if(serial == -1)
	{
	    ApplicationDescriptorSeq applications;
	    AdapterInfoSeq adapters;
	    ObjectInfoSeq objects;
	    {
		Lock sync(*this);
		assert(_serial != -1);
		serial = _serial;

		map<string, ApplicationDescriptor>::const_iterator p;
		for(p = _applications.begin(); p != _applications.end(); ++p)
		{
		    applications.push_back(p->second);
		}
		
		map<string, AdapterInfo>::const_iterator q;
		for(q = _adapters.begin(); q != _adapters.end(); ++q)
		{
		    adapters.push_back(q->second);
		}

		map<Ice::Identity, ObjectInfo>::const_iterator r;
		for(r = _objects.begin(); r != _objects.end(); ++r)
		{
		    objects.push_back(r->second);
		}
	    }
	    observer->init_async(new RegistryInitCB(this, observer, serial), serial, applications, adapters, objects);
	    return;
	}

	//
	// If the registry cache changed since we've send the init()
	// call we need to do it again. Otherwise, we can subscribe to
	// the IceStorm topic.
	//
	Lock sync(*this);
	if(serial != _serial)
	{
	    serial = -1;
	    continue;
	}

	IceStorm::QoS qos;
	qos["reliability"] = "twoway ordered";
	_topic->subscribe(qos, observer);
	break;
    }
}

void 
RegistryObserverTopic::unsubscribe(const RegistryObserverPrx& observer)
{
    Lock sync(*this);
    _topic->unsubscribe(observer);
}

void
RegistryObserverTopic::updateSerial(int serial)
{
    //
    // This loop ensures that updates from the database are processed
    // sequentially.
    //
    while(_serial + 1 != serial)
    {
	wait();
    }
    _serial = serial;
    notifyAll();
}
