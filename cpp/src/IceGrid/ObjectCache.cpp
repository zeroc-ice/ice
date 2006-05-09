// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/IdentityUtil.h>
#include <Ice/LoggerUtil.h>
#include <IceGrid/ObjectCache.h>
#include <IceGrid/NodeSessionI.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

pointer_to_unary_function<int, int> ObjectCache::_rand(IceUtil::random);

namespace IceGrid
{

struct ObjectEntryCI : binary_function<ObjectEntryPtr&, ObjectEntryPtr&, bool>
{

    bool
    operator()(const ObjectEntryPtr& lhs, const ObjectEntryPtr& rhs)
    {
	return ::Ice::proxyIdentityLess(lhs->getProxy(), rhs->getProxy());
    }
};

struct ObjectLoadCI : binary_function<pair<Ice::ObjectPrx, float>&, pair<Ice::ObjectPrx, float>&, bool>
{
    bool operator()(const pair<Ice::ObjectPrx, float>& lhs, const pair<Ice::ObjectPrx, float>& rhs)
    {
	return lhs.second < rhs.second;
    }
};

};

ObjectCache::TypeEntry::TypeEntry() : _allocatablesCount(0)
{
}

void
ObjectCache::TypeEntry::add(const ObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    _objects.insert(lower_bound(_objects.begin(), _objects.end(), obj, ObjectEntryCI()), obj);
    _allocatablesCount += obj->isAllocatable() ? 1 : 0;
}

bool
ObjectCache::TypeEntry::remove(const ObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    _allocatablesCount -= obj->isAllocatable() ? 1 : 0;
    vector<ObjectEntryPtr>::iterator q = lower_bound(_objects.begin(), _objects.end(), obj, ObjectEntryCI());
    assert(q->get() == obj.get());
    _objects.erase(q);
    return _objects.empty();
}

void
ObjectCache::TypeEntry::addAllocationRequest(const ObjectAllocationRequestPtr& request)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    if(request->pending())
    {
	_requests.push_back(request);
    }
}

bool
ObjectCache::TypeEntry::canTryAllocate(const ObjectEntryPtr& entry)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    list<ObjectAllocationRequestPtr>::iterator p = _requests.begin();
    while(p != _requests.end())
    {
	AllocationRequestPtr request = *p;
	try
	{
	    if(request->isCanceled()) // If the request has been canceled, we just remove it.
	    {
		p = _requests.erase(p);
	    }
	    else if(entry->tryAllocate(request, true))
	    {
		p = _requests.erase(p);
		return true; // The request successfully allocated the entry!
	    }
	    else if(entry->getSession()) // If entry is allocated, we're done
	    {
		return false;
	    }
	    else
	    {
		++p;
	    }
	}
	catch(const AllocationException&)
	{
	    p = _requests.erase(p);
	}
    }
    return false;
}

bool
ObjectCache::TypeEntry::hasAllocatables() const
{
    return _allocatablesCount;
}

ObjectCache::ObjectCache(AdapterCache& adapterCache) : 
    _adapterCache(adapterCache)
{
}

void
ObjectCache::add(const ObjectInfo& info, const string& application, bool allocatable, const AllocatablePtr& parent)
{
    const Ice::Identity& id = info.proxy->ice_getIdentity();

    Lock sync(*this);
    assert(!getImpl(id));

    ObjectEntryPtr entry = new ObjectEntry(*this, info, application, allocatable, parent);
    addImpl(id, entry);

    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    if(p == _types.end())
    {
	p = _types.insert(p, make_pair(entry->getType(), TypeEntry()));
    }
    p->second.add(entry);

    if(_traceLevels && _traceLevels->object > 0)
    {
	Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
	out << "added object `" << Ice::identityToString(id) << "'";	
    }    
}

ObjectEntryPtr
ObjectCache::get(const Ice::Identity& id) const
{
    Lock sync(*this);
    ObjectEntryPtr entry = getImpl(id);
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

    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    assert(p != _types.end());
    if(p->second.remove(entry))
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

void
ObjectCache::allocateByType(const string& type, const ObjectAllocationRequestPtr& request)
{
    Lock sync(*this);
    map<string, TypeEntry>::iterator p = _types.find(type);
    if(p == _types.end() || !p->second.hasAllocatables())
    {
	throw AllocationException("no allocatable objects with type `" + type + "' registered");
    }

    vector<ObjectEntryPtr> objects = p->second.getObjects();
    random_shuffle(objects.begin(), objects.end(), _rand); // TODO: OPTIMIZE
    try
    {
	for(vector<ObjectEntryPtr>::const_iterator q = objects.begin(); q != objects.end(); ++q)
	{
	    if((*q)->tryAllocate(request))
	    {
		return;
	    }
	}
    }
    catch(const AllocationException&)
    {
	return;
    }

    p->second.addAllocationRequest(request);
}

bool
ObjectCache::canTryAllocate(const ObjectEntryPtr& entry)
{
    //
    // Notify the type entry that an object was released.
    //
    Lock sync(*this);
    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    if(p == _types.end())
    {
	return false;
    }
    return p->second.canTryAllocate(entry);
}

Ice::ObjectProxySeq
ObjectCache::getObjectsByType(const string& type)
{
    Lock sync(*this);
    Ice::ObjectProxySeq proxies;
    map<string, TypeEntry>::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
	return proxies;
    }
    const vector<ObjectEntryPtr>& objects = p->second.getObjects();
    for(vector<ObjectEntryPtr>::const_iterator q = objects.begin(); q != objects.end(); ++q)
    {
	if((*q)->isAllocatable()) // Only return non-allocatable objects.
	{
	    proxies.push_back((*q)->getProxy());
	}
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

ObjectInfoSeq
ObjectCache::getAllByType(const string& type)
{
    Lock sync(*this);
    ObjectInfoSeq infos;
    map<string, TypeEntry>::const_iterator p = _types.find(type);
    if(p == _types.end())
    {
	return infos;
    }

    const vector<ObjectEntryPtr>& objects = p->second.getObjects();
    for(vector<ObjectEntryPtr>::const_iterator q = objects.begin(); q != objects.end(); ++q)
    {
	infos.push_back((*q)->getObjectInfo());
    }
    return infos;
}

ObjectEntry::ObjectEntry(ObjectCache& cache, 
			 const ObjectInfo& info, 
			 const string& application, 
			 bool allocatable,
			 const AllocatablePtr& parent) :
    Allocatable(allocatable, parent),
    _cache(cache),
    _info(info),
    _application(application)
{
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

void
ObjectEntry::allocated(const SessionIPtr& session)
{
    //
    // Add the object allocation to the session. The object will be
    // released once the session is destroyed.
    //
    if(!session->addAllocation(this))
    {
	throw AllocationException("session destroyed");
    }

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->object > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
	const Ice::Identity id = _info.proxy->ice_getIdentity();
	out << "object `" << id << "' allocated by `" << session->getUserId() << "' (" << _count << ")";
    }    
}

void
ObjectEntry::released(const SessionIPtr& session)
{
    //
    // Remove the object allocation from the session.
    //
    session->removeAllocation(this);

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->object > 1)
    {
	Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
	const Ice::Identity id = _info.proxy->ice_getIdentity();
	out << "object `" << id << "' released by `" << session->getUserId() << "' (" << _count << ")";
    }    
}

bool
ObjectEntry::canTryAllocate()
{
    return _cache.canTryAllocate(this);
}
