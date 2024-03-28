//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_ADMIN_I_H
#define ICE_GRID_ADMIN_I_H

#include <IceGrid/Admin.h>

namespace IceGrid
{
    class AdminSessionI;
    class Database;
    class RegistryI;
    class TraceLevels;

    class AdminI final : public Admin
    {
    public:
        AdminI(
            const std::shared_ptr<Database>&,
            const std::shared_ptr<RegistryI>&,
            const std::shared_ptr<AdminSessionI>&);

        void addApplication(ApplicationDescriptor, const Ice::Current&) final;
        void syncApplication(ApplicationDescriptor, const Ice::Current&) final;
        void updateApplication(ApplicationUpdateDescriptor, const Ice::Current&) final;
        void syncApplicationWithoutRestart(ApplicationDescriptor, const Ice::Current&) final;
        void updateApplicationWithoutRestart(ApplicationUpdateDescriptor, const Ice::Current&) final;
        void removeApplication(std::string, const Ice::Current&) final;
        void instantiateServer(std::string, std::string, ServerInstanceDescriptor, const Ice::Current&) final;
        ApplicationInfo getApplicationInfo(std::string, const Ice::Current&) const final;
        ApplicationDescriptor getDefaultApplicationDescriptor(const Ice::Current&) const final;
        Ice::StringSeq getAllApplicationNames(const Ice::Current&) const final;

        ServerInfo getServerInfo(std::string, const Ice::Current&) const final;
        ServerState getServerState(std::string, const Ice::Current&) const final;
        std::int32_t getServerPid(std::string, const Ice::Current&) const final;
        std::string getServerAdminCategory(const Ice::Current&) const final;
        std::optional<Ice::ObjectPrx> getServerAdmin(std::string, const Ice::Current&) const final;
        void startServerAsync(
            std::string,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;
        void stopServerAsync(
            std::string,
            std::function<void()>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current&) final;

        void sendSignal(std::string, std::string, const Ice::Current&) final;
        Ice::StringSeq getAllServerIds(const Ice::Current&) const final;
        void enableServer(std::string, bool, const Ice::Current&) final;
        bool isServerEnabled(std::string, const Ice::Current&) const final;

        AdapterInfoSeq getAdapterInfo(std::string, const Ice::Current&) const final;
        void removeAdapter(std::string, const Ice::Current&) final;
        Ice::StringSeq getAllAdapterIds(const Ice::Current&) const final;

        void addObject(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
        void updateObject(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
        void addObjectWithType(std::optional<Ice::ObjectPrx>, std::string, const Ice::Current&) final;
        void removeObject(Ice::Identity, const Ice::Current&) final;
        ObjectInfo getObjectInfo(Ice::Identity, const Ice::Current&) const final;
        ObjectInfoSeq getObjectInfosByType(std::string, const Ice::Current&) const final;
        ObjectInfoSeq getAllObjectInfos(std::string, const Ice::Current&) const final;

        NodeInfo getNodeInfo(std::string, const Ice::Current&) const final;
        std::optional<Ice::ObjectPrx> getNodeAdmin(std::string, const Ice::Current&) const final;
        bool pingNode(std::string, const Ice::Current&) const final;
        LoadInfo getNodeLoad(std::string, const Ice::Current&) const final;
        int getNodeProcessorSocketCount(std::string, const Ice::Current&) const final;
        void shutdownNode(std::string, const Ice::Current&) final;
        std::string getNodeHostname(std::string, const Ice::Current&) const final;
        Ice::StringSeq getAllNodeNames(const Ice::Current&) const final;

        RegistryInfo getRegistryInfo(std::string, const Ice::Current&) const final;
        std::optional<Ice::ObjectPrx> getRegistryAdmin(std::string, const Ice::Current&) const final;
        bool pingRegistry(std::string, const Ice::Current&) const final;
        void shutdownRegistry(std::string, const Ice::Current&) final;
        Ice::StringSeq getAllRegistryNames(const Ice::Current&) const final;

        void shutdown(const Ice::Current&) final;

    private:
        void checkIsReadOnly() const;

        const std::shared_ptr<Database> _database;
        const std::shared_ptr<RegistryI> _registry;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const std::shared_ptr<AdminSessionI> _session;

        std::mutex _mutex;
    };
}

#endif
