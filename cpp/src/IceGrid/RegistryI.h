// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRYI_H
#define ICE_GRID_REGISTRYI_H

#include <IceUtil/Timer.h>
#include <Ice/UniquePtr.h>
#include <IceGrid/Registry.h>
#include <IceGrid/Internal.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/ReplicaSessionManager.h>
#include <IceGrid/PluginFacade.h>
#include <Glacier2/PermissionsVerifierF.h>
#include <IceStorm/Service.h>

namespace IceGrid
{

class Database;
typedef IceUtil::Handle<Database> DatabasePtr;

class WellKnownObjectsManager;
typedef IceUtil::Handle<WellKnownObjectsManager> WellKnownObjectsManagerPtr;

class TraceLevels;
typedef IceUtil::Handle<TraceLevels> TraceLevelsPtr;

class ReapThread;
typedef IceUtil::Handle<ReapThread> ReapThreadPtr;    

class SessionServantManager;
typedef IceUtil::Handle<SessionServantManager> SessionServantManagerPtr;    

class ClientSessionFactory;
typedef IceUtil::Handle<ClientSessionFactory> ClientSessionFactoryPtr;    

class AdminSessionFactory;
typedef IceUtil::Handle<AdminSessionFactory> AdminSessionFactoryPtr;    

std::string getInstanceName(const Ice::CommunicatorPtr&);

class RegistryI : public Registry
{
public:

    RegistryI(const Ice::CommunicatorPtr&, const TraceLevelsPtr&, bool, bool, const std::string&, const std::string&);
    ~RegistryI();

    bool start();
    bool startImpl();
    void stop();

    virtual SessionPrx createSession(const std::string&, const std::string&, const Ice::Current&);
    virtual AdminSessionPrx createAdminSession(const std::string&, const std::string&, const Ice::Current&);

    virtual SessionPrx createSessionFromSecureConnection(const Ice::Current&);
    virtual AdminSessionPrx createAdminSessionFromSecureConnection(const Ice::Current&);

    virtual int getSessionTimeout(const Ice::Current& = Ice::noExplicitCurrent) const;
    virtual int getACMTimeout(const Ice::Current& = Ice::noExplicitCurrent) const;
    
    std::string getName() const;
    RegistryInfo getInfo() const;

    void waitForShutdown();
    virtual void shutdown();
    
    std::string getServerAdminCategory() const { return _instanceName + "-RegistryServerAdminRouter"; }
    std::string getNodeAdminCategory() const { return _instanceName + "-RegistryNodeAdminRouter"; }
    std::string getReplicaAdminCategory() const { return _instanceName + "-RegistryReplicaAdminRouter"; }

    Ice::ObjectPrx createAdminCallbackProxy(const Ice::Identity&) const;

    const Ice::ObjectAdapterPtr& getRegistryAdapter() { return _registryAdapter; }

    Ice::LocatorPrx getLocator();

private:

    void setupLocatorRegistry(); 
    LocatorPrx setupLocator(const RegistryPrx&, const QueryPrx&); 
    QueryPrx setupQuery();
    RegistryPrx setupRegistry();
    InternalRegistryPrx setupInternalRegistry();
    bool setupUserAccountMapper();
    Ice::ObjectAdapterPtr setupClientSessionFactory(const LocatorPrx&);
    Ice::ObjectAdapterPtr setupAdminSessionFactory(const Ice::ObjectPtr&, const Ice::ObjectPtr&, 
                                                   const Ice::ObjectPtr&, const LocatorPrx&);

    Glacier2::PermissionsVerifierPrx getPermissionsVerifier(const LocatorPrx&, const std::string&);
    Glacier2::SSLPermissionsVerifierPrx getSSLPermissionsVerifier(const LocatorPrx&, const std::string&);
    Glacier2::SSLInfo getSSLInfo(const Ice::ConnectionPtr&, std::string&);

    NodePrxSeq registerReplicas(const InternalRegistryPrx&, const NodePrxSeq&);
    void registerNodes(const InternalRegistryPrx&, const NodePrxSeq&);
    
    const Ice::CommunicatorPtr _communicator;
    const TraceLevelsPtr _traceLevels;
    const bool _nowarn;
    const bool _readonly;
    const std::string _initFromReplica;
    const std::string _collocatedNodeName;

    DatabasePtr _database;
    Ice::ObjectAdapterPtr _clientAdapter;
    Ice::ObjectAdapterPtr _serverAdapter;
    Ice::ObjectAdapterPtr _registryAdapter;
    WellKnownObjectsManagerPtr _wellKnownObjects;
    std::string _instanceName;
    bool _master;
    std::string _replicaName;
    ReapThreadPtr _reaper;
    IceUtil::TimerPtr _timer;
    SessionServantManagerPtr _servantManager;
    int _sessionTimeout;
    IceInternal::UniquePtr<ReplicaSessionManager> _session;
    mutable PlatformInfo _platform;
    
    ClientSessionFactoryPtr _clientSessionFactory;
    Glacier2::PermissionsVerifierPrx _clientVerifier;
    Glacier2::SSLPermissionsVerifierPrx _sslClientVerifier;

    AdminSessionFactoryPtr _adminSessionFactory;
    Glacier2::PermissionsVerifierPrx _adminVerifier;
    Glacier2::SSLPermissionsVerifierPrx _sslAdminVerifier;

    IceStormInternal::ServicePtr _iceStorm;
};
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

}

#endif
