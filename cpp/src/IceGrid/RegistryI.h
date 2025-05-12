// Copyright (c) ZeroC, Inc.

#ifndef ICEGRID_REGISTRYI_H
#define ICEGRID_REGISTRYI_H

#include "../Ice/Timer.h"
#include "../IceStorm/Service.h"
#include "Glacier2/PermissionsVerifier.h"
#include "IceGrid/PluginFacade.h"
#include "IceGrid/Registry.h"
#include "Internal.h"
#include "PlatformInfo.h"
#include "ReplicaSessionManager.h"

namespace IceGrid
{
    class AdminSessionFactory;
    class ClientSessionFactory;
    class Database;
    class ReapThread;
    class SessionServantManager;
    class TraceLevels;
    class WellKnownObjectsManager;

    std::string getInstanceName(const Ice::CommunicatorPtr&);

    class RegistryI : public Registry, public std::enable_shared_from_this<RegistryI>
    {
    public:
        RegistryI(const Ice::CommunicatorPtr&, const std::shared_ptr<TraceLevels>&, bool, std::string, std::string);
        ~RegistryI() override = default;

        bool start();
        bool startImpl();
        void stop();

        std::optional<SessionPrx> createSession(std::string, std::string, const Ice::Current&) override;
        std::optional<AdminSessionPrx> createAdminSession(std::string, std::string, const Ice::Current&) override;

        std::optional<SessionPrx> createSessionFromSecureConnection(const Ice::Current&) override;
        std::optional<AdminSessionPrx> createAdminSessionFromSecureConnection(const Ice::Current&) override;

        [[nodiscard]] int getSessionTimeout(const Ice::Current&) const override;

        [[nodiscard]] std::string getName() const;
        [[nodiscard]] RegistryInfo getInfo() const;

        void waitForShutdown();
        virtual void shutdown();

        [[nodiscard]] std::string getServerAdminCategory() const
        {
            return _instanceName + "-RegistryServerAdminRouter";
        }
        [[nodiscard]] std::string getNodeAdminCategory() const { return _instanceName + "-RegistryNodeAdminRouter"; }
        [[nodiscard]] std::string getReplicaAdminCategory() const
        {
            return _instanceName + "-RegistryReplicaAdminRouter";
        }

        [[nodiscard]] Ice::ObjectPrx createAdminCallbackProxy(const Ice::Identity&) const;

        const Ice::ObjectAdapterPtr& getRegistryAdapter() { return _registryAdapter; }

        Ice::LocatorPrx getLocator();

    private:
        void setupLocatorRegistry();
        LocatorPrx setupLocator(RegistryPrx, QueryPrx);
        QueryPrx setupQuery();
        RegistryPrx setupRegistry();
        InternalRegistryPrx setupInternalRegistry();
        Ice::ObjectAdapterPtr setupClientSessionFactory(const LocatorPrx&);
        Ice::ObjectAdapterPtr setupAdminSessionFactory(
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

        const Ice::CommunicatorPtr _communicator;
        const std::shared_ptr<TraceLevels> _traceLevels;
        const bool _readonly;
        const std::string _initFromReplica;
        const std::string _collocatedNodeName;

        std::shared_ptr<Database> _database;
        Ice::ObjectAdapterPtr _clientAdapter;
        Ice::ObjectAdapterPtr _serverAdapter;
        Ice::ObjectAdapterPtr _registryAdapter;
        std::shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        std::string _instanceName;
        bool _master;
        std::string _replicaName;
        std::shared_ptr<ReapThread> _reaper;
        IceInternal::TimerPtr _timer;
        std::shared_ptr<SessionServantManager> _servantManager;
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
