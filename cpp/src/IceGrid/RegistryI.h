//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#ifndef ICE_GRID_REGISTRYI_H
#define ICE_GRID_REGISTRYI_H

#include <IceUtil/Timer.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/PluginFacade.h>
#include <Glacier2/PermissionsVerifier.h>
#include <IceStorm/Service.h>

namespace IceGrid
{

class AdminSessionFactory;
class ClientSessionFactory;
class Database;
class ReapThread;
class SessionServantManager;
class TraceLevels;
class WellKnownObjectsManager;

std::string getInstanceName(const std::shared_ptr<Ice::Communicator>&);

class RegistryI : public Registry, public std::enable_shared_from_this<RegistryI>
{
public:

    RegistryI(const std::shared_ptr<Ice::Communicator>&, const std::shared_ptr<TraceLevels>&, bool, bool,
              const std::string&, const std::string&);
    virtual ~RegistryI() = default;

    bool start();
    bool startImpl();
    void stop();

    std::shared_ptr<SessionPrx> createSession(std::string, std::string, const Ice::Current&) override;
    std::shared_ptr<AdminSessionPrx> createAdminSession(std::string, std::string, const Ice::Current&) override;

    std::shared_ptr<SessionPrx> createSessionFromSecureConnection(const Ice::Current&) override;
    std::shared_ptr<AdminSessionPrx> createAdminSessionFromSecureConnection(const Ice::Current&) override;

    int getSessionTimeout(const Ice::Current&) const override;
    int getACMTimeout(const Ice::Current&) const override;

    std::string getName() const;
    RegistryInfo getInfo() const;

    void waitForShutdown();
    virtual void shutdown();

    std::string getServerAdminCategory() const { return _instanceName + "-RegistryServerAdminRouter"; }
    std::string getNodeAdminCategory() const { return _instanceName + "-RegistryNodeAdminRouter"; }
    std::string getReplicaAdminCategory() const { return _instanceName + "-RegistryReplicaAdminRouter"; }

    std::shared_ptr<Ice::ObjectPrx> createAdminCallbackProxy(const Ice::Identity&) const;

    const std::shared_ptr<Ice::ObjectAdapter>& getRegistryAdapter() { return _registryAdapter; }

    std::shared_ptr<Ice::LocatorPrx> getLocator();

private:

    void setupLocatorRegistry();
    std::shared_ptr<LocatorPrx> setupLocator(const std::shared_ptr<RegistryPrx>&, const std::shared_ptr<QueryPrx>&);
    std::shared_ptr<QueryPrx> setupQuery();
    std::shared_ptr<RegistryPrx> setupRegistry();
    std::shared_ptr<InternalRegistryPrx> setupInternalRegistry();
    bool setupUserAccountMapper();
    std::shared_ptr<Ice::ObjectAdapter> setupClientSessionFactory(const std::shared_ptr<LocatorPrx>&);
    std::shared_ptr<Ice::ObjectAdapter> setupAdminSessionFactory(const std::shared_ptr<Ice::Object>&,
                                                                 const std::shared_ptr<Ice::Object>&,
                                                                 const std::shared_ptr<Ice::Object>&,
                                                                 const std::shared_ptr<LocatorPrx>&);

    std::shared_ptr<Glacier2::PermissionsVerifierPrx> getPermissionsVerifier(const std::shared_ptr<LocatorPrx>&,
                                                                             const std::string&);
    std::shared_ptr<Glacier2::SSLPermissionsVerifierPrx> getSSLPermissionsVerifier(const std::shared_ptr<LocatorPrx>&,
                                                                                   const std::string&);
    Glacier2::SSLInfo getSSLInfo(const std::shared_ptr<Ice::Connection>&, std::string&);

    NodePrxSeq registerReplicas(const std::shared_ptr<InternalRegistryPrx>&, const NodePrxSeq&);
    void registerNodes(const std::shared_ptr<InternalRegistryPrx>&, const NodePrxSeq&);

    const std::shared_ptr<Ice::Communicator> _communicator;
    const std::shared_ptr<TraceLevels> _traceLevels;
    const bool _nowarn;
    const bool _readonly;
    const std::string _initFromReplica;
    const std::string _collocatedNodeName;

    std::shared_ptr<Database> _database;
    std::shared_ptr<Ice::ObjectAdapter> _clientAdapter;
    std::shared_ptr<Ice::ObjectAdapter> _serverAdapter;
    std::shared_ptr<Ice::ObjectAdapter> _registryAdapter;
    std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    std::string _instanceName;
    bool _master;
    std::string _replicaName;
    std::shared_ptr<ReapThread> _reaper;
    IceUtil::TimerPtr _timer;
    std::shared_ptr<SessionServantManager> _servantManager;
    std::chrono::seconds _sessionTimeout;
    std::unique_ptr<ReplicaSessionManager> _session;
    mutable PlatformInfo _platform;

    std::shared_ptr<ClientSessionFactory> _clientSessionFactory;
    std::shared_ptr<Glacier2::PermissionsVerifierPrx> _clientVerifier;
    std::shared_ptr<Glacier2::SSLPermissionsVerifierPrx> _sslClientVerifier;

    std::shared_ptr<AdminSessionFactory> _adminSessionFactory;
    std::shared_ptr<Glacier2::PermissionsVerifierPrx> _adminVerifier;
    std::shared_ptr<Glacier2::SSLPermissionsVerifierPrx> _sslAdminVerifier;

    std::shared_ptr<IceStormInternal::Service> _iceStorm;
};

}

#endif
