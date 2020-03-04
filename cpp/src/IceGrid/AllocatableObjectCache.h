//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ALLOCATABLEOBJECTCACHE_H
#define ICE_GRID_ALLOCATABLEOBJECTCACHE_H

#include <Ice/CommunicatorF.h>
#include <IceGrid/Cache.h>
#include <IceGrid/Admin.h>
#include <IceGrid/Allocatable.h>

namespace IceGrid
{

class AllocatableObjectCache;
class ServerEntry;

class AllocatableObjectEntry : public Allocatable
{
public:

    AllocatableObjectEntry(AllocatableObjectCache&, const ObjectInfo&, const std::shared_ptr<ServerEntry>&);
    std::shared_ptr<Ice::ObjectPrx> getProxy() const;
    std::string getType() const;

    bool canRemove();

    bool isEnabled() const override;
    void allocated(const std::shared_ptr<SessionI>&) override;
    void released(const std::shared_ptr<SessionI>&) override;
    bool canTryAllocate() override;

    void  destroy();
    void checkAllocatable() override;

private:

    AllocatableObjectCache& _cache;
    const ObjectInfo _info;
    std::shared_ptr<ServerEntry> _server;
    bool _destroyed;
};

class ObjectAllocationRequest : public AllocationRequest
{
public:

    ObjectAllocationRequest(const std::shared_ptr<SessionI>& session) : AllocationRequest(session)
    {
    }

    virtual void response(const std::shared_ptr<Ice::ObjectPrx>&) = 0;
    virtual void exception(std::exception_ptr) = 0;

private:

    void allocated(const std::shared_ptr<Allocatable>& allocatable, const std::shared_ptr<SessionI>&) override
    {
        response(std::dynamic_pointer_cast<AllocatableObjectEntry>(allocatable)->getProxy());
    }

    void canceled(std::exception_ptr ex) override
    {
        exception(ex);
    }
};

class AllocatableObjectCache : public Cache<Ice::Identity, AllocatableObjectEntry>
{
public:

    AllocatableObjectCache(const std::shared_ptr<Ice::Communicator>&);

    void add(const ObjectInfo&, const std::shared_ptr<ServerEntry>&);
    std::shared_ptr<AllocatableObjectEntry> get(const Ice::Identity&) const;
    void remove(const Ice::Identity&);

    void allocateByType(const std::string&, const std::shared_ptr<ObjectAllocationRequest>&);
    bool canTryAllocate(const std::shared_ptr<AllocatableObjectEntry>&);

    const std::shared_ptr<Ice::Communicator>& getCommunicator() const { return _communicator; }

private:

    class TypeEntry
    {
    public:

        void add(const std::shared_ptr<AllocatableObjectEntry>&);
        bool remove(const std::shared_ptr<AllocatableObjectEntry>&);

        void addAllocationRequest(const std::shared_ptr<ObjectAllocationRequest>&);
        bool canTryAllocate(const std::shared_ptr<AllocatableObjectEntry>&, bool);

        const std::vector<std::shared_ptr<AllocatableObjectEntry>>& getObjects() const { return _objects; }

    private:

        std::vector<std::shared_ptr<AllocatableObjectEntry>> _objects;
        std::list<std::shared_ptr<ObjectAllocationRequest>> _requests;
    };

    const std::shared_ptr<Ice::Communicator> _communicator;
    std::map<std::string, TypeEntry> _types;
    std::map<std::string, std::vector<Ice::Identity> > _allocatablesByType;
};

};

#endif
