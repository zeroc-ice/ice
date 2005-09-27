// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

using namespace std;
using namespace IceGrid;

NodeCache::NodeCache(int sessionTimeout) : _sessionTimeout(sessionTimeout)
{
}

NodeEntryPtr
NodeCache::get(const string& name, bool create) const
{
    Lock sync(*this);
    NodeCache& self = const_cast<NodeCache&>(*this);
    NodeEntryPtr entry = self.getImpl(name, create);
    if(!entry)
    {
	NodeNotExistException ex;
	ex.name = name;
	throw ex;
    }
    return entry;
}

NodeEntry::NodeEntry(Cache<string, NodeEntry>& cache, const std::string& name) : 
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
    if(remove)
    {
	_cache.remove(_name);
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
