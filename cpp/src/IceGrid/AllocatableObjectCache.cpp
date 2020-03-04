//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <IceUtil/Random.h>
#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <IceGrid/AllocatableObjectCache.h>
#include <IceGrid/ServerCache.h>
#include <IceGrid/SessionI.h>

using namespace std;
using namespace IceGrid;

namespace
{

bool compareAllocatableObjectEntry(const shared_ptr<AllocatableObjectEntry>& lhs,
                                   const shared_ptr<AllocatableObjectEntry>& rhs)
{
    return Ice::proxyIdentityLess(lhs->getProxy(), rhs->getProxy());
}

};

void
AllocatableObjectCache::TypeEntry::add(const shared_ptr<AllocatableObjectEntry>& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    _objects.insert(lower_bound(_objects.begin(), _objects.end(), obj, compareAllocatableObjectEntry), obj);
    if(!_requests.empty())
    {
        canTryAllocate(obj, false);
    }
}

bool
AllocatableObjectCache::TypeEntry::remove(const shared_ptr<AllocatableObjectEntry>& obj)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    vector<shared_ptr<AllocatableObjectEntry>>::iterator q;
    q = lower_bound(_objects.begin(), _objects.end(), obj, compareAllocatableObjectEntry);
    assert(q->get() == obj.get());
    _objects.erase(q);

    if(!_requests.empty() && _objects.empty())
    {
        for(const auto& req : _requests)
        {
            req->cancel(make_exception_ptr(AllocationException("no allocatable objects with type `" + obj->getType() + "' registered")));
        }
    }
    return _objects.empty();
}

void
AllocatableObjectCache::TypeEntry::addAllocationRequest(const shared_ptr<ObjectAllocationRequest>& request)
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
AllocatableObjectCache::TypeEntry::canTryAllocate(const shared_ptr<AllocatableObjectEntry>& entry, bool fromRelease)
{
    //
    // No mutex protection here, this is called with the cache locked.
    //
    auto p = _requests.begin();
    while(p != _requests.end())
    {
        auto request = *p;
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

AllocatableObjectCache::AllocatableObjectCache(const shared_ptr<Ice::Communicator>& communicator) :
    _communicator(communicator)
{
}

void
AllocatableObjectCache::add(const ObjectInfo& info, const shared_ptr<ServerEntry>& parent)
{
    auto id = info.proxy->ice_getIdentity();

    lock_guard lock(_mutex);

    if(getImpl(id))
    {
        Ice::Error out(_communicator->getLogger());
        out << "can't add duplicate allocatable object `" << _communicator->identityToString(id) << "'";
        return;
    }

    auto entry = make_shared<AllocatableObjectEntry>(*this, info, parent);
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

shared_ptr<AllocatableObjectEntry>
AllocatableObjectCache::get(const Ice::Identity& id) const
{
    lock_guard lock(_mutex);

    auto entry = getImpl(id);
    if(!entry)
    {
        throw ObjectNotRegisteredException(id);
    }
    return entry;
}

void
AllocatableObjectCache::remove(const Ice::Identity& id)
{
    shared_ptr<AllocatableObjectEntry> entry;
    {
        lock_guard lock(_mutex);

        entry = getImpl(id);
        if(!entry)
        {
            Ice::Error out(_communicator->getLogger());
            out << "can't remove unknown object `" << _communicator->identityToString(id) << "'";
        }
        removeImpl(id);

        auto p = _types.find(entry->getType());
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
AllocatableObjectCache::allocateByType(const string& type, const shared_ptr<ObjectAllocationRequest>& request)
{
    lock_guard lock(_mutex);

    auto p = _types.find(type);
    if(p == _types.end())
    {
        throw AllocationException("no allocatable objects with type `" + type + "' registered");
    }

    vector<shared_ptr<AllocatableObjectEntry>> objects = p->second.getObjects();
    IceUtilInternal::shuffle(objects.begin(), objects.end()); // TODO: OPTIMIZE
    int allocatable = 0;
    try
    {
        for(const auto& obj : objects)
        {
            if(obj->isEnabled())
            {
                ++allocatable;
                if(obj->tryAllocate(request))
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
AllocatableObjectCache::canTryAllocate(const shared_ptr<AllocatableObjectEntry>& entry)
{
    //
    // Notify the type entry that an object was released.
    //
    lock_guard lock(_mutex);
    auto p = _types.find(entry->getType());
    if(p == _types.end())
    {
        return false;
    }
    return p->second.canTryAllocate(entry, true);
}

AllocatableObjectEntry::AllocatableObjectEntry(AllocatableObjectCache& cache,
                                               const ObjectInfo& info,
                                               const shared_ptr<ServerEntry>& parent) :
    Allocatable(true, parent),
    _cache(cache),
    _info(info),
    _server(parent),
    _destroyed(false)
{
    assert(_server);
}

shared_ptr<Ice::ObjectPrx>
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
AllocatableObjectEntry::allocated(const shared_ptr<SessionI>& session)
{
    //
    // Add the object allocation to the session. The object will be
    // released once the session is destroyed.
    //
    session->addAllocation(shared_from_this());

    auto traceLevels = _cache.getTraceLevels();
    if(traceLevels && traceLevels->object > 1)
    {
        Ice::Trace out(traceLevels->logger, traceLevels->objectCat);
        out << "object `" << _info.proxy->ice_toString() << "' allocated by `" << session->getId() << "' (" << _count
            << ")";
    }

    auto identities = session->getGlacier2IdentitySet();
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
AllocatableObjectEntry::released(const shared_ptr<SessionI>& session)
{
    //
    // Remove the object allocation from the session.
    //
    session->removeAllocation(shared_from_this());

    auto traceLevels = _cache.getTraceLevels();

    auto identities = session->getGlacier2IdentitySet();
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
    shared_ptr<SessionI> session;
    {
        lock_guard lock(_mutex);
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
    return _cache.canTryAllocate(static_pointer_cast<AllocatableObjectEntry>(shared_from_this()));
}
