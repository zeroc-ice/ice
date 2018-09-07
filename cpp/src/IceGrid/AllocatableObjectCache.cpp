// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/Random.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <IceGrid/AllocatableObjectCache.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

struct AllocatableObjectEntryCI : binary_function<AllocatableObjectEntryPtr&, AllocatableObjectEntryPtr&, bool>
{

    bool
    operator()(const AllocatableObjectEntryPtr& lhs, const AllocatableObjectEntryPtr& rhs)
    {
        return ::Ice::proxyIdentityLess(lhs->getProxy(), rhs->getProxy());
    }
};

};

AllocatableObjectCache::TypeEntry::TypeEntry()
{
}

void
AllocatableObjectCache::TypeEntry::add(const AllocatableObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    _objects.insert(lower_bound(_objects.begin(), _objects.end(), obj, AllocatableObjectEntryCI()), obj);
    if(!_requests.empty())
    {
        canTryAllocate(obj, false);
    }
}

bool
AllocatableObjectCache::TypeEntry::remove(const AllocatableObjectEntryPtr& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    vector<AllocatableObjectEntryPtr>::iterator q;
    q = lower_bound(_objects.begin(), _objects.end(), obj, AllocatableObjectEntryCI());
    assert(q->get() == obj.get());
    _objects.erase(q);

    if(!_requests.empty() && _objects.empty())
    {
        for(list<ObjectAllocationRequestPtr>::const_iterator p = _requests.begin(); p != _requests.end(); ++p)
        {
            (*p)->cancel(AllocationException("no allocatable objects with type `" + obj->getType() + "' registered"));
        }
    }
    return _objects.empty();
}

void
AllocatableObjectCache::TypeEntry::addAllocationRequest(const ObjectAllocationRequestPtr& request)
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
AllocatableObjectCache::TypeEntry::canTryAllocate(const AllocatableObjectEntryPtr& entry, bool fromRelease)
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
            else if(entry->tryAllocate(request, fromRelease))
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
        catch(const SessionDestroyedException&)
        {
            p = _requests.erase(p);
        }
    }
    return false;
}

AllocatableObjectCache::AllocatableObjectCache(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
AllocatableObjectCache::add(const ObjectInfo& info, const ServerEntryPtr& parent)
{
    const Ice::Identity& id = info.proxy->ice_getIdentity();

    Lock sync(*this);
    if(getImpl(id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate allocatable object `" << _communicator->identityToString(id) << "'";
        return;
    }

    AllocatableObjectEntryPtr entry = new AllocatableObjectEntry(*this, info, parent);
    addImpl(id, entry);

    map<string, TypeEntry>::iterator p = _types.find(entry->getType());
    if(p == _types.end())
    {
        p = _types.insert(p, map<string, TypeEntry>::value_type(entry->getType(), TypeEntry()));
    }
    p->second.add(entry);

    if(_traceLevels && _traceLevels->object > 0)
    {
        Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
        out << "added allocatable object `" << _communicator->identityToString(id) << "'";
    }
}

AllocatableObjectEntryPtr
AllocatableObjectCache::get(const Ice::Identity& id) const
{
    Lock sync(*this);
    AllocatableObjectEntryPtr entry = getImpl(id);
    if(!entry)
    {
        throw ObjectNotRegisteredException(id);
    }
    return entry;
}

void
AllocatableObjectCache::remove(const Ice::Identity& id)
{
    AllocatableObjectEntryPtr entry;
    {
        Lock sync(*this);
        entry = getImpl(id);
        if(!entry)
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't remove unknown object `" << _communicator->identityToString(id) << "'";
        }
        removeImpl(id);

        map<string, TypeEntry>::iterator p = _types.find(entry->getType());
        assert(p != _types.end());
        if(p->second.remove(entry))
        {
            _types.erase(p);
        }

        if(_traceLevels && _traceLevels->object > 0)
        {
            Ice::Trace out(_traceLevels->logger, _traceLevels->objectCat);
            out << "removed allocatable object `" << _communicator->identityToString(id) << "'";
        }
    }

    //
    // This must be done outside the synchronization because destroy
    // might release the object and release might try to callback on
    // the cache.
    //
    assert(entry);
    entry->destroy();
}

void
AllocatableObjectCache::allocateByType(const string& type, const ObjectAllocationRequestPtr& request)
{
    Lock sync(*this);
    map<string, TypeEntry>::iterator p = _types.find(type);
    if(p == _types.end())
    {
        throw AllocationException("no allocatable objects with type `" + type + "' registered");
    }

    vector<AllocatableObjectEntryPtr> objects = p->second.getObjects();
    IceUtilInternal::shuffle(objects.begin(), objects.end()); // TODO: OPTIMIZE
    int allocatable = 0;
    try
    {
        for(vector<AllocatableObjectEntryPtr>::const_iterator q = objects.begin(); q != objects.end(); ++q)
        {
            if((*q)->isEnabled())
            {
                ++allocatable;
                if((*q)->tryAllocate(request))
                {
                    return;
                }
            }
        }
    }
    catch(const SessionDestroyedException&)
    {
        return; // The request has been answered already, no need to throw here.
    }
    if(allocatable == 0)
    {
        throw AllocationException("no allocatable objects with type `" + type + "' enabled");
    }
    p->second.addAllocationRequest(request);
}

bool
AllocatableObjectCache::canTryAllocate(const AllocatableObjectEntryPtr& entry)
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
    return p->second.canTryAllocate(entry, true);
}

AllocatableObjectEntry::AllocatableObjectEntry(AllocatableObjectCache& cache,
                                               const ObjectInfo& info,
                                               const ServerEntryPtr& parent) :
    Allocatable(true, parent),
    _cache(cache),
    _info(info),
    _server(parent),
    _destroyed(false)
{
    assert(_server);
}

Ice::ObjectPrx
AllocatableObjectEntry::getProxy() const
{
    return _info.proxy;
}

string
AllocatableObjectEntry::getType() const
{
    return _info.type;
}

bool
AllocatableObjectEntry::canRemove()
{
    return true;
}

bool
AllocatableObjectEntry::isEnabled() const
{
    return _server->isEnabled();
}

void
AllocatableObjectEntry::allocated(const SessionIPtr& session)
{
    //
    // Add the object allocation to the session. The object will be
    // released once the session is destroyed.
    //
    session->addAllocation(this);

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->object > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
        out << "object `" << _info.proxy->ice_toString() << "' allocated by `" << session->getId() << "' (" << _count
            << ")";
    }

    Glacier2::IdentitySetPrx identities = session->getGlacier2IdentitySet();
    if(identities)
    {
        try
        {
            Ice::IdentitySeq seq(1);
            seq.push_back(_info.proxy->ice_getIdentity());
            identities->add(seq);
        }
        catch(const Ice::LocalException& ex)
        {
            if(traceLevels && traceLevels->object > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
                out << "couldn't add Glacier2 filter for object `" << _info.proxy->ice_toString();
                out << "' allocated by `" << session->getId() << "':\n" << ex;
            }
        }
    }
}

void
AllocatableObjectEntry::released(const SessionIPtr& session)
{
    //
    // Remove the object allocation from the session.
    //
    session->removeAllocation(this);

    TraceLevelsPtr traceLevels = _cache.getTraceLevels();

    Glacier2::IdentitySetPrx identities = session->getGlacier2IdentitySet();
    if(identities)
    {
        try
        {
            Ice::IdentitySeq seq(1);
            seq.push_back(_info.proxy->ice_getIdentity());
            identities->remove(seq);
        }
        catch(const Ice::LocalException& ex)
        {
            if(traceLevels && traceLevels->object > 0)
            {
                Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
                out << "couldn't remove Glacier2 filter for object `" << _info.proxy->ice_toString();
                out << "' allocated by `" << session->getId() << "':\n" << ex;
            }
        }
    }

    if(traceLevels && traceLevels->object > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
        out << "object `" << _info.proxy->ice_toString() << "' released by `" << session->getId() << "' (" << _count
            << ")";
    }
}

void
AllocatableObjectEntry::destroy()
{
    SessionIPtr session;
    {
        Lock sync(*this);
        _destroyed = true;
        session = _session;
    }
    if(session)
    {
        try
        {
            release(session);
        }
        catch(const AllocationException&)
        {
        }
    }
}

void
AllocatableObjectEntry::checkAllocatable()
{
    if(_destroyed)
    {
        throw ObjectNotRegisteredException(_info.proxy->ice_getIdentity());
    }

    Allocatable::checkAllocatable();
}

bool
AllocatableObjectEntry::canTryAllocate()
{
    return _cache.canTryAllocate(this);
}
