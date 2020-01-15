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

    AdminI(const std::shared_ptr<Database>&, const std::shared_ptr<RegistryI>&, const std::shared_ptr<AdminSessionI>&);

    void addApplication(ApplicationDescriptor, const Ice::Current&) override;
    void syncApplication(ApplicationDescriptor, const Ice::Current&) override;
    void updateApplication(ApplicationUpdateDescriptor, const Ice::Current&) override;
    void syncApplicationWithoutRestart(ApplicationDescriptor, const Ice::Current&) override;
    void updateApplicationWithoutRestart(ApplicationUpdateDescriptor, const Ice::Current&) override;
    void removeApplication(std::string, const Ice::Current&) override;
    void instantiateServer(std::string, std::string, ServerInstanceDescriptor, const Ice::Current&) override;
    ApplicationInfo getApplicationInfo(std::string, const Ice::Current&) const override;
    ApplicationDescriptor getDefaultApplicationDescriptor(const Ice::Current&) const override;
    Ice::StringSeq getAllApplicationNames(const Ice::Current&) const override;

    ServerInfo getServerInfo(std::string, const Ice::Current&) const override;
    ServerState getServerState(std::string, const Ice::Current&) const override;
    Ice::Int getServerPid(std::string, const Ice::Current&) const override;
    std::string getServerAdminCategory(const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getServerAdmin(std::string, const Ice::Current&) const override;
    void startServerAsync(std::string, std::function<void()>, std::function<void(std::exception_ptr)>,
                          const Ice::Current&) override;
    void stopServerAsync(std::string, std::function<void()>, std::function<void(std::exception_ptr)>,
                         const Ice::Current&) override;
    void sendSignal(std::string, std::string, const Ice::Current&) override;
    Ice::StringSeq getAllServerIds(const Ice::Current&) const override;
    void enableServer(std::string, bool, const Ice::Current&) override;
    bool isServerEnabled(std::string, const Ice::Current&) const override;

    AdapterInfoSeq getAdapterInfo(std::string, const ::Ice::Current&) const override;
    void removeAdapter(std::string, const Ice::Current&) override;
    Ice::StringSeq getAllAdapterIds(const ::Ice::Current&) const override;

    void addObject(std::shared_ptr<Ice::ObjectPrx>, const ::Ice::Current&) override;
    void updateObject(std::shared_ptr<Ice::ObjectPrx>, const ::Ice::Current&) override;
    void addObjectWithType(std::shared_ptr<Ice::ObjectPrx>, std::string, const ::Ice::Current&) override;
    void removeObject(Ice::Identity, const ::Ice::Current&) override;
    ObjectInfo getObjectInfo(Ice::Identity, const ::Ice::Current&) const override;
    ObjectInfoSeq getObjectInfosByType(std::string, const ::Ice::Current&) const override;
    ObjectInfoSeq getAllObjectInfos(std::string, const ::Ice::Current&) const override;

    NodeInfo getNodeInfo(std::string, const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getNodeAdmin(std::string, const Ice::Current&) const override;
    bool pingNode(std::string, const Ice::Current&) const override;
    LoadInfo getNodeLoad(std::string, const Ice::Current&) const override;
    int getNodeProcessorSocketCount(std::string, const Ice::Current&) const override;
    void shutdownNode(std::string, const Ice::Current&) override;
    std::string getNodeHostname(std::string, const Ice::Current&) const override;
    Ice::StringSeq getAllNodeNames(const ::Ice::Current&) const override;

    RegistryInfo getRegistryInfo(std::string, const Ice::Current&) const override;
    std::shared_ptr<Ice::ObjectPrx> getRegistryAdmin(std::string, const Ice::Current&) const override;
    bool pingRegistry(std::string, const Ice::Current&) const override;
    void shutdownRegistry(std::string, const Ice::Current&) override;
    Ice::StringSeq getAllRegistryNames(const ::Ice::Current&) const override;

    void shutdown(const Ice::Current&) override;

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
