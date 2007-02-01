// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#ifndef ICE_GRID_REGISTRYI_H
#define ICE_GRID_REGISTRYI_H

#include <IceGrid/Registry.h>
#include <IceGrid/Query.h>
#include <IceGrid/Locator.h>
#include <IceGrid/Internal.h>
#include <IceGrid/PlatformInfo.h>
#include <IceGrid/ReplicaSessionManager.h>
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

class WaitQueue;
typedef IceUtil::Handle<WaitQueue> WaitQueuePtr;    

class SessionServantLocatorI;
typedef IceUtil::Handle<SessionServantLocatorI> SessionServantLocatorIPtr;    

class ClientSessionFactory;
typedef IceUtil::Handle<ClientSessionFactory> ClientSessionFactoryPtr;    

class AdminSessionFactory;
typedef IceUtil::Handle<AdminSessionFactory> AdminSessionFactoryPtr;    

class RegistryI : public Registry
{
public:

    RegistryI(const Ice::CommunicatorPtr&, const TraceLevelsPtr&);
    ~RegistryI();

    bool start(bool);
    void stop();

    virtual SessionPrx createSession(const std::string&, const std::string&, const Ice::Current&);
    virtual AdminSessionPrx createAdminSession(const std::string&, const std::string&, const Ice::Current&);

    virtual SessionPrx createSessionFromSecureConnection(const Ice::Current&);
    virtual AdminSessionPrx createAdminSessionFromSecureConnection(const Ice::Current&);

    virtual int getSessionTimeout(const Ice::Current& = Ice::Current()) const;
    
    std::string getName() const;
    RegistryInfo getInfo() const;

    void waitForShutdown();
    virtual void shutdown();
    
private:

    Ice::LocatorRegistryPrx setupLocatorRegistry(const Ice::ObjectAdapterPtr&); 
    LocatorPrx setupLocator(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const Ice::LocatorRegistryPrx&,
                            const RegistryPrx&, const QueryPrx&); 
    QueryPrx setupQuery(const Ice::ObjectAdapterPtr&);
    RegistryPrx setupRegistry(const Ice::ObjectAdapterPtr&);
    InternalRegistryPrx setupInternalRegistry(const Ice::ObjectAdapterPtr&);
    void setupNullPermissionsVerifier(const Ice::ObjectAdapterPtr&);
    bool setupUserAccountMapper(const Ice::ObjectAdapterPtr&);
    void setupClientSessionFactory(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const LocatorPrx&,
                                   bool);
    void setupAdminSessionFactory(const Ice::ObjectAdapterPtr&, const Ice::ObjectAdapterPtr&, const LocatorPrx&,
                                  bool);

    void setupThreadPool(const Ice::PropertiesPtr&, const std::string&, int, int = 0);
    Glacier2::PermissionsVerifierPrx getPermissionsVerifier(const Ice::ObjectAdapterPtr&, const LocatorPrx&,
                                                            const std::string&, const std::string&, bool);

    Glacier2::SSLPermissionsVerifierPrx getSSLPermissionsVerifier(const LocatorPrx&, const std::string&, bool);
    Glacier2::SSLInfo getSSLInfo(const Ice::ConnectionPtr&, std::string&);

    NodePrxSeq registerReplicas(const InternalRegistryPrx&, const InternalRegistryPrxSeq&, const NodePrxSeq&);
    void registerNodes(const InternalRegistryPrx&, const NodePrxSeq&);
    
    const Ice::CommunicatorPtr _communicator;
    const TraceLevelsPtr _traceLevels;

    DatabasePtr _database;
    Ice::ObjectAdapterPtr _clientAdapter;
    WellKnownObjectsManagerPtr _wellKnownObjects;
    std::string _instanceName;
    bool _master;
    std::string _replicaName;
    ReapThreadPtr _reaper;
    WaitQueuePtr _waitQueue;
    SessionServantLocatorIPtr _sessionServantLocator;
    int _sessionTimeout;
    ReplicaSessionManager _session;
    mutable PlatformInfo _platform;

    Glacier2::PermissionsVerifierPrx _nullPermissionsVerifier;
    Glacier2::SSLPermissionsVerifierPrx _nullSSLPermissionsVerifier;

    ClientSessionFactoryPtr _clientSessionFactory;
    Glacier2::PermissionsVerifierPrx _clientVerifier;
    Glacier2::SSLPermissionsVerifierPrx _sslClientVerifier;

    AdminSessionFactoryPtr _adminSessionFactory;
    Glacier2::PermissionsVerifierPrx _adminVerifier;
    Glacier2::SSLPermissionsVerifierPrx _sslAdminVerifier;

    IceStorm::ServicePtr _iceStorm;
};
typedef IceUtil::Handle<RegistryI> RegistryIPtr;

}

#endif
