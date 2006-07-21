// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/LoggerUtil.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Database.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

ReplicaCache::ReplicaCache(const Ice::CommunicatorPtr& communicator, 
			   const IceStorm::TopicManagerPrx& topicManager,
			   const std::string& instanceName,
			   const Ice::ObjectPrx& clientProxy,
			   const Ice::ObjectPrx& serverProxy) :
    _communicator(communicator),
    _instanceName(instanceName),
    _clientProxy(clientProxy),
    _serverProxy(serverProxy)
{
    IceStorm::TopicPrx t;
    try
    {
	t = topicManager->create("NodeNotifier");
    }
    catch(const IceStorm::TopicExists&)
    {
	t = topicManager->retrieve("NodeNotifier");
    }

    const_cast<IceStorm::TopicPrx&>(_topic) = t;
    const_cast<NodePrx&>(_nodes) = NodePrx::uncheckedCast(_topic->getPublisher());
}

void
ReplicaCache::destroy()
{
    //
    // TODO: XXX: Is this also really needed for replicas?!
    //
    _entries.clear();
}

void
ReplicaCache::add(const string& name, const ReplicaSessionIPtr& session, const DatabasePtr& database)
{
    Lock sync(*this);

    if(getImpl(name))
    {
	throw ReplicaActiveException();
    }
    
    if(_traceLevels && _traceLevels->replica > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	out << "replica `" << name << "' up";
    }
    
    addImpl(name, new ReplicaEntry(name, session));

    ObjectInfo info;
    info.type = InternalRegistry::ice_staticId();
    info.proxy = session->getProxy();
    database->addObject(info, true);

    Ice::ObjectPrx clientProxy = getClientProxy();
    Ice::Identity id;
    id.category = _instanceName;
    id.name = "Query";
    database->updateObject(clientProxy->ice_identity(id));

    RegistryObserverTopicPtr topic = database->getRegistryObserverTopic();
    if(topic)
    {
	topic->subscribe(session->getReplicaInfo().observer);
    }

    try
    {
	_nodes->replicaAdded(session->getProxy());
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
    }
}

void
ReplicaCache::remove(const string& name, const DatabasePtr& database)
{
    Lock sync(*this);

    ReplicaEntryPtr entry = removeImpl(name);
    assert(entry);

    try
    {
	_nodes->replicaRemoved(entry->getSession()->getProxy());
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
    }

    RegistryObserverTopicPtr topic = database->getRegistryObserverTopic();
    if(topic)
    {
	topic->unsubscribe(entry->getSession()->getReplicaInfo().observer);
    }

    database->removeObject(entry->getSession()->getProxy()->ice_getIdentity());

    Ice::ObjectPrx clientProxy = getClientProxy();
    Ice::Identity id;
    id.category = _instanceName;
    id.name = "Query";
    database->updateObject(clientProxy->ice_identity(id));

    if(_traceLevels && _traceLevels->replica > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	out << "replica `" << name << "' down";
    }
}

void
ReplicaCache::nodeAdded(const NodePrx& node)
{
    IceStorm::QoS qos;
    qos["reliability"] = "twoway ordered";
    try
    {
	_topic->subscribe(qos, node);
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
    }
}

void
ReplicaCache::nodeRemoved(const NodePrx& node)
{
    try
    {
	_topic->unsubscribe(node);
    }
    catch(const Ice::ConnectionRefusedException&)
    {
	// The replica is being shutdown.
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
    }
}

Ice::ObjectPrx
ReplicaCache::getClientProxy() const
{
    Ice::EndpointSeq endpoints;

    Ice::EndpointSeq endpts = _clientProxy->ice_getEndpoints();
    endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());

    for(map<string, ReplicaEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
	endpts = p->second->getSession()->getReplicaInfo().clientProxy->ice_getEndpoints();
	endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    return _clientProxy->ice_endpoints(endpoints);
}

Ice::ObjectPrx
ReplicaCache::getServerProxy() const
{
    Ice::EndpointSeq endpoints;

    Ice::EndpointSeq endpts = _serverProxy->ice_getEndpoints();
    endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());

    for(map<string, ReplicaEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
	endpts = p->second->getSession()->getReplicaInfo().serverProxy->ice_getEndpoints();
	endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    return _serverProxy->ice_endpoints(endpoints);
}


ReplicaEntry::ReplicaEntry(const std::string& name, const ReplicaSessionIPtr& session) : 
    _name(name),
    _session(session)
{
}

const ReplicaSessionIPtr&
ReplicaEntry::getSession() const
{ 
    return _session;
}

