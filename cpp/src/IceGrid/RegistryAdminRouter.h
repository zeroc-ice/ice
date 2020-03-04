//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REGISTRY_ADMIN_ROUTER_H
#define ICE_GRID_REGISTRY_ADMIN_ROUTER_H

#include <IceGrid/Database.h>
#include <IceGrid/AdminRouter.h>

namespace IceGrid
{

class RegistryServerAdminRouter final : public AdminRouter,
                                        public std::enable_shared_from_this<RegistryServerAdminRouter>
{
public:

    RegistryServerAdminRouter(const std::shared_ptr<Database>&);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current) override;

private:

    const std::shared_ptr<Database> _database;
};

class RegistryNodeAdminRouter final : public AdminRouter
{
public:

    RegistryNodeAdminRouter(const std::string&, const std::shared_ptr<Database>&);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current) override;

private:

    const std::string _collocNodeName;
    const std::shared_ptr<Database> _database;
};

class RegistryReplicaAdminRouter final : public AdminRouter
{
public:

    RegistryReplicaAdminRouter(const std::string&, const std::shared_ptr<Database>&);

    void ice_invokeAsync(std::pair<const Ice::Byte*, const Ice::Byte*>,
                         std::function<void(bool, const std::pair<const Ice::Byte*, const Ice::Byte*>&)>,
                         std::function<void(std::exception_ptr)>,
                         const Ice::Current& current) override;

private:

    const std::string _name;
    const std::shared_ptr<Database> _database;
};

}
#endif
