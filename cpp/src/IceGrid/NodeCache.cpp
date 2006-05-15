// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Functional.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/NodeCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/DescriptorHelper.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

class LoadCB : public AMI_Node_loadServer
{
public:

    LoadCB(const TraceLevelsPtr& traceLevels, const ServerEntryPtr& server, const string& id, const string& node) : 
	_traceLevels(traceLevels), _server(server), _id(id), _node(node)
    {
    }

    void
    ice_response(const ServerPrx& proxy, const AdapterPrxDict& adapters, int at, int dt)
    {
	if(_traceLevels && _traceLevels->server > 1)
	{
	    Ice::Trace out(_traceLevels->logger, _traceLevels->serverCat);
	    out << "loaded `" << _id << "' on node `" << _node << "'";	
	}
	_server->loadCallback(ServerPrx::uncheckedCast(proxy->ice_collocationOptimization(false)), adapters, at, dt);
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

NodeCache::NodeCache(const Ice::CommunicatorPtr& communicator, int sessionTimeout) :
    _communicator(communicator),
    _sessionTimeout(sessionTimeout)
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
    _name(name)
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
    bool remove = false;
    {
	Lock sync(*this);
	_descriptors.erase(application);
	remove = _servers.empty() && !_session && _descriptors.empty();
    }
    if(remove)
    {
	_cache.remove(_name);
    }    
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
    bool remove = false;
    {
	Lock sync(*this);
	_servers.erase(entry->getId());
	remove = _servers.empty() && !_session && _descriptors.empty();
    }
    if(remove)
    {
	_cache.remove(_name);
    }
}

void
NodeEntry::setSession(const NodeSessionIPtr& session)
{
    bool remove = false;
    {
	Lock sync(*this);
	if(session && _session)
	{
	    throw NodeActiveException();
	}
	_session = session;
	remove = _servers.empty() && !_session && _descriptors.empty();
    }
    
    if(session)
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
	    out << "node `" << _name << "' up";
	}

	ServerEntrySeq entries;
	{
	    Lock sync(*this);
	    for(map<string, ServerEntryPtr>::const_iterator q = _servers.begin() ; q != _servers.end(); ++q)
	    {
		entries.push_back(q->second);
	    }
	}
	for_each(entries.begin(), entries.end(), IceUtil::voidMemFun(&ServerEntry::sync));
    }
    else
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->node > 0)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->nodeCat);
	    out << "node `" << _name << "' down";
	}
    }

    //
    // NOTE: this needs to be the last thing to do as this will
    // destroy this entry.
    //
    if(remove)
    {
	_cache.remove(_name);
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
    return !_session && _servers.empty();
}

void
NodeEntry::loadServer(const ServerEntryPtr& entry, const ServerInfo& server)
{
    NodePrx node;
    ServerDescriptorPtr desc;
    {
	Lock sync(*this);
	if(!_session)
	{
	    entry->exception(NodeUnreachableException(_name, "the node is not active"));
	    return;
	}
	node = _session->getNode();
	desc = getServerDescriptor(server);
    }
    assert(desc);

    if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
    {
	Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
	out << "loading `" << desc->id << "' on node `" << _name << "'";	
    }

    AMI_Node_loadServerPtr amiCB = new LoadCB(_cache.getTraceLevels(), entry, desc->id, _name);
    node->loadServer_async(amiCB, server.application, desc);
}

void
NodeEntry::destroyServer(const ServerEntryPtr& entry, const string& id)
{
    try
    {
	if(_cache.getTraceLevels() && _cache.getTraceLevels()->server > 2)
	{
	    Ice::Trace out(_cache.getTraceLevels()->logger, _cache.getTraceLevels()->serverCat);
	    out << "unloading `" << id << "' on node `" << _name << "'";	
	}
	AMI_Node_destroyServerPtr amiCB = new DestroyCB(_cache.getTraceLevels(), entry, id, _name);
	getProxy()->destroyServer_async(amiCB, id);
    }
    catch(const NodeUnreachableException& ex)
    {
	entry->exception(ex);
	return;
    }
}

ServerInfo
NodeEntry::getServerInfo(const ServerInfo& server)
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException(_name, "the node is not active");
    }
    ServerInfo info = server;
    info.descriptor = getServerDescriptor(server);
    assert(info.descriptor);
    return info;
}

ServerDescriptorPtr
NodeEntry::getServerDescriptor(const ServerInfo& server)
{
    assert(_session);
    try
    {
	NodeInfo info = _session->getInfo();

	Resolver resolve("server `" + server.descriptor->id + "'", map<string, string>());
	resolve.setReserved("application", server.application);
	resolve.setReserved("node", server.node);
	resolve.setReserved("server", server.descriptor->id);
	resolve.setReserved("node.os", info.os);
	resolve.setReserved("node.hostname", info.hostname);
	resolve.setReserved("node.release", info.release);
	resolve.setReserved("node.version", info.version);
	resolve.setReserved("node.machine", info.machine);
	resolve.setReserved("node.datadir", info.dataDir);
	
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(server.descriptor);
	if(iceBox)
	{
	    return IceBoxHelper(_cache.getCommunicator(), iceBox).instantiate(resolve, PropertySetDescriptor());
	}
	else
	{
	    return ServerHelper(_cache.getCommunicator(), server.descriptor).instantiate(resolve, 
	    										 PropertySetDescriptor());
	}
    }
    catch(const DeploymentException& ex)
    {
	Ice::Warning out(_cache.getTraceLevels()->logger);
	out << "couldn't instantiate `" + server.descriptor->id + "':\n" << ex.reason;
	return server.descriptor;
    }
}
