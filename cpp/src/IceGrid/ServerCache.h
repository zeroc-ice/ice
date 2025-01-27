// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_SERVERCACHE_H
#define ICEGRID_SERVERCACHE_H

#include "Allocatable.h"
#include "Cache.h"
#include "IceGrid/Descriptor.h"
#include "IceGrid/Registry.h"
#include "Internal.h"

namespace IceGrid
{
    class AdapterCache;
    class AllocatableObjectCache;
    class CheckServerResult;
    class NodeCache;
    class NodeEntry;
    class NodeObserverTopic;
    class ObjectCache;
    class ServerCache;

    class CheckUpdateResult final
    {
    public:
        CheckUpdateResult(std::string, std::string, bool, bool, std::future<bool>&&);

        bool getResult();

        const std::string& getServer() { return _server; }

    private:
        const std::string _server;
        const std::string _node;
        const bool _remove;
        const bool _noRestart;
        std::future<bool> _result;
    };

    class ServerEntry final : public Allocatable
    {
    public:
        ServerEntry(ServerCache&, std::string);

        void sync();
        void waitForSync(std::chrono::seconds);
        void waitForSync() { waitForSync(std::chrono::seconds(-1)); }
        void waitForSyncNoThrow(std::chrono::seconds);
        void waitForSyncNoThrow() { waitForSyncNoThrow(std::chrono::seconds(-1)); }
        void unsync();

        bool addSyncCallback(const std::shared_ptr<SynchronizationCallback>&);

        void update(const ServerInfo&, bool);

        void destroy(bool);

        [[nodiscard]] ServerInfo getInfo(bool = false) const;
        [[nodiscard]] std::string getId() const;

        ServerPrx getProxy(
            std::chrono::seconds&,
            std::chrono::seconds&,
            std::string&,
            bool = true,
            std::chrono::seconds = std::chrono::seconds(0));
        ServerPrx getProxy(bool = true, std::chrono::seconds = std::chrono::seconds(0));
        Ice::ObjectPrx getAdminProxy();

        AdapterPrx getAdapter(const std::string&, bool);
        AdapterPrx getAdapter(std::chrono::seconds&, std::chrono::seconds&, const std::string&, bool);
        [[nodiscard]] float getLoad(LoadSample) const;

        bool canRemove();
        std::shared_ptr<CheckUpdateResult> checkUpdate(const ServerInfo&, bool);
        bool isDestroyed();

        void loadCallback(ServerPrx, const AdapterPrxDict&, std::chrono::seconds, std::chrono::seconds);
        void destroyCallback();
        void exception(std::exception_ptr);

        [[nodiscard]] bool isEnabled() const override;
        void allocated(const std::shared_ptr<SessionI>&) override;
        void allocatedNoSync(const std::shared_ptr<SessionI>&) override;
        void released(const std::shared_ptr<SessionI>&) override;
        void releasedNoSync(const std::shared_ptr<SessionI>&) override;

    private:
        void syncImpl();
        void waitImpl(std::chrono::seconds);
        void synchronized();
        void synchronized(std::exception_ptr);

        ServerCache& _cache;
        const std::string _id;
        std::unique_ptr<ServerInfo> _loaded;
        std::unique_ptr<ServerInfo> _load;
        std::unique_ptr<ServerInfo> _destroy;

        std::optional<ServerPrx> _proxy;
        AdapterPrxDict _adapters;
        std::chrono::seconds _activationTimeout;
        std::chrono::seconds _deactivationTimeout;

        bool _synchronizing{false};
        bool _updated{false};
        std::exception_ptr _exception;
        bool _noRestart{false};
        std::vector<std::shared_ptr<SynchronizationCallback>> _callbacks;

        std::shared_ptr<SessionI> _allocationSession;

        mutable std::mutex _mutex;
        std::condition_variable _condVar;
    };
    using ServerEntrySeq = std::vector<std::shared_ptr<ServerEntry>>;

    class ServerCache : public CacheByString<ServerEntry>
    {
    public:
        ServerCache(
            const Ice::CommunicatorPtr&,
            std::string,
            NodeCache&,
            AdapterCache&,
            ObjectCache&,
            AllocatableObjectCache&);

        std::shared_ptr<ServerEntry> add(const ServerInfo&);
        [[nodiscard]] std::shared_ptr<ServerEntry> get(const std::string&) const;
        [[nodiscard]] bool has(const std::string&) const;
        std::shared_ptr<ServerEntry> remove(const std::string&, bool);

        void preUpdate(const ServerInfo&, bool);
        std::shared_ptr<ServerEntry> postUpdate(const ServerInfo&, bool);

        void clear(const std::string&);

        [[nodiscard]] NodeCache& getNodeCache() const { return _nodeCache; }
        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const { return _communicator; }
        [[nodiscard]] const std::string& getInstanceName() const { return _instanceName; }

        [[nodiscard]] const std::shared_ptr<NodeObserverTopic>& getNodeObserverTopic() const
        {
            return _nodeObserverTopic;
        }
        void setNodeObserverTopic(const std::shared_ptr<NodeObserverTopic>&);

    private:
        void addCommunicator(
            const std::shared_ptr<CommunicatorDescriptor>&,
            const std::shared_ptr<CommunicatorDescriptor>&,
            const std::shared_ptr<ServerEntry>&,
            const std::string&);
        void removeCommunicator(
            const std::shared_ptr<CommunicatorDescriptor>&,
            const std::shared_ptr<CommunicatorDescriptor>&,
            const std::shared_ptr<ServerEntry>&);

        friend struct AddCommunicator;
        friend struct RemoveCommunicator;

        const Ice::CommunicatorPtr _communicator;
        const std::string _instanceName;
        NodeCache& _nodeCache;
        AdapterCache& _adapterCache;
        ObjectCache& _objectCache;
        AllocatableObjectCache& _allocatableObjectCache;
        std::shared_ptr<NodeObserverTopic> _nodeObserverTopic;

        mutable std::mutex _mutex;
    };

};

#endif
