// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/IdentityUtil.h>
#include <Ice/LoggerUtil.h>

#include <IceGrid/ObjectCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>

using namespace std;
using namespace IceGrid;

ObjectCache::ObjectCache(const Ice::CommunicatorPtr& communicator) : 
    _communicator(communicator)
{
}

void
ObjectCache::add(const string& app, const string& adapterId, const string& endpoints, const ObjectDescriptor& desc)
{
    Lock sync(*this);
    assert(!getImpl(desc.id));

    ObjectEntryPtr entry = getImpl(desc.id, true);

    ObjectInfo info;
    info.type = desc.type;
    if(adapterId.empty())
    {
	info.proxy = _communicator->stringToProxy(Ice::identityToString(desc.id) + ":" + endpoints);
    }
    else
    {
	info.proxy = _communicator->stringToProxy(Ice::identityToString(desc.id) + "@" + adapterId);
    }
    entry->set(app, info);

    map<string, set<Ice::Identity> >::iterator p = _types.find(entry->getType());
    if(p == _types.end())
    {
	p = _types.insert(p, map<string, set<Ice::Identity> >::value_type(entry->getType(), set<Ice::Identity>()));
    }
    p->second.insert(desc.id);

    if(_traceLevels && _traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "added object `" << Ice::identityToString(desc.id) << "'";	
    }    
}

ObjectEntryPtr
ObjectCache::get(const Ice::Identity& id) const
{
    Lock sync(*this);
    ObjectCache& self = const_cast<ObjectCache&>(*this);
    ObjectEntryPtr entry = self.getImpl(id);
    if(!entry)
    {
	throw ObjectNotRegisteredException(id);
    }
    return entry;
}

ObjectEntryPtr
ObjectCache::remove(const Ice::Identity& id)
{
    Lock sync(*this);
    ObjectEntryPtr entry = removeImpl(id);
    assert(entry);

    map<string, set<Ice::Identity> >::iterator p = _types.find(entry->getType());
    assert(p != _types.end());
    p->second.erase(id);
    if(p->second.empty())
    {
	_types.erase(p);
    }

    if(_traceLevels && _traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "removed object `" << Ice::identityToString(id) << "'";	
    }    

    return entry;
}

Ice::ObjectProxySeq
ObjectCache::getObjectsByType(const string& type)
{
    Lock sync(*this);
    Ice::ObjectProxySeq proxies;
    map<string, set<Ice::Identity> >::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
	return proxies;
    }
    for(set<Ice::Identity>::const_iterator q = p->second.begin(); q != p->second.end(); ++q)
    {
	proxies.push_back(getImpl(*q)->getProxy());
    }
    return proxies;
}

ObjectInfoSeq
ObjectCache::getAll(const string& expression)
{
    Lock sync(*this);
    ObjectInfoSeq infos;
    for(map<Ice::Identity, ObjectEntryPtr>::const_iterator p = _entries.begin(); p != _entries.end(); ++p)
    {
	if(expression.empty() || IceUtil::match(Ice::identityToString(p->first), expression, true))
	{
	    infos.push_back(p->second->getObjectInfo());
	}
    }
    return infos;
}

ObjectEntry::ObjectEntry(Cache<Ice::Identity, ObjectEntry>&, const Ice::Identity&)
{
}

void
ObjectEntry::set(const string& app, const ObjectInfo& info)
{
    _application = app;
    _info = info;
}

Ice::ObjectPrx
ObjectEntry::getProxy() const
{
    return _info.proxy;
}

string
ObjectEntry::getType() const
{
    return _info.type;
}

string
ObjectEntry::getApplication() const
{
    return _application;
}

const ObjectInfo&
ObjectEntry::getObjectInfo() const
{
    return _info;
}

bool
ObjectEntry::canRemove()
{
    return true;
}
