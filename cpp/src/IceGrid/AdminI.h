// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_ADMIN_I_H
#define ICE_GRID_ADMIN_I_H

#include <IceGrid/Admin.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class RegistryI;
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

class AdminSessionI;
typedef IceUtil::Handle<AdminSessionI> AdminSessionIPtr;

class AdminI : public Admin, public IceUtil::Mutex
{
public:

    AdminI(const DatabasePtr&, const RegistryIPtr&, const AdminSessionIPtr&);
    virtual ~AdminI();

    virtual void addApplication(const ApplicationDescriptor&, const Ice::Current&);
    virtual void syncApplication(const ApplicationDescriptor&, const Ice::Current&);
    virtual void updateApplication(const ApplicationUpdateDescriptor&, const Ice::Current&);
    virtual void syncApplicationWithoutRestart(const ApplicationDescriptor&, const Ice::Current&);
    virtual void updateApplicationWithoutRestart(const ApplicationUpdateDescriptor&, const Ice::Current&);
    virtual void removeApplication(const std::string&, const Ice::Current&);
    virtual void instantiateServer(const std::string&, const std::string&, const ServerInstanceDescriptor&,
                                   const Ice::Current&);
    virtual void patchApplication_async(const AMD_Admin_patchApplicationPtr&, const std::string&, bool,
                                        const Ice::Current&);
    virtual ApplicationInfo getApplicationInfo(const ::std::string&, const Ice::Current&) const;
    virtual ApplicationDescriptor getDefaultApplicationDescriptor(const Ice::Current&) const;
    virtual Ice::StringSeq getAllApplicationNames(const Ice::Current&) const;

    virtual ServerInfo getServerInfo(const ::std::string&, const Ice::Current&) const;
    virtual ServerState getServerState(const ::std::string&, const Ice::Current&) const;
    virtual Ice::Int getServerPid(const ::std::string&, const Ice::Current&) const;
    virtual std::string getServerAdminCategory(const Ice::Current&) const;
    virtual Ice::ObjectPrx getServerAdmin(const std::string&, const Ice::Current&) const;
    virtual void startServer_async(const AMD_Admin_startServerPtr&, const ::std::string&, const Ice::Current&);
    virtual void stopServer_async(const AMD_Admin_stopServerPtr&, const ::std::string&, const Ice::Current&);
    virtual void patchServer_async(const AMD_Admin_patchServerPtr&, const ::std::string&, bool, const Ice::Current&);
    virtual void sendSignal(const ::std::string&, const ::std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllServerIds(const Ice::Current&) const;
    virtual void enableServer(const ::std::string&, bool, const Ice::Current&);
    virtual bool isServerEnabled(const ::std::string&, const Ice::Current&) const;

    virtual AdapterInfoSeq getAdapterInfo(const ::std::string&, const ::Ice::Current&) const;
    virtual void removeAdapter(const std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllAdapterIds(const ::Ice::Current&) const;

    virtual void addObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void updateObject(const ::Ice::ObjectPrx&, const ::Ice::Current&);
    virtual void addObjectWithType(const ::Ice::ObjectPrx&, const ::std::string&, const ::Ice::Current&);
    virtual void removeObject(const ::Ice::Identity&, const ::Ice::Current&);
    virtual ObjectInfo getObjectInfo(const Ice::Identity&, const ::Ice::Current&) const;
    virtual ObjectInfoSeq getObjectInfosByType(const std::string&, const ::Ice::Current&) const;
    virtual ObjectInfoSeq getAllObjectInfos(const std::string&, const ::Ice::Current&) const;

    virtual NodeInfo getNodeInfo(const std::string&, const Ice::Current&) const;
    virtual Ice::ObjectPrx getNodeAdmin(const std::string&, const Ice::Current&) const;
    virtual bool pingNode(const std::string&, const Ice::Current&) const;
    virtual LoadInfo getNodeLoad(const std::string&, const Ice::Current&) const;
    virtual int getNodeProcessorSocketCount(const std::string&, const Ice::Current&) const;
    virtual void shutdownNode(const std::string&, const Ice::Current&);
    virtual std::string getNodeHostname(const std::string&, const Ice::Current&) const;
    virtual Ice::StringSeq getAllNodeNames(const ::Ice::Current&) const;

    virtual RegistryInfo getRegistryInfo(const std::string&, const Ice::Current&) const;
    virtual Ice::ObjectPrx getRegistryAdmin(const std::string&, const Ice::Current&) const;
    virtual bool pingRegistry(const std::string&, const Ice::Current&) const;
    virtual void shutdownRegistry(const std::string&, const Ice::Current&);
    virtual Ice::StringSeq getAllRegistryNames(const ::Ice::Current&) const;

    virtual void shutdown(const Ice::Current&);

    virtual Ice::SliceChecksumDict getSliceChecksums(const Ice::Current&) const;

private:

    void checkIsReadOnly() const;

    const DatabasePtr _database;
    const RegistryIPtr _registry;
    const TraceLevelsPtr _traceLevels;
    const AdminSessionIPtr _session;

};
typedef IceUtil::Handle<AdminI> AdminIPtr;

}

#endif
