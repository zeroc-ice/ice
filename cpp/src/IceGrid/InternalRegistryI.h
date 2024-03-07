//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_INTERNALREGISTRYI_H
#define ICE_GRID_INTERNALREGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>

namespace IceGrid
{

    class Database;
    class FileCache;
    class ReapThread;
    class RegistryI;
    class ReplicaSessionManager;
    class WellKnownObjectsManager;

    class InternalRegistryI : public InternalRegistry
    {
    public:
        InternalRegistryI(
            const std::shared_ptr<RegistryI>&,
            const std::shared_ptr<Database>&,
            const std::shared_ptr<ReapThread>&,
            const std::shared_ptr<WellKnownObjectsManager>&,
            ReplicaSessionManager&);

        NodeSessionPrxPtr
        registerNode(std::shared_ptr<InternalNodeInfo>, NodePrxPtr, LoadInfo, const Ice::Current&) override;
        ReplicaSessionPrxPtr
        registerReplica(std::shared_ptr<InternalReplicaInfo>, InternalRegistryPrxPtr, const Ice::Current&) override;

        void registerWithReplica(InternalRegistryPrxPtr, const Ice::Current&) override;

        NodePrxSeq getNodes(const Ice::Current&) const override;
        InternalRegistryPrxSeq getReplicas(const Ice::Current&) const override;

        ApplicationInfoSeq getApplications(std::int64_t&, const Ice::Current&) const override;
        AdapterInfoSeq getAdapters(std::int64_t&, const Ice::Current&) const override;
        ObjectInfoSeq getObjects(std::int64_t&, const Ice::Current&) const override;

        void shutdown(const Ice::Current&) const override;

        std::int64_t getOffsetFromEnd(std::string, int, const Ice::Current&) const override;
        bool read(std::string, std::int64_t, int, std::int64_t&, Ice::StringSeq&, const Ice::Current&) const override;

    private:
        std::string getFilePath(const std::string&) const;

        const std::shared_ptr<RegistryI> _registry;
        const std::shared_ptr<Database> _database;
        const std::shared_ptr<ReapThread> _reaper;
        const std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        const std::shared_ptr<FileCache> _fileCache;
        ReplicaSessionManager& _session;
        std::chrono::seconds _nodeSessionTimeout;
        std::chrono::seconds _replicaSessionTimeout;
        bool _requireNodeCertCN;
        bool _requireReplicaCertCN;
    };

};

#endif
