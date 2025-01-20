// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_PLUGIN_FACADE_I_H
#define ICEGRID_PLUGIN_FACADE_I_H

#include "IceGrid/PluginFacade.h"

namespace IceGrid
{
    class Database;

    class RegistryPluginFacadeI : public RegistryPluginFacade
    {
    public:
        [[nodiscard]] ApplicationInfo getApplicationInfo(const std::string&) const override;

        [[nodiscard]] ServerInfo getServerInfo(const std::string&) const override;

        [[nodiscard]] std::string getAdapterServer(const std::string&) const override;
        [[nodiscard]] std::string getAdapterApplication(const std::string&) const override;
        [[nodiscard]] std::string getAdapterNode(const std::string&) const override;
        [[nodiscard]] AdapterInfoSeq getAdapterInfo(const std::string&) const override;

        [[nodiscard]] ObjectInfo getObjectInfo(const Ice::Identity&) const override;

        [[nodiscard]] NodeInfo getNodeInfo(const std::string&) const override;
        [[nodiscard]] LoadInfo getNodeLoad(const std::string&) const override;

        [[nodiscard]] std::string getPropertyForAdapter(const std::string&, const std::string&) const override;

        void addReplicaGroupFilter(const std::string&, const std::shared_ptr<ReplicaGroupFilter>&) noexcept override;
        bool removeReplicaGroupFilter(const std::string&, const std::shared_ptr<ReplicaGroupFilter>&) noexcept override;

        void addTypeFilter(const std::string&, const std::shared_ptr<TypeFilter>&) noexcept override;
        bool removeTypeFilter(const std::string&, const std::shared_ptr<TypeFilter>&) noexcept override;

        std::vector<std::shared_ptr<ReplicaGroupFilter>> getReplicaGroupFilters(const std::string&) const;
        [[nodiscard]] bool hasReplicaGroupFilters() const;

        std::vector<std::shared_ptr<TypeFilter>> getTypeFilters(const std::string&) const;
        [[nodiscard]] bool hasTypeFilters() const;

        void setDatabase(const std::shared_ptr<Database>&);

    private:
        std::shared_ptr<Database> _database;
        std::map<std::string, std::vector<std::shared_ptr<ReplicaGroupFilter>>> _replicaGroupFilters;
        std::map<std::string, std::vector<std::shared_ptr<TypeFilter>>> _typeFilters;

        mutable std::mutex _mutex;
    };
}

#endif
