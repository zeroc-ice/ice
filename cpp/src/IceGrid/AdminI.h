// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_ADMIN_I_H
#define ICEGRID_ADMIN_I_H

#include "IceGrid/Admin.h"

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
        [[nodiscard]] ApplicationInfo getApplicationInfo(std::string, const Ice::Current&) const final;
        [[nodiscard]] ApplicationDescriptor getDefaultApplicationDescriptor(const Ice::Current&) const final;
        [[nodiscard]] Ice::StringSeq getAllApplicationNames(const Ice::Current&) const final;

        [[nodiscard]] ServerInfo getServerInfo(std::string, const Ice::Current&) const final;
        [[nodiscard]] ServerState getServerState(std::string, const Ice::Current&) const final;
        [[nodiscard]] std::int32_t getServerPid(std::string, const Ice::Current&) const final;
        [[nodiscard]] std::string getServerAdminCategory(const Ice::Current&) const final;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getServerAdmin(std::string, const Ice::Current&) const final;
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
        [[nodiscard]] Ice::StringSeq getAllServerIds(const Ice::Current&) const final;
        void enableServer(std::string, bool, const Ice::Current&) final;
        [[nodiscard]] bool isServerEnabled(std::string, const Ice::Current&) const final;

        [[nodiscard]] AdapterInfoSeq getAdapterInfo(std::string, const Ice::Current&) const final;
        void removeAdapter(std::string, const Ice::Current&) final;
        [[nodiscard]] Ice::StringSeq getAllAdapterIds(const Ice::Current&) const final;

        void addObject(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
        void updateObject(std::optional<Ice::ObjectPrx>, const Ice::Current&) final;
        void addObjectWithType(std::optional<Ice::ObjectPrx>, std::string, const Ice::Current&) final;
        void removeObject(Ice::Identity, const Ice::Current&) final;
        [[nodiscard]] ObjectInfo getObjectInfo(Ice::Identity, const Ice::Current&) const final;
        [[nodiscard]] ObjectInfoSeq getObjectInfosByType(std::string, const Ice::Current&) const final;
        [[nodiscard]] ObjectInfoSeq getAllObjectInfos(std::string, const Ice::Current&) const final;

        [[nodiscard]] NodeInfo getNodeInfo(std::string, const Ice::Current&) const final;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getNodeAdmin(std::string, const Ice::Current&) const final;
        [[nodiscard]] bool pingNode(std::string, const Ice::Current&) const final;
        [[nodiscard]] LoadInfo getNodeLoad(std::string, const Ice::Current&) const final;
        [[nodiscard]] int getNodeProcessorSocketCount(std::string, const Ice::Current&) const final;
        void shutdownNode(std::string, const Ice::Current&) final;
        [[nodiscard]] std::string getNodeHostname(std::string, const Ice::Current&) const final;
        [[nodiscard]] Ice::StringSeq getAllNodeNames(const Ice::Current&) const final;

        [[nodiscard]] RegistryInfo getRegistryInfo(std::string, const Ice::Current&) const final;
        [[nodiscard]] std::optional<Ice::ObjectPrx> getRegistryAdmin(std::string, const Ice::Current&) const final;
        [[nodiscard]] bool pingRegistry(std::string, const Ice::Current&) const final;
        void shutdownRegistry(std::string, const Ice::Current&) final;
        [[nodiscard]] Ice::StringSeq getAllRegistryNames(const Ice::Current&) const final;

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
