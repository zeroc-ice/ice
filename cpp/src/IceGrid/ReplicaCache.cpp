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

void
ReplicaCache::destroy()
{
    _entries.clear();
}

ReplicaEntryPtr
ReplicaCache::add(const string& name, const ReplicaSessionIPtr& session)
{
    ReplicaEntryPtr entry;
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
    
	entry = addImpl(name, new ReplicaEntry(name, session));
    }

    try
    {
	_nodes->replicaAdded(session->getInternalRegistry());
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
    }

    return entry;
}

ReplicaEntryPtr
ReplicaCache::remove(const string& name)
{
    ReplicaEntryPtr entry;
    {
	Lock sync(*this);
	
	entry = removeImpl(name);
	assert(entry);
	
	if(_traceLevels && _traceLevels->replica > 0)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
	    out << "replica `" << name << "' down";
	}

	//
	// Remove the replica expected updates.
	//
	removeReplicaUpdates(name);
    }
    
    try
    {
	_nodes->replicaRemoved(entry->getSession()->getInternalRegistry());
    }
    catch(const Ice::LocalException&)
    {
	// TODO: XXX
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
ReplicaCache::getEndpoints(const string& name, const Ice::ObjectPrx& proxy) const
{
    Ice::EndpointSeq endpoints;

    if(proxy)
    {
	Ice::EndpointSeq endpts = proxy->ice_getEndpoints();
	endpoints.insert(endpoints.end(), endpts.begin(), endpts.end());
    }

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

void
ReplicaCache::waitForUpdateReplication(const string& name, int serial)
{
    Lock sync(*this);
    if(_entries.empty())
    {
	return;
    }
    
    vector<string> replicas;
    for(map<string, ReplicaEntryPtr>::const_iterator s = _entries.begin(); s != _entries.end(); ++s)
    {
	replicas.push_back(s->first);
    }

    ostringstream os;
    os << name << "-" << serial;
    const string key = os.str();

    _waitForUpdates.insert(make_pair(key, set<string>(replicas.begin(), replicas.end())));

    //
    // Wait until all the updates are received.
    //
    while(true)
    {
	map<string, set<string> >::const_iterator p = _waitForUpdates.find(key);
	if(p == _waitForUpdates.end())
	{
	    map<string, map<string, string> >::iterator q = _updateFailures.find(key);
	    if(q != _updateFailures.end())
	    {
		map<string, string> failures = q->second;
		_updateFailures.erase(q);
		
		ostringstream os;
		for(map<string, string>::const_iterator r = failures.begin(); r != failures.end(); ++r)
		{
		    os << "replication failed on replica `" << r->first << "':\n" << r->second << "\n";
		}
		Ice::Error err(_traceLevels->logger);
		err << os.str();
	    }
	    return;
	}
	else
	{
	    wait();
	}
    }
}

void
ReplicaCache::replicaReceivedUpdate(const string& name, const string& update, int serial, const string& failure)
{
    Lock sync(*this);

    ostringstream os;
    os << update << "-" << serial;
    const string key = os.str();

    map<string, set<string> >::iterator p = _waitForUpdates.find(key);
    if(p != _waitForUpdates.end())
    {
	p->second.erase(name);
	
	if(!failure.empty())
	{
	    map<string, map<string, string> >::iterator q = _updateFailures.find(key);
	    if(q == _updateFailures.end())
	    {
		q = _updateFailures.insert(make_pair(key, map<string ,string>())).first;
	    }
	    q->second.insert(make_pair(name, failure));
	}

	if(p->second.empty())
	{
	    _waitForUpdates.erase(p);
	    notifyAll();
	}
    }
}

void
ReplicaCache::startApplicationReplication(const string& application, int revision)
{
    //
    // Add the given application to the set of application being
    // replicated.
    //
    Lock sync(*this);
    _applicationReplication.insert(application);
}


void
ReplicaCache::finishApplicationReplication(const string& application, int revision)
{
    //
    // Notify waiting threads that the given application replication
    // is completed.
    //
    Lock sync(*this);
    _applicationReplication.erase(application);
    notifyAll();
}

void
ReplicaCache::waitForApplicationReplication(const string& application, int revision)
{
    //
    // Wait for the given application to be replicated.
    // 
    Lock sync(*this);
    while(_applicationReplication.find(application) != _applicationReplication.end())
    {
	wait();
    }
}

void
ReplicaCache::removeReplicaUpdates(const string& name)
{
    // Must b called within the synchronization.

    map<string, set<string> >::iterator p = _waitForUpdates.begin();
    bool notifyMonitor = false;
    while(p != _waitForUpdates.end())
    {
	p->second.erase(name);
	if(p->second.empty())
	{
	    _waitForUpdates.erase(p++);
	    notifyMonitor = true;
	}
	else
	{
	    ++p;
	}
    }
    
    if(notifyMonitor)
    {
	notifyAll();
    }
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

