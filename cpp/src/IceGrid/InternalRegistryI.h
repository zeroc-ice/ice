// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_INTERNALREGISTRYI_H
#define ICEGRID_INTERNALREGISTRYI_H

#include "IceGrid/Registry.h"
#include "Internal.h"

namespace IceGrid
{
    class Database;
    class FileCache;
    class ReapThread;
    class RegistryI;
    class ReplicaSessionManager;
    class WellKnownObjectsManager;

    class InternalRegistryI final : public InternalRegistry
    {
    public:
        InternalRegistryI(
            const std::shared_ptr<RegistryI>&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<ReapThread>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            ReplicaSessionManager&);

        std::optional<NodeSessionPrx>
        registerNode(std::shared_ptr<InternalNodeInfo>, std::optional<NodePrx>, LoadInfo, const Ice::Current&) final;

        std::optional<ReplicaSessionPrx> registerReplica(
            std::shared_ptr<InternalReplicaInfo>,
            std::optional<InternalRegistryPrx>,
            const Ice::Current&) final;

        void registerWithReplica(std::optional<InternalRegistryPrx>, const Ice::Current&) final;

        [[nodiscard]] NodePrxSeq getNodes(const Ice::Current&) const final;
        [[nodiscard]] InternalRegistryPrxSeq getReplicas(const Ice::Current&) const final;

        ApplicationInfoSeq getApplications(std::int64_t&, const Ice::Current&) const final;
        AdapterInfoSeq getAdapters(std::int64_t&, const Ice::Current&) const final;
        ObjectInfoSeq getObjects(std::int64_t&, const Ice::Current&) const final;

        void shutdown(const Ice::Current&) const final;

        [[nodiscard]] std::int64_t getOffsetFromEnd(std::string, int, const Ice::Current&) const final;
        bool read(std::string, std::int64_t, int, std::int64_t&, Ice::StringSeq&, const Ice::Current&) const final;

    private:
        [[nodiscard]] std::string getFilePath(const std::string&) const;

        const std::shared_ptr<RegistryI> _registry;
        const std::shared_ptr<Database> _database;
        const std::shared_ptr<ReapThread> _reaper;
        const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        const std::shared_ptr<FileCache> _fileCache;
        ReplicaSessionManager& _session;
        std::chrono::seconds _nodeSessionTimeout;
        std::chrono::seconds _replicaSessionTimeout;
    };

};

#endif
