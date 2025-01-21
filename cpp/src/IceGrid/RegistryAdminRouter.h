// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REGISTRY_ADMIN_ROUTER_H
#define ICEGRID_REGISTRY_ADMIN_ROUTER_H

#include "AdminRouter.h"
#include "Database.h"

namespace IceGrid
{
    class RegistryServerAdminRouter final : public AdminRouter,
                                            public std::enable_shared_from_this<RegistryServerAdminRouter>
    {
    public:
        RegistryServerAdminRouter(const std::shared_ptr<Database>&);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

    private:
        const std::shared_ptr<Database> _database;
    };

    class RegistryNodeAdminRouter final : public AdminRouter
    {
    public:
        RegistryNodeAdminRouter(std::string, const std::shared_ptr<Database>&);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

    private:
        const std::string _collocNodeName;
        const std::shared_ptr<Database> _database;
    };

    class RegistryReplicaAdminRouter final : public AdminRouter
    {
    public:
        RegistryReplicaAdminRouter(std::string, const std::shared_ptr<Database>&);

        void ice_invokeAsync(
            std::pair<const std::byte*, const std::byte*>,
            std::function<void(bool, std::pair<const std::byte*, const std::byte*>)>,
            std::function<void(std::exception_ptr)>,
            const Ice::Current& current) override;

    private:
        const std::string _name;
        const std::shared_ptr<Database> _database;
    };
}
#endif
