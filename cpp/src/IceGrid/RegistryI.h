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
        RegistryI(
            const std::shared_ptr<Ice::Communicator>&,
            const std::shared_ptr<TraceLevels>&,
            bool,
            bool,
            const std::string&,
            const std::string&);
        virtual ~RegistryI() = default;

        bool start();
        bool startImpl();
        void stop();

        std::optional<SessionPrx> createSession(std::string, std::string, const Ice::Current&) override;
        std::optional<AdminSessionPrx> createAdminSession(std::string, std::string, const Ice::Current&) override;

        std::optional<SessionPrx> createSessionFromSecureConnection(const Ice::Current&) override;
        std::optional<AdminSessionPrx> createAdminSessionFromSecureConnection(const Ice::Current&) override;

        int getSessionTimeout(const Ice::Current&) const override;
        int getACMTimeout(const Ice::Current&) const override;

        std::string getName() const;
        RegistryInfo getInfo() const;

        void waitForShutdown();
        virtual void shutdown();

        std::string getServerAdminCategory() const { return _instanceName + "-RegistryServerAdminRouter"; }
        std::string getNodeAdminCategory() const { return _instanceName + "-RegistryNodeAdminRouter"; }
        std::string getReplicaAdminCategory() const { return _instanceName + "-RegistryReplicaAdminRouter"; }

        Ice::ObjectPrx createAdminCallbackProxy(const Ice::Identity&) const;

        const std::shared_ptr<Ice::ObjectAdapter>& getRegistryAdapter() { return _registryAdapter; }

        Ice::LocatorPrx getLocator();

    private:
        void setupLocatorRegistry();
        LocatorPrx setupLocator(RegistryPrx, QueryPrx);
        QueryPrx setupQuery();
        RegistryPrx setupRegistry();
        InternalRegistryPrx setupInternalRegistry();
        std::shared_ptr<Ice::ObjectAdapter> setupClientSessionFactory(const LocatorPrx&);
        std::shared_ptr<Ice::ObjectAdapter> setupAdminSessionFactory(
            const Ice::ObjectPtr&,
            const Ice::ObjectPtr&,
            const Ice::ObjectPtr&,
            const LocatorPrx&);

        std::optional<Glacier2::PermissionsVerifierPrx> getPermissionsVerifier(const LocatorPrx&, const std::string&);
        std::optional<Glacier2::SSLPermissionsVerifierPrx>
        getSSLPermissionsVerifier(const LocatorPrx&, const std::string&);
        Glacier2::SSLInfo getSSLInfo(const Ice::ConnectionPtr&, std::string&);

        NodePrxSeq registerReplicas(const InternalRegistryPrx&, const NodePrxSeq&);
        void registerNodes(const NodePrxSeq&);

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
        std::optional<Glacier2::PermissionsVerifierPrx> _clientVerifier;
        std::optional<Glacier2::SSLPermissionsVerifierPrx> _sslClientVerifier;

        std::shared_ptr<AdminSessionFactory> _adminSessionFactory;
        std::optional<Glacier2::PermissionsVerifierPrx> _adminVerifier;
        std::optional<Glacier2::SSLPermissionsVerifierPrx> _sslAdminVerifier;

        std::shared_ptr<IceStormInternal::Service> _iceStorm;
    };
}

#endif
