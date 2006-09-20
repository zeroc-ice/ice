// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LocalException.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/ReplicaSessionI.h>
#include <IceGrid/Topics.h>

using namespace std;
using namespace IceGrid;

ReplicaCache::ReplicaCache(const Ice::CommunicatorPtr& communicator, const IceStorm::TopicManagerPrx& topicManager) :
    _communicator(communicator)
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

ReplicaEntryPtr
ReplicaCache::add(const string& name, const ReplicaSessionIPtr& session)
{
    ReplicaEntryPtr entry;
    {
	Lock sync(*this);

	while(true)
	{
	    ReplicaEntryPtr entry = getImpl(name);
	    if(entry)
	    {
		if(entry->getSession()->isDestroyed())
		{
		    wait();
		    continue;
		}
		else
		{
		    throw ReplicaActiveException();
		}
	    }
	    break;
	}
    
	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "replica `" << name << "' up";
	}
    
	entry = addImpl(name, new ReplicaEntry(name, session));
    }

    //
    // Note: it's safe to do this outside the synchronization because
    // remove() can't be called until this method returns (and until
    // the replica session is fully created).
    //
    try
    {
	_nodes->replicaAdded(session->getInternalRegistry());
    }
    catch(const Ice::ConnectionRefusedException&)
    {
	// Expected if the replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
	TraceLevelsPtr traceLevels = getTraceLevels();
	if(traceLevels)
	{
	    Ice::Warning out(traceLevels->logger);
	    out << "unexpected exception while publishing `replicaAdded' update:\n" << ex;    
	}
    }

    return entry;
}

ReplicaEntryPtr
ReplicaCache::remove(const string& name, bool shutdown)
{
    ReplicaEntryPtr entry;
    {
	Lock sync(*this);
	entry = getImpl(name);
	assert(entry);
	removeImpl(name);
	notifyAll();
	
	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "replica `" << name << "' down";
	}
    }

    if(!shutdown)
    {
	try
	{
	    _nodes->replicaRemoved(entry->getSession()->getInternalRegistry());
	}
	catch(const Ice::ConnectionRefusedException&)
	{
	    // Expected if the replica is being shutdown.
	}
	catch(const Ice::LocalException& ex)
	{
	    TraceLevelsPtr traceLevels = getTraceLevels();
	    if(traceLevels)
	    {
		Ice::Warning out(traceLevels->logger);
		out << "unexpected exception while publishing `replicaRemoved' update:\n" << ex;    
	    }
	}
    }
    
    return entry;
}

ReplicaEntryPtr
ReplicaCache::get(const string& name) const
{
    Lock sync(*this);
    ReplicaEntryPtr entry = getImpl(name);
    if(!entry)
    {
	RegistryNotExistException ex;
	ex.name = name;
	throw ex;
    }
    return entry;
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
    catch(const Ice::ConnectionRefusedException& ex)
    {
	// The replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
	TraceLevelsPtr traceLevels = getTraceLevels();
	if(traceLevels)
	{
	    Ice::Warning out(traceLevels->logger);
	    out << "unexpected exception while subscribing node from replica observer topic:\n" << ex;    
	}
    }
}

void
ReplicaCache::nodeRemoved(const NodePrx& node)
{
    try
    {
	_topic->unsubscribe(node);
    }
    catch(const Ice::ConnectionRefusedException& ex)
    {
	// The replica is being shutdown.
    }
    catch(const Ice::LocalException& ex)
    {
	TraceLevelsPtr traceLevels = getTraceLevels();
	if(traceLevels)
	{
	    Ice::Warning out(traceLevels->logger);
	    out << "unexpected exception while unsubscribing node from replica observer topic:\n" << ex;    
	}
    }
}

Ice::ObjectPrx
ReplicaCache::getEndpoints(const string& name, const Ice::ObjectPrx& proxy) const
{
    Ice::EndpointSeq endpoints;

    if(proxy)
    {
	Ice::EndpointSeq endpts = proxy->ice_getEndpoints();
	endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

    Lock sync(*this);
    for(map<string, ReplicaEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
	Ice::ObjectPrx prx = p->second->getSession()->getEndpoint(name);
	if(prx)
	{
	    Ice::EndpointSeq endpts = prx->ice_getEndpoints();
	    endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
	}
    }

    return _communicator->stringToProxy("dummy")->ice_endpoints(endpoints);
}

ReplicaEntry::ReplicaEntry(const std::string& name, const ReplicaSessionIPtr& session) : 
    _name(name),
    _session(session)
{
}

ReplicaEntry::~ReplicaEntry()
{
}

const ReplicaSessionIPtr&
ReplicaEntry::getSession() const
{ 
    return _session;
}

RegistryInfo
ReplicaEntry::getInfo() const
{
    return _session->getInfo();
}

InternalRegistryPrx
ReplicaEntry::getProxy() const
{
    return _session->getInternalRegistry();
}

