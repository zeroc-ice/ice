// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

NodeObserverTopic::NodeObserverTopic(const IceStorm::TopicPrx& topic, const NodeObserverPrx& publisher) : 
    _topic(topic), _publisher(publisher)
{
}

void
NodeObserverTopic::init(const string& node, 
			const ServerDynamicInfoSeq& servers,
			const AdapterDynamicInfoSeq& adapters,
			const Ice::Current& current)
{
    Lock sync(*this);

    _nodes.insert(node);
    _servers[node] = servers;
    _adapters[node] = adapters;

    _publisher->init(node, servers, adapters);
}

void 
NodeObserverTopic::updateServer(const string& node, const ServerDynamicInfo& server, const Ice::Current&)
{
    Lock sync(*this);
    assert(_nodes.find(node) != _nodes.end());

    ServerDynamicInfoSeq& servers = _servers[node];
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

    AdapterDynamicInfoSeq& adapters = _adapters[node];
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
NodeObserverTopic::subscribe(const NodeObserverPrx& observer)
{
    IceStorm::QoS qos;
    qos["reliability"] = "twoway ordered";

    Lock sync(*this);
    _topic->subscribe(qos, observer);
    for(set<string>::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
    {
	try
	{
	    observer->init(*p, _servers[*p], _adapters[*p]);
	}
	catch(const Ice::LocalException& ex)
	{
	    cerr << ex << endl;
	    break;
	}
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
    _servers.erase(name);
    _adapters.erase(name);
}


RegistryObserverTopic::RegistryObserverTopic(const IceStorm::TopicPrx& topic, 
					     const RegistryObserverPrx& publisher,
					     NodeObserverTopic& nodeObserver) :
    _topic(topic), _publisher(publisher), _nodeObserver(nodeObserver)
{
}

void 
RegistryObserverTopic::init(int serial, const ApplicationDescriptorSeq& apps, const Ice::StringSeq& nodes, 
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

    _applications.push_back(desc);

    _publisher->applicationAdded(serial, desc);
}

void 
RegistryObserverTopic::applicationRemoved(int serial, const string& name, const Ice::Current&)
{
    Lock sync(*this);

    //
    // TODO: update cache
    //

    _publisher->applicationRemoved(serial, name);
}

void 
RegistryObserverTopic::applicationSynced(int serial, const ApplicationDescriptorPtr& desc, const Ice::Current&)
{
    Lock sync(*this);

    //
    // TODO: update cache
    //

    _publisher->applicationSynced(serial, desc);
}

void 
RegistryObserverTopic::applicationUpdated(int serial, const ApplicationUpdateDescriptor& desc, const Ice::Current&)
{
    Lock sync(*this);

    //
    // TODO: update cache
    //

    _publisher->applicationUpdated(serial, desc);
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
RegistryObserverTopic::subscribe(const RegistryObserverPrx& observer)
{
    IceStorm::QoS qos;
    qos["reliability"] = "twoway ordered";

    Lock sync(*this);
    _topic->subscribe(qos, observer);

    try
    {
	observer->init(_serial, _applications, _nodes);
    }
    catch(const Ice::LocalException& ex)
    {
	cerr << ex << endl;
    }
}

void 
RegistryObserverTopic::unsubscribe(const RegistryObserverPrx& observer)
{
    Lock sync(*this);
    _topic->unsubscribe(observer);
}
