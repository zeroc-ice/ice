// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICEGRID_PLUGIN_FACADE_I_H
#define ICEGRID_PLUGIN_FACADE_I_H

#include <IceUtil/Mutex.h>
#include <IceUtil/Handle.h>

#include <IceGrid/PluginFacade.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class RegistryPluginFacadeI : public RegistryPluginFacade, private IceUtil::Mutex
{
public:

    RegistryPluginFacadeI();

    virtual ApplicationInfo getApplicationInfo(const ::std::string&) const;

    virtual ServerInfo getServerInfo(const std::string&) const;

    virtual std::string getAdapterServer(const std::string&) const;
    virtual std::string getAdapterApplication(const std::string&) const;
    virtual std::string getAdapterNode(const std::string&) const;
    virtual AdapterInfoSeq getAdapterInfo(const ::std::string&) const;

    virtual ObjectInfo getObjectInfo(const Ice::Identity&) const;

    virtual NodeInfo getNodeInfo(const std::string&) const;
    virtual LoadInfo getNodeLoad(const std::string&) const;

    virtual std::string getPropertyForAdapter(const std::string&, const std::string&) const;

    virtual void addReplicaGroupFilter(const std::string&, const ReplicaGroupFilterPtr&);
    virtual bool removeReplicaGroupFilter(const std::string&, const ReplicaGroupFilterPtr&);

    virtual void addTypeFilter(const std::string&, const TypeFilterPtr&);
    virtual bool removeTypeFilter(const std::string&, const TypeFilterPtr&);

    std::vector<ReplicaGroupFilterPtr> getReplicaGroupFilters(const std::string&) const;
    bool hasReplicaGroupFilters() const;

    std::vector<TypeFilterPtr> getTypeFilters(const std::string&) const;
    bool hasTypeFilters() const;

    void setDatabase(const DatabasePtr&);

private:

    DatabasePtr _database;
    std::map<std::string, std::vector<ReplicaGroupFilterPtr> > _replicaGroupFilters;
    std::map<std::string, std::vector<TypeFilterPtr> > _typeFilters;
};
typedef IceUtil::Handle<RegistryPluginFacadeI> RegistryPluginFacadeIPtr;

}

#endif
