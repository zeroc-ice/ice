// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ALLOCATABLEOBJECTCACHE_H
#define ICE_GRID_ALLOCATABLEOBJECTCACHE_H

#include <IceUtil/Mutex.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Allocatable.h>

namespace IceGrid
{

class AllocatableObjectCache;

class AllocatableObjectEntry : public Allocatable
{
public:

    AllocatableObjectEntry(AllocatableObjectCache&, const ObjectInfo&, const AllocatablePtr&);
    Ice::ObjectPrx getProxy() const;
    std::string getType() const;

    bool canRemove();

    virtual void allocated(const SessionIPtr&);
    virtual void released(const SessionIPtr&);
    virtual bool canTryAllocate();

    void  destroy();
    virtual void checkAllocatable();

private:

    AllocatableObjectCache& _cache;
    const ObjectInfo _info;
    bool _destroyed;
};
typedef IceUtil::Handle<AllocatableObjectEntry> AllocatableObjectEntryPtr;

class ObjectAllocationRequest : public AllocationRequest
{
public:

    ObjectAllocationRequest(const SessionIPtr& session) : AllocationRequest(session) { }

    virtual void response(const Ice::ObjectPrx&) = 0;
    virtual void exception(const Ice::UserException&) = 0;

private:

    virtual void allocated(const AllocatablePtr& allocatable, const SessionIPtr& /*session*/)
    {
        response(AllocatableObjectEntryPtr::dynamicCast(allocatable)->getProxy());
    }

    virtual void canceled(const Ice::UserException& ex)
    {
        exception(ex);
    }
};
typedef IceUtil::Handle<ObjectAllocationRequest> ObjectAllocationRequestPtr;

class AdapterCache;

class AllocatableObjectCache : public Cache<Ice::Identity, AllocatableObjectEntry>
{
public:

    AllocatableObjectCache(const Ice::CommunicatorPtr&);

    void add(const ObjectInfo&, const AllocatablePtr&);
    AllocatableObjectEntryPtr get(const Ice::Identity&) const;
    void remove(const Ice::Identity&);

    void allocateByType(const std::string&, const ObjectAllocationRequestPtr&);
    bool canTryAllocate(const AllocatableObjectEntryPtr&);

    const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }

private:

    class TypeEntry
    {
    public:

        TypeEntry();

        void add(const AllocatableObjectEntryPtr&);
        bool remove(const AllocatableObjectEntryPtr&);

        void addAllocationRequest(const ObjectAllocationRequestPtr&);
        bool canTryAllocate(const AllocatableObjectEntryPtr&, bool);

        const std::vector<AllocatableObjectEntryPtr>& getObjects() const { return _objects; }

    private:

        std::vector<AllocatableObjectEntryPtr> _objects;
        std::list<ObjectAllocationRequestPtr> _requests;
    };

    const Ice::CommunicatorPtr _communicator;
    std::map<std::string, TypeEntry> _types;
    std::map<std::string, std::vector<Ice::Identity> > _allocatablesByType;
};

};

#endif
