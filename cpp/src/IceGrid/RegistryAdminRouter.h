// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRY_ADMIN_ROUTER_H
#define ICE_GRID_REGISTRY_ADMIN_ROUTER_H

#include <IceGrid/Database.h>
#include <IceGrid/AdminRouter.h>

namespace IceGrid
{

class RegistryServerAdminRouter : public AdminRouter
{
public:

    RegistryServerAdminRouter(const DatabasePtr&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:

    const DatabasePtr _database;
};

class RegistryNodeAdminRouter : public AdminRouter
{
public:

    RegistryNodeAdminRouter(const std::string&, const DatabasePtr&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:

    const std::string _collocNodeName;
    const DatabasePtr _database;
};

class RegistryReplicaAdminRouter : public AdminRouter
{
public:

    RegistryReplicaAdminRouter(const std::string&, const DatabasePtr&);

    virtual void ice_invoke_async(const Ice::AMD_Object_ice_invokePtr&,
                                  const std::pair<const Ice::Byte*, const Ice::Byte*>&,
                                  const Ice::Current&);

private:

    const std::string _name;
    const DatabasePtr _database;
};

}
#endif
