// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_NODE_I_H
#define ICEGRID_NODE_I_H

#include "../Ice/Timer.h"
#include "FileCache.h"
#include "IceGrid/UserAccountMapper.h"
#include "Internal.h"
#include "PlatformInfo.h"

#include <deque>
#include <exception>
#include <set>

namespace IceGrid
{
    class Activator;
    class NodeSessionManager;
    class ServerCommand;
    class ServerI;
    class TraceLevels;

    class NodeI : public Node, public std::enable_shared_from_this<NodeI>
    {
    public:
        class Update final : public std::enable_shared_from_this<Update>
        {
        public:
            using UpdateFunction = std::function<void(std::function<void()>, std::function<void(std::exception_ptr)>)>;

            Update(UpdateFunction, const std::shared_ptr<NodeI>&, const std::optional<NodeObserverPrx>&);
            bool send();

        private:
            UpdateFunction _func;
            std::shared_ptr<NodeI> _node;
            std::optional<NodeObserverPrx> _observer;
        };

        NodeI(
            const Ice::ObjectAdapterPtr&,
            NodeSessionManager&,
            const std::shared_ptr<Activator>&,
            IceInternal::TimerPtr,
            const std::shared_ptr<TraceLevels>&,
            NodePrx,
            std::string,
            const std::optional<UserAccountMapperPrx>&,
            std::string);

        void loadServerAsync(
            std::shared_ptr<InternalServerDescriptor>,
            std::string,
            std::function<void(const std::optional<ServerPrx>&, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void loadServerWithoutRestartAsync(
            std::shared_ptr<InternalServerDescriptor>,
            std::string,
            std::function<void(const std::optional<ServerPrx>&, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void destroyServerAsync(
            std::string,
            std::string,
            int,
            std::string,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) override;

        void destroyServerWithoutRestartAsync(
            std::string,
            std::string,
            int,
            std::string,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

        void registerWithReplica(std::optional<InternalRegistryPrx>, const Ice::Current&) override;

        void replicaInit(InternalRegistryPrxSeq, const Ice::Current&) override;
        void replicaAdded(std::optional<InternalRegistryPrx>, const Ice::Current&) override;
        void replicaRemoved(std::optional<InternalRegistryPrx>, const Ice::Current&) override;

        [[nodiscard]] std::string getName(const Ice::Current&) const override;
        [[nodiscard]] std::string getHostname(const Ice::Current&) const override;
        [[nodiscard]] LoadInfo getLoad(const Ice::Current&) const override;
        [[nodiscard]] int getProcessorSocketCount(const Ice::Current&) const override;
        void shutdown(const Ice::Current&) const override;

        [[nodiscard]] std::int64_t getOffsetFromEnd(std::string, int, const Ice::Current&) const override;
        bool read(std::string, std::int64_t, int, std::int64_t&, Ice::StringSeq&, const Ice::Current&) const override;

        void shutdown();

        [[nodiscard]] IceInternal::TimerPtr getTimer() const;
        [[nodiscard]] Ice::CommunicatorPtr getCommunicator() const;
        [[nodiscard]] Ice::ObjectAdapterPtr getAdapter() const;
        [[nodiscard]] std::shared_ptr<Activator> getActivator() const;
        [[nodiscard]] std::shared_ptr<TraceLevels> getTraceLevels() const;
        [[nodiscard]] std::optional<UserAccountMapperPrx> getUserAccountMapper() const;
        PlatformInfo& getPlatformInfo();
        [[nodiscard]] std::shared_ptr<FileCache> getFileCache() const;
        [[nodiscard]] NodePrx getProxy() const;
        [[nodiscard]] const PropertyDescriptorSeq& getPropertiesOverride() const;
        [[nodiscard]] const std::string& getInstanceName() const;

        [[nodiscard]] std::string getOutputDir() const;
        [[nodiscard]] bool getRedirectErrToOut() const;
        [[nodiscard]] bool allowEndpointsOverride() const;

        std::optional<NodeSessionPrx> registerWithRegistry(const InternalRegistryPrx&);
        void checkConsistency(const NodeSessionPrx&);
        [[nodiscard]] std::optional<NodeSessionPrx> getMasterNodeSession() const;

        void addObserver(NodeSessionPrx, NodeObserverPrx);
        void removeObserver(const NodeSessionPrx&);
        void observerUpdateServer(const ServerDynamicInfo&);
        void observerUpdateAdapter(const AdapterDynamicInfo&);

        void queueUpdate(const std::optional<NodeObserverPrx>&, Update::UpdateFunction);
        void dequeueUpdate(const std::optional<NodeObserverPrx>&, const std::shared_ptr<Update>&, bool);

        void addServer(const std::shared_ptr<ServerI>&, const std::string&);
        void removeServer(const std::shared_ptr<ServerI>&, const std::string&);

        [[nodiscard]] Ice::Identity createServerIdentity(const std::string&) const;
        [[nodiscard]] std::string getServerAdminCategory() const;

        bool canRemoveServerDirectory(const std::string&);

    private:
        std::vector<std::shared_ptr<ServerCommand>> checkConsistencyNoSync(const Ice::StringSeq&);

        std::set<std::shared_ptr<ServerI>> getApplicationServers(const std::string&) const;
        [[nodiscard]] std::string getFilePath(const std::string&) const;

        void loadServer(
            std::shared_ptr<InternalServerDescriptor>,
            std::string,
            bool,
            std::function<void(const std::optional<ServerPrx>&, const AdapterPrxDict&, int, int)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&);

        void destroyServer(
            std::string,
            std::string,
            int,
            std::string,
            bool,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&);

        const Ice::CommunicatorPtr _communicator;
        const Ice::ObjectAdapterPtr _adapter;
        NodeSessionManager& _sessions;
        const std::shared_ptr<Activator> _activator;
        const IceInternal::TimerPtr _timer;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::string _name;
        const NodePrx _proxy;
        const std::string _outputDir;
        const bool _redirectErrToOut{false};
        const bool _allowEndpointsOverride{false};
        const int _waitTime{0};
        const std::string _instanceName;
        const std::optional<UserAccountMapperPrx> _userAccountMapper;
        PlatformInfo _platform;
        const std::string _dataDir;
        const std::string _serversDir;
        const std::string _tmpDir;
        const std::shared_ptr<FileCache> _fileCache;
        PropertyDescriptorSeq _propertiesOverride;

        unsigned long _serial{1};
        bool _consistencyCheckDone{false};

        std::mutex _observerMutex;
        std::map<NodeSessionPrx, NodeObserverPrx> _observers;
        std::map<std::string, ServerDynamicInfo> _serversDynamicInfo;
        std::map<std::string, AdapterDynamicInfo> _adaptersDynamicInfo;

        std::map<std::optional<NodeObserverPrx>, std::deque<std::shared_ptr<Update>>> _observerUpdates;

        mutable std::mutex _serversMutex;
        std::map<std::string, std::set<std::shared_ptr<ServerI>>> _serversByApplication;

        std::mutex _mutex;
        std::condition_variable _condVar;
    };
}

#endif
