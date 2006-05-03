// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_OBJECTCACHE_H
#define ICE_GRID_OBJECTCACHE_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Shared.h>
#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Internal.h>
#include <IceGrid/Query.h>
#include <IceGrid/Allocatable.h>

namespace IceGrid
{

class ObjectCache;

class ServerEntry;
typedef IceUtil::Handle<ServerEntry> ServerEntryPtr;

class ObjectEntry : public Allocatable
{
public:
    
    ObjectEntry(ObjectCache&, const ObjectInfo&, const std::string&, bool, const AllocatablePtr&);
    Ice::ObjectPrx getProxy(const SessionIPtr&) const;
    std::string getType() const;
    std::string getApplication() const;
    const ObjectInfo& getObjectInfo() const;

    bool canRemove();

    virtual bool release(const SessionIPtr&);
    virtual bool allocated(const SessionIPtr&);
    virtual void released(const SessionIPtr&);
    
private:

    ObjectCache& _cache;
    const ObjectInfo _info;
    const std::string _application;
};
typedef IceUtil::Handle<ObjectEntry> ObjectEntryPtr;

class ObjectAllocationRequest : public AllocationRequest
{
public:

    ObjectAllocationRequest(const SessionIPtr& session) : AllocationRequest(session) { }
    
    virtual void response(const Ice::ObjectPrx&) = 0;
    virtual void exception(const AllocationException&) = 0;

private:

    virtual bool allocated(const AllocatablePtr& allocatable, const SessionIPtr& session)
    {
	response(ObjectEntryPtr::dynamicCast(allocatable)->getObjectInfo().proxy);
	return true;
    }

    virtual void canceled(const AllocationException& ex)
    {
	exception(ex);
    }
};
typedef IceUtil::Handle<ObjectAllocationRequest> ObjectAllocationRequestPtr;

class AdapterCache;

class ObjectCache : public Cache<Ice::Identity, ObjectEntry>
{
public:

    ObjectCache(AdapterCache&);

    void add(const ObjectInfo&, const std::string&, bool, const AllocatablePtr&);
    ObjectEntryPtr get(const Ice::Identity&) const;
    ObjectEntryPtr remove(const Ice::Identity&);

    void allocateByType(const std::string&, const ObjectAllocationRequestPtr&);
    void allocateByTypeOnLeastLoadedNode(const std::string&, const ObjectAllocationRequestPtr&, LoadSample);
    void released(const ObjectEntryPtr&);

    Ice::ObjectProxySeq getObjectsByType(const std::string&); 
    ObjectInfoSeq getAll(const std::string&);
    ObjectInfoSeq getAllByType(const std::string&);

private:
    
    class TypeEntry
    {
    public:

	TypeEntry(ObjectCache&);


	void add(const Ice::ObjectPrx&);
	bool remove(const Ice::ObjectPrx&);
	
	void addAllocationRequest(const ObjectAllocationRequestPtr&);
	void released(const ObjectEntryPtr&);

	const Ice::ObjectProxySeq& getObjects() const { return _objects; }

    private:
	
	ObjectCache& _cache;
	Ice::ObjectProxySeq _objects;
	std::list<ObjectAllocationRequestPtr> _requests;
    };

    AdapterCache& _adapterCache;

    std::map<std::string, TypeEntry> _types;
    std::map<std::string, std::vector<Ice::Identity> > _allocatablesByType;

    static std::pointer_to_unary_function<int, int> _rand;
};

};

#endif
