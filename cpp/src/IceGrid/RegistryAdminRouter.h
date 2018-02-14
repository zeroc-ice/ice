// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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

    virtual Ice::ObjectPrx getTarget(const Ice::Current&);

private:

    const DatabasePtr _database;
};


class RegistryNodeAdminRouter : public AdminRouter
{
public:

    RegistryNodeAdminRouter(const std::string&, const DatabasePtr&);
    
    virtual Ice::ObjectPrx getTarget(const Ice::Current&);
    
private:

    const std::string _collocNodeName;
    const DatabasePtr _database;
};

class RegistryReplicaAdminRouter : public AdminRouter
{
public:

    RegistryReplicaAdminRouter(const std::string&, const DatabasePtr&);
    
    virtual Ice::ObjectPrx getTarget(const Ice::Current&);
    
private:

    const std::string _name;
    const DatabasePtr _database;
};


}
#endif
