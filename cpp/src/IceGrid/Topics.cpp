// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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
    ice_exception(const Ice::Exception&)
    {
	// Ignore
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
    ice_exception(const Ice::Exception&)
    {
	// Ignore
    }

private:

    const NodeObserverTopicPtr _topic;
    const NodeObserverPrx _observer;
    const int _serial;
};


NodeObserverTopic::NodeObserverTopic(const IceStorm::TopicPrx& topic, const NodeObserverPrx& publisher) : 
    _topic(topic), _publisher(publisher), _serial(0)
{
}

void
NodeObserverTopic::init(const NodeDynamicInfoSeq&, const Ice::Current&)
{
    assert(false);
}

void
NodeObserverTopic::initNode(const NodeDynamicInfo& info, const Ice::Current& current)
{
    Lock sync(*this);

    _nodes.insert(make_pair(info.name, info));

    _publisher->initNode(info);
}

void 
NodeObserverTopic::updateServer(const string& node, const ServerDynamicInfo& server, const Ice::Current&)
{
    Lock sync(*this);
    assert(_nodes.find(node) != _nodes.end());

    ++_serial;

    ServerDynamicInfoSeq& servers = _nodes[node].servers;
    ServerDynamicInfoSeq::iterator p = servers.begin();
    while(p != servers.end())
    {
	if(p->name == server.name)
	{
	    *p = server;
	    break;
	}
	++p;
    }
    if(p == servers.end())
    {
	servers.push_back(server);
    }
    
    _publisher->updateServer(node, server);
}

void 
NodeObserverTopic::updateAdapter(const string& node, const AdapterDynamicInfo& adapter, const Ice::Current&)
{
    Lock sync(*this);
    assert(_nodes.find(node) != _nodes.end());

    ++_serial;

    AdapterDynamicInfoSeq& adapters = _nodes[node].adapters;
    AdapterDynamicInfoSeq::iterator p = adapters.begin();
    while(p != adapters.end())
    {
	if(p->id == adapter.id)
	{
	    *p = adapter;
	    break;
	}
	++p;
    }
    if(p == adapters.end())
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
	    Lock sync(*this);
	    NodeDynamicInfoSeq nodes;
	    nodes.reserve(_nodes.size());
	    for(map<string, NodeDynamicInfo>::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
	    {
		nodes.push_back(p->second);
	    }
	    observer->init_async(new NodeInitCB(this, observer, _serial), nodes);
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

void
NodeObserverTopic::removeNode(const string& name)
{
    Lock sync(*this);
    _nodes.erase(name);
}

RegistryObserverTopic::RegistryObserverTopic(const IceStorm::TopicPrx& topic, 
					     const RegistryObserverPrx& publisher,
					     NodeObserverTopic& nodeObserver) :
    _topic(topic), _publisher(publisher), _nodeObserver(nodeObserver), _serial(0)
{
}

void 
RegistryObserverTopic::init(int serial,
			    const ApplicationDescriptorSeq& apps, 
			    const Ice::StringSeq& nodes, 
			    const Ice::Current&)
{
    Lock sync(*this);

    _serial = serial;
    _applications = apps;
    _nodes = nodes;

    _publisher->init(serial, apps, nodes);
}

void 
RegistryObserverTopic::applicationAdded(int serial, const ApplicationDescriptorPtr& desc, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);

    _applications.push_back(desc);

    _publisher->applicationAdded(serial, desc);
}

void 
RegistryObserverTopic::applicationRemoved(int serial, const string& name, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);
    for(ApplicationDescriptorSeq::iterator p = _applications.begin(); p != _applications.end(); ++p)
    {
	if((*p)->name == name)
	{
	    _applications.erase(p);
	    break;
	}
    }

    _publisher->applicationRemoved(serial, name);
}

void 
RegistryObserverTopic::applicationSynced(int serial, const ApplicationDescriptorPtr& desc, const Ice::Current&)
{
    Lock sync(*this);

    updateSerial(serial);
    for(ApplicationDescriptorSeq::iterator p = _applications.begin(); p != _applications.end(); ++p)
    {
	if((*p)->name == desc->name)
	{
	    *p = desc;
	    break;
	}
    }

    _publisher->applicationSynced(serial, desc);
}

void 
RegistryObserverTopic::applicationUpdated(int serial, const ApplicationUpdateDescriptor& desc, const Ice::Current& c)
{
    Lock sync(*this);

    updateSerial(serial);
    ApplicationUpdateDescriptor newDesc;
    try
    {
	for(ApplicationDescriptorSeq::iterator p = _applications.begin(); p != _applications.end(); ++p)
	{
	    if((*p)->name == desc.name)
	    {
		ApplicationDescriptorHelper helper(c.adapter->getCommunicator(), *p);
		newDesc = helper.update(desc);
		*p = helper.getDescriptor();
		break;
	    }
	}
    }
    catch(...)
    {
	assert(false);
    }

    _publisher->applicationUpdated(serial, newDesc);
}

void 
RegistryObserverTopic::nodeUp(const string& name, const Ice::Current&)
{
    Lock sync(*this);

    assert(find(_nodes.begin(), _nodes.end(), name) == _nodes.end());
    _nodes.push_back(name);

    _publisher->nodeUp(name);
}

void 
RegistryObserverTopic::nodeDown(const string& name, const Ice::Current&)
{
    Lock sync(*this);

    Ice::StringSeq::iterator p = find(_nodes.begin(), _nodes.end(), name);
    assert(p != _nodes.end());
    _nodes.erase(p);

    _nodeObserver.removeNode(name);

    _publisher->nodeDown(name);
}

void 
RegistryObserverTopic::subscribe(const RegistryObserverPrx& observer, int serial)
{
    while(true)
    {
	if(serial == -1)
	{
	    Lock sync(*this);
	    assert(_serial != -1);
	    observer->init_async(new RegistryInitCB(this, observer, _serial), _serial, _applications, _nodes);
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
