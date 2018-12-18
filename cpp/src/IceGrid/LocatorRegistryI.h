// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

#ifndef ICE_GRID_LOCATOR_REGISTRY_I_H
#define ICE_GRID_LOCATOR_REGISTRY_I_H

#include <IceGrid/Internal.h>
#include <Ice/Locator.h>
#include <IceUtil/Shared.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReplicaSessionManager;

class LocatorRegistryI : public Ice::LocatorRegistry
{
public:

    class AdapterSetDirectProxyCB : public virtual IceUtil::Shared
    {
    public:

        virtual void response() = 0;
        virtual void exception(const ::Ice::Exception&) = 0;
    };
    typedef IceUtil::Handle<AdapterSetDirectProxyCB> AdapterSetDirectProxyCBPtr;

    LocatorRegistryI(const DatabasePtr&, bool, bool, ReplicaSessionManager&);

    virtual void setAdapterDirectProxy_async(const Ice::AMD_LocatorRegistry_setAdapterDirectProxyPtr&,
                                             const std::string&, const Ice::ObjectPrx&, const Ice::Current&);

    virtual void setReplicatedAdapterDirectProxy_async(
        const Ice::AMD_LocatorRegistry_setReplicatedAdapterDirectProxyPtr&, const std::string&, const std::string&,
        const Ice::ObjectPrx&, const Ice::Current&);

    virtual void setServerProcessProxy_async(const Ice::AMD_LocatorRegistry_setServerProcessProxyPtr&,
                                             const ::std::string&, const ::Ice::ProcessPrx&, const ::Ice::Current&);

    void setAdapterDirectProxy(const AdapterSetDirectProxyCBPtr&, const std::string&, const std::string&,
                               const Ice::ObjectPrx&);

    const TraceLevelsPtr& getTraceLevels() const;

private:

    const DatabasePtr _database;
    const bool _dynamicRegistration;
    const bool _master;
    ReplicaSessionManager& _session;
};
typedef IceUtil::Handle<LocatorRegistryI> LocatorRegistryIPtr;

}

#endif
