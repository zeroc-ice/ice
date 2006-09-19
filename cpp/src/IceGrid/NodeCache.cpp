// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Ice/Communicator.h>
#include <Ice/Properties.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/ReplicaCache.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class LoadCB : public AMI_Node_loadServer
{
public:

    LoadCB(const TraceLevelsPtr& traceLevels, 
	   const ServerEntryPtr& server, 
	   const string& id, 
	   const string& node,
	   int timeout) : 
	_traceLevels(traceLevels), _server(server), _id(id), _node(node), _timeout(timeout)
    {
    }

    void
    ice_response(const ServerPrx& proxy, const AdapterPrxDict& adpts, int at, int dt)
    {
	if(_traceLevels && _traceLevels->server > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "loaded `" << _id << "' on node `" << _node << "'";	
	}

	//
	// Apply the node session timeout on the proxies.
	//
	ServerPrx server = ServerPrx::uncheckedCast(proxy->ice_timeout(_timeout * 1000));
	AdapterPrxDict adapters;
	for(AdapterPrxDict::const_iterator p = adpts.begin(); p != adpts.end(); ++p)
	{
	    adapters.insert(make_pair(p->first, AdapterPrx::uncheckedCast(p->second->ice_timeout(_timeout * 1000))));
	}

	_server->loadCallback(server, adapters, at + _timeout, dt + _timeout);
    }

    void
    ice_exception(const Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const NodeNotExistException& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex;
	    }
	    ostringstream os;
	    os << ex;
	    _server->exception(NodeUnreachableException(_node, os.str()));
	}
	catch(const DeploymentException& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex.reason;
	    }

	    ostringstream os;
	    os << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex.reason;
	    _server->exception(DeploymentException(os.str()));
	}
	catch(const Ice::Exception& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't load `" << _id << "' on node `" << _node << "':\n" << ex;
	    }
	    
	    ostringstream os;
	    os << ex;
	    _server->exception(NodeUnreachableException(_node, os.str()));
	}
    }

private:
    
    const TraceLevelsPtr _traceLevels;
    const ServerEntryPtr _server;
    const string _id;
    const string _node;
    const int _timeout;
};

class DestroyCB : public AMI_Node_destroyServer
{
public:

    DestroyCB(const TraceLevelsPtr& traceLevels, const ServerEntryPtr& server, const string& id, const string& node) : 
	_traceLevels(traceLevels), _server(server), _id(id), _node(node)
    {
    }

    void
    ice_response()
    {
	if(_traceLevels && _traceLevels->server > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "unloaded `" << _id << "' on node `" << _node << "'";	
	}
	_server->destroyCallback();
    }

    void
    ice_exception(const Ice::Exception& ex)
    {
	try
	{
	    ex.ice_throw();
	}
	catch(const NodeNotExistException& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex;
	    }
	    ostringstream os;
	    os << ex;
 	    _server->exception(NodeUnreachableException(_node, os.str()));
	}
	catch(const DeploymentException& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex.reason;
	    }

	    ostringstream os;
	    os << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex.reason;
	    _server->exception(DeploymentException(os.str()));
	}
	catch(const Ice::Exception& ex)
	{
	    if(_traceLevels && _traceLevels->server > 1)
	    {
		Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
		out << "couldn't unload `" << _id << "' on node `" << _node << "':\n" << ex;
	    }
	    ostringstream os;
	    os << ex;
	    _server->exception(NodeUnreachableException(_node, os.str()));
	}
    }

private:
    
    const TraceLevelsPtr _traceLevels;
    const ServerEntryPtr _server;
    const string _id;
    const string _node;
};

};

NodeCache::NodeCache(const Ice::CommunicatorPtr& communicator, ReplicaCache& replicaCache, bool master) :
    _communicator(communicator),
    _master(master),
    _replicaCache(replicaCache)
{
}

void
NodeCache::destroy()
{
    map<string, NodeEntryPtr> entries = _entries; // Copying the map is necessary as setSession might remove the entry.
    for(map<string, NodeEntryPtr>::const_iterator p = entries.begin(); p != entries.end(); ++p)
    {
	p->second->setSession(0); // Break cyclic reference count.
    }    
}

NodeEntryPtr
NodeCache::get(const string& name, bool create) const
{
    Lock sync(*this);
    NodeEntryPtr entry = getImpl(name);
    if(!entry && create)
    {
	NodeCache& self = const_cast<NodeCache&>(*this);
	entry = new NodeEntry(self, name);
	self.addImpl(name, entry);
    }
    if(!entry)
    {
	NodeNotExistException ex;
	ex.name = name;
	throw ex;
    }
    return entry;
}

NodeEntry::NodeEntry(NodeCache& cache, const std::string& name) : 
    _cache(cache),
    _ref(0),
    _name(name)
{
}

NodeEntry::~NodeEntry()
{
}

void
NodeEntry::addDescriptor(const string& application, const NodeDescriptor& descriptor)
{
    Lock sync(*this);
    _descriptors.insert(make_pair(application, descriptor));
}

void
NodeEntry::removeDescriptor(const string& application)
{
    Lock sync(*this);
    _descriptors.erase(application);
}

void
NodeEntry::addServer(const ServerEntryPtr& entry)
{
    Lock sync(*this);
    _servers.insert(make_pair(entry->getId(), entry));
}

void
NodeEntry::removeServer(const ServerEntryPtr& entry)
{
    Lock sync(*this);
    _servers.erase(entry->getId());
}

void
NodeEntry::setSession(const NodeSessionIPtr& session)
{
    Lock sync(*this);

    if(session)
    {
	// If the current session has just been destroyed, wait for the setSession(0) call.
	assert(session != _session);
	while(_session && _session->isDestroyed()) 
	{
	    wait();
	}
    }
    
    if(session && _session)
    {
	throw NodeActiveException();
    }
    else if(!session && !_session)
    {
	return;
    }
    
    if(!session && _session)
    {
	_cache.getReplicaCache().nodeRemoved(_session->getNode());
    }
    
    _session = session;
    notifyAll();

    if(_session)
    {
	_cache.getReplicaCache().nodeAdded(session->getNode());
    }
    
    if(session)
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
	    out << "node `" << _name << "' up";
	}
    }
    else
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
	    out << "node `" << _name << "' down";
	}
    }
}

NodePrx
NodeEntry::getProxy() const
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException(_name, "the node is not active");
    }
    return _session->getNode();
}

NodeInfo
NodeEntry::getInfo() const
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException(_name, "the node is not active");
    }
    return _session->getInfo();
}

Ice::StringSeq
NodeEntry::getServers() const
{
    Lock sync(*this);
    Ice::StringSeq names;
    for(map<string, ServerEntryPtr>::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
    {
	names.push_back(p->second->getId());
    }
    return names;
}

LoadInfo
NodeEntry::getLoadInfoAndLoadFactor(const string& application, float& loadFactor) const
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException(_name, "the node is not active");
    }
    map<string, NodeDescriptor>::const_iterator p = _descriptors.find(application);
    loadFactor = -1.0f;
    if(p != _descriptors.end())
    {
	if(!p->second.loadFactor.empty())
	{
	    istringstream is(p->second.loadFactor);
	    is >> loadFactor;
	}
    }
    if(loadFactor < 0.0f)
    {
	if(_session->getInfo().os != "Windows")
	{
	    //
	    // On Unix platforms, we divide the load averages by the
	    // number of processors. A load of 2 on a dual processor
	    // machine is the same as a load of 1 on a single process
	    // machine.
	    //
	    loadFactor = 1.0f / _session->getInfo().nProcessors;
	}
	else
	{
	    //
	    // On Windows, load1, load5 and load15 are the average of
	    // the CPU utilization (all CPUs). We don't need to divide
	    // by the number of CPU.
	    //
	    loadFactor = 1.0f;
	}
    }
    return _session->getLoadInfo();
}

bool
NodeEntry::canRemove()
{
    Lock sync(*this);
    return _servers.empty() && !_session && _descriptors.empty();
}

void
NodeEntry::loadServer(const ServerEntryPtr& entry, const ServerInfo& server, const SessionIPtr& session)
{
    try
    {
	NodePrx node;
	int timeout;
	ServerDescriptorPtr desc;
	{
	    Lock sync(*this);
	    if(!_session)
	    {
		throw NodeUnreachableException(_name, "the node is not active");
	    }
	    node = _session->getNode();
	    timeout = _session->getTimeout();
	    try
	    {
		desc = getServerDescriptor(server, session);
	    }
	    catch(const DeploymentException&)
	    {
		//
		// We ignore the deployment error for now (which can
		// only be caused in theory by session variables not
		// being defined because the server isn't
		// allocated...)
		//
		// TODO: Once we have node-bound & not node-bound
		// servers, we shouldn't ignore errors anymore
		// (session servers will only be bound to the node if
		// they are allocated by a session).
		//
		desc = server.descriptor;
	    }
	}
	assert(desc);
	
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
	    out << "loading `" << desc->id << "' on node `" << _name << "'";
	    if(session)
	    {
		out << " for session `" << session->getId() << "'";
	    }
	}
	
	AMI_Node_loadServerPtr amiCB = new LoadCB(_cache.getTraceLevels(), entry, entry->getId(), _name, timeout);
	ServerInfo info = server;
	info.descriptor = desc;
	node->loadServer_async(amiCB, info, _cache.isMaster());
    }
    catch(const NodeUnreachableException& ex)
    {
	entry->exception(ex);
    }
}

void
NodeEntry::destroyServer(const ServerEntryPtr& entry, const ServerInfo& info)
{
    try
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
	    out << "unloading `" << info.descriptor->id << "' on node `" << _name << "'";	
	}
	AMI_Node_destroyServerPtr amiCB = new DestroyCB(_cache.getTraceLevels(), entry, info.descriptor->id, _name);
	getProxy()->destroyServer_async(amiCB, info.descriptor->id, info.uuid, info.revision);
    }
    catch(const NodeUnreachableException& ex)
    {
	entry->exception(ex);
    }
}

ServerInfo
NodeEntry::getServerInfo(const ServerInfo& server, const SessionIPtr& session)
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException(_name, "the node is not active");
    }
    ServerInfo info = server;
    info.descriptor = getServerDescriptor(server, session);
    assert(info.descriptor);
    return info;
}

ServerDescriptorPtr
NodeEntry::getServerDescriptor(const ServerInfo& server, const SessionIPtr& session)
{
    assert(_session);

    Resolver resolve(_session->getInfo(), _cache.getCommunicator());
    resolve.setReserved("application", server.application);
    resolve.setReserved("server", server.descriptor->id);
    resolve.setContext("server `${server}'");

    if(session)
    {
	resolve.setReserved("session.id", session->getId());
    }

    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server.descriptor);
    if(iceBox)
    {
	return IceBoxHelper(_cache.getCommunicator(), iceBox).instantiate(resolve, PropertyDescriptorSeq());
    }
    else
    {
	return ServerHelper(_cache.getCommunicator(), server.descriptor).instantiate(resolve, PropertyDescriptorSeq());
    }
}

void
NodeEntry::__incRef()
{
    Lock sync(*this);
    assert(_ref >= 0);
    ++_ref;
}

void
NodeEntry::__decRef()
{
    //
    // The node entry implements its own reference counting. If the
    // reference count drops to 1, this means that only the cache
    // holds a reference on the node entry. If that's the case, we
    // check if the node entry can be removed or not and if it can be
    // removed we remove it from the cache map.
    //

    bool doRemove = false;
    bool doDelete = false;
    {
	Lock sync(*this);
	assert(_ref > 0);
	--_ref;
	
	if(_ref == 1)
	{
	    doRemove = _servers.empty() && !_session && _descriptors.empty();
	}
	else if(_ref == 0)
	{
	    doDelete = true;
	}
    }

    if(doRemove)
    {
	_cache.remove(_name);
    }
    else if(doDelete)
    {
	delete this;
    }
}
