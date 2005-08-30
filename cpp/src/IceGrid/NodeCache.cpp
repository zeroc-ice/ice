// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Functional.h>

#include <IceGrid/NodeCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>

using namespace std;
using namespace IceGrid;

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
	remove = _servers.empty() && !_session;
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
	remove = _servers.empty() && !_session;
    }
    if(remove)
    {
	_cache.remove(_name);
    }
    
    if(session)
    {
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
}

NodePrx
NodeEntry::getProxy() const
{
    Lock sync(*this);
    if(!_session)
    {
	throw NodeUnreachableException();
    }
    return _session->getNode();
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

bool
NodeEntry::canRemove()
{
    Lock sync(*this);
    return !_session && _servers.empty();
}
