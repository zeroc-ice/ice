// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_ALLOCATABLEOBJECTCACHE_H
#define ICEGRID_ALLOCATABLEOBJECTCACHE_H

#include "Allocatable.h"
#include "Cache.h"
#include "Ice/CommunicatorF.h"
#include "IceGrid/Admin.h"

namespace IceGrid
{
    class AllocatableObjectCache;
    class ServerEntry;

    class AllocatableObjectEntry : public Allocatable
    {
    public:
        AllocatableObjectEntry(AllocatableObjectCache&, ObjectInfo, const std::shared_ptr<ServerEntry>&);
        [[nodiscard]] Ice::ObjectPrx getProxy() const;
        [[nodiscard]] std::string getType() const;

        bool canRemove();

        [[nodiscard]] bool isEnabled() const override;
        void allocated(const std::shared_ptr<SessionI>&) override;
        void released(const std::shared_ptr<SessionI>&) override;
        bool canTryAllocate() override;

        void destroy();
        void checkAllocatable() override;

    private:
        AllocatableObjectCache& _cache;
        const ObjectInfo _info;
        std::shared_ptr<ServerEntry> _server;
        bool _destroyed{false};
    };

    class ObjectAllocationRequest : public AllocationRequest
    {
    public:
        ObjectAllocationRequest(const std::shared_ptr<SessionI>& session) : AllocationRequest(session) {}

        virtual void response(const Ice::ObjectPrx&) = 0;
        virtual void exception(std::exception_ptr) = 0;

    private:
        void allocated(const std::shared_ptr<Allocatable>& allocatable, const std::shared_ptr<SessionI>&) override
        {
            response(std::dynamic_pointer_cast<AllocatableObjectEntry>(allocatable)->getProxy());
        }

        void canceled(std::exception_ptr ex) override { exception(ex); }
    };

    class AllocatableObjectCache : public Cache<Ice::Identity, AllocatableObjectEntry>
    {
    public:
        AllocatableObjectCache(const Ice::CommunicatorPtr&);

        void add(const ObjectInfo&, const std::shared_ptr<ServerEntry>&);
        [[nodiscard]] std::shared_ptr<AllocatableObjectEntry> get(const Ice::Identity&) const;
        void remove(const Ice::Identity&);

        void allocateByType(const std::string&, const std::shared_ptr<ObjectAllocationRequest>&);
        bool canTryAllocate(const std::shared_ptr<AllocatableObjectEntry>&);

        [[nodiscard]] const Ice::CommunicatorPtr& getCommunicator() const { return _communicator; }

    private:
        class TypeEntry
        {
        public:
            void add(const std::shared_ptr<AllocatableObjectEntry>&);
            bool remove(const std::shared_ptr<AllocatableObjectEntry>&);

            void addAllocationRequest(const std::shared_ptr<ObjectAllocationRequest>&);
            bool canTryAllocate(const std::shared_ptr<AllocatableObjectEntry>&, bool);

            [[nodiscard]] const std::vector<std::shared_ptr<AllocatableObjectEntry>>& getObjects() const
            {
                return _objects;
            }

        private:
            std::vector<std::shared_ptr<AllocatableObjectEntry>> _objects;
            std::list<std::shared_ptr<ObjectAllocationRequest>> _requests;
        };

        const Ice::CommunicatorPtr _communicator;
        std::map<std::string, TypeEntry> _types;
        std::map<std::string, std::vector<Ice::Identity>> _allocatablesByType;
    };

};

#endif
