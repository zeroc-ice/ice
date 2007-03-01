// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <Ice/Network.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname

#include <IceStorm/Service.h>
#include <IceSSL/Plugin.h>
#include <Glacier2/PermissionsVerifier.h>

#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/ReapThread.h>

#include <IceGrid/RegistryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/InternalRegistryI.h>
#include <IceGrid/SessionServantLocatorI.h>
#include <IceGrid/FileUserAccountMapperI.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/FileCache.h>

#include <fstream>

#include <openssl/des.h> // For crypt() passwords

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#   ifdef _MSC_VER
#      define S_ISDIR(mode) ((mode) & _S_IFDIR)
#      define S_ISREG(mode) ((mode) & _S_IFREG)
#   endif
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{


class NullPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(const string& userId, const string& password, string&, const Current&) const
    {
        return true;
    }
};

class NullSSLPermissionsVerifierI : public Glacier2::SSLPermissionsVerifier
{
public:

    virtual bool
    authorize(const Glacier2::SSLInfo&, std::string&, const Ice::Current&) const
    {
        return true;
    }
};

class CryptPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    CryptPermissionsVerifierI(const map<string, string>& passwords) : _passwords(passwords)
    {
    }

    bool checkPermissions(const string& userId, const string& password, string&, const Current&) const 
    {
        map<string, string>::const_iterator p = _passwords.find(userId);
        if(p == _passwords.end())
        {
            return false;
        }
        
        if(p->second.size() != 13) // Crypt passwords are 13 characters long.
        {
            return false;
        }
        
        char buff[14];
        string salt = p->second.substr(0, 2);
#if OPENSSL_VERSION_NUMBER >= 0x0090700fL
        DES_fcrypt(password.c_str(), salt.c_str(), buff);
#else
        des_fcrypt(password.c_str(), salt.c_str(), buff);
#endif
        return p->second == buff;
    }

private:

    const std::map<std::string, std::string> _passwords;
};

};

RegistryI::RegistryI(const CommunicatorPtr& communicator, const TraceLevelsPtr& traceLevels) : 
    _communicator(communicator),
    _traceLevels(traceLevels),
    _platform("IceGrid.Registry", communicator, traceLevels)
{
}

RegistryI::~RegistryI()
{
}

bool
RegistryI::start(bool nowarn)
{
    assert(_communicator);
    PropertiesPtr properties = _communicator->getProperties();

    //
    // Initialize the database environment.
    //
    string dbPath = properties->getProperty("IceGrid.Registry.Data");
    if(dbPath.empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Data' is not set";
        return false;
    }
    else
    {
        struct stat filestat;
        if(stat(dbPath.c_str(), &filestat) != 0 || !S_ISDIR(filestat.st_mode))
        {
            Error out(_communicator->getLogger());
            SyscallException ex(__FILE__, __LINE__);
            ex.error = getSystemErrno();
            out << "property `IceGrid.Registry.Data' is set to an invalid path:\n" << ex;
            return false;
        }
    }

    //
    // Check that required properties are set and valid.
    //
    if(properties->getProperty("IceGrid.Registry.Client.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Client.Endpoints' is not set";
        return false;
    }

    if(properties->getProperty("IceGrid.Registry.Server.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Server.Endpoints' is not set";
        return false;
    }

    if(properties->getProperty("IceGrid.Registry.Internal.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Internal.Endpoints' is not set";
        return false;
    }

    if(!properties->getProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        if(!nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "session manager endpoints `IceGrid.Registry.SessionManager.Endpoints' enabled";
        }
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IceGrid.Registry.Client.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Server.AdapterId", "");
    properties->setProperty("IceGrid.Registry.SessionManager.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Internal.AdapterId", "");

    setupThreadPool(properties, "Ice.ThreadPool.Client", 1, 100);
    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.SessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _replicaName = properties->getPropertyWithDefault("IceGrid.Registry.ReplicaName", "Master");
    _master = _replicaName == "Master";
    _sessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionTimeout", 30);

    //
    // Get the instance name
    //
    if(_master)
    {
        _instanceName = properties->getProperty("IceGrid.InstanceName");    
        if(_instanceName.empty())
        {
            if(_communicator->getDefaultLocator())
            {
                _instanceName = _communicator->getDefaultLocator()->ice_getIdentity().category;
            }
            else
            {
                _instanceName = "IceGrid";
            }
        }
    }
    else
    {
        if(properties->getProperty("Ice.Default.Locator").empty())
        {
            Error out(_communicator->getLogger());
            out << "property `Ice.Default.Locator' is not set";
            return false;
        }
        _instanceName = _communicator->getDefaultLocator()->ice_getIdentity().category;
    }

    //
    // Ensure that nothing is running on this port. This is also
    // useful to ensure that we don't run twice the same instance of
    // the service too (which would cause the database environment of
    // the already running instance to be "corrupted".)
    //
    try
    {
        string endpoints = properties->getProperty("IceGrid.Registry.Client.Endpoints");
        string strPrx = _instanceName + "/Locator:" + endpoints;
        _communicator->stringToProxy(strPrx)->ice_timeout(5000)->ice_ping();

        Error out(_communicator->getLogger());
        out << "an IceGrid registry is already running and listening on the client endpoints `" << endpoints << "'";
        return false;
    }
    catch(const Ice::LocalException&)
    {
    }
    
    properties->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
    properties->setProperty("Freeze.DbEnv.Registry.DbPrivate", "0");

    //
    // Create the reaper thread.
    //
    _reaper = new ReapThread();
    _reaper->start();

    //
    // Create the internal registry object adapter.
    //
    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Internal");
    registryAdapter->activate();

    //
    // Create the internal IceStorm service.
    //
    Identity registryTopicManagerId;
    registryTopicManagerId.category = _instanceName;
    registryTopicManagerId.name = "RegistryTopicManager";
    _iceStorm = IceStorm::Service::create(_communicator, 
                                          registryAdapter, 
                                          registryAdapter, 
                                          "IceGrid.Registry", 
                                          registryTopicManagerId,
                                          "Registry");
    const IceStorm::TopicManagerPrx topicManager = _iceStorm->getTopicManager();

    //
    // Create the registry database.
    //
    _database = new Database(registryAdapter, topicManager, _instanceName, _traceLevels, getInfo());
    _wellKnownObjects = new WellKnownObjectsManager(_database);

    //
    // Get the saved replica/node proxies and remove them from the
    // database.
    //
    Ice::ObjectProxySeq proxies;
    Ice::ObjectProxySeq::const_iterator p;

    NodePrxSeq nodes;
    proxies = _database->getInternalObjectsByType(Node::ice_staticId());
    for(p = proxies.begin(); p != proxies.end(); ++p)
    {
        nodes.push_back(NodePrx::uncheckedCast(*p));
    }

    InternalRegistryPrxSeq replicas;
    proxies = _database->getObjectsByType(InternalRegistry::ice_staticId());
    for(p = proxies.begin(); p != proxies.end(); ++p)
    {
        replicas.push_back(InternalRegistryPrx::uncheckedCast(*p));
    }

    //
    // NOTE: The internal registry object must be added only once the
    // node/replica proxies are retrieved and removed from the
    // database. Otherwise, if some replica/node register as soon as
    // the internal registry is setup we might clear valid proxies.
    //
    InternalRegistryPrx internalRegistry = setupInternalRegistry(registryAdapter);
    if(_master)
    {
        nodes = registerReplicas(internalRegistry, replicas, nodes);
        registerNodes(internalRegistry, nodes);
    }
    else
    {
        InternalReplicaInfoPtr info = _platform.getInternalReplicaInfo();
        _session.create(_replicaName, info, _database, _wellKnownObjects, internalRegistry);
        registerNodes(internalRegistry, _session.getNodes(nodes));
    }

    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    _clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");
    ObjectAdapterPtr sessionManagerAdapter;
    if(!properties->getProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        sessionManagerAdapter = _communicator->createObjectAdapter("IceGrid.Registry.SessionManager");
    }

    Ice::Identity dummy;
    dummy.name = "dummy";
    _wellKnownObjects->addEndpoint("Client", _clientAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Server", serverAdapter->createDirectProxy(dummy));
    if(sessionManagerAdapter)
    {
        _wellKnownObjects->addEndpoint("SessionManager", sessionManagerAdapter->createDirectProxy(dummy));
    }
    _wellKnownObjects->addEndpoint("Internal", registryAdapter->createDirectProxy(dummy));

    setupNullPermissionsVerifier(registryAdapter);
    if(!setupUserAccountMapper(registryAdapter))
    {
        return false;
    }

    QueryPrx query = setupQuery(_clientAdapter);
    RegistryPrx registry = setupRegistry(_clientAdapter);

    Ice::LocatorRegistryPrx locatorRegistry = setupLocatorRegistry(serverAdapter);
    LocatorPrx internalLocator = setupLocator(_clientAdapter, registryAdapter, locatorRegistry, registry, query);

    //
    // Add a default servant locator to the client object adapter. The
    // default servant ensure that request on session objects are from
    // the same connection as the connection that created the session.
    //
    _sessionServantLocator = new SessionServantLocatorI(_clientAdapter, _instanceName);
    _clientAdapter->addServantLocator(_sessionServantLocator, "");    
    
    setupClientSessionFactory(registryAdapter, sessionManagerAdapter, internalLocator, nowarn);
    setupAdminSessionFactory(registryAdapter, sessionManagerAdapter, internalLocator, nowarn);

    _wellKnownObjects->finish();
    if(_master)
    {
        _wellKnownObjects->registerAll();
    }
    else
    {
        _session.registerAllWellKnownObjects();
    }

    //
    // We are ready to go!
    //
    serverAdapter->activate();
    _clientAdapter->activate();
    if(sessionManagerAdapter)
    {
        sessionManagerAdapter->activate();
    }

    return true;
}

Ice::LocatorRegistryPrx
RegistryI::setupLocatorRegistry(const Ice::ObjectAdapterPtr& serverAdapter)
{
    bool dynReg = _communicator->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegId;
    locatorRegId.category = _instanceName;
    locatorRegId.name = "LocatorRegistry-" + _replicaName;
    ObjectPrx obj = serverAdapter->add(new LocatorRegistryI(_database, dynReg, _master, _session), locatorRegId);
    return LocatorRegistryPrx::uncheckedCast(obj);
}

IceGrid::LocatorPrx
RegistryI::setupLocator(const Ice::ObjectAdapterPtr& clientAdapter, 
                        const Ice::ObjectAdapterPtr& registryAdapter,
                        const Ice::LocatorRegistryPrx& locatorRegistry,
                        const RegistryPrx& registry,
                        const QueryPrx& query)
{
    LocatorPtr locator = new LocatorI(_communicator, _database, locatorRegistry, registry, query);
    Identity locatorId;
    locatorId.category = _instanceName;

    locatorId.name = "Locator";
    clientAdapter->add(locator, locatorId);

    locatorId.name = "Locator-" + _replicaName;
    clientAdapter->add(locator, locatorId);
    
    return LocatorPrx::uncheckedCast(registryAdapter->addWithUUID(locator));
}

QueryPrx
RegistryI::setupQuery(const Ice::ObjectAdapterPtr& clientAdapter)
{
    Identity queryId;
    queryId.category = _instanceName;
    queryId.name = "Query";
    return QueryPrx::uncheckedCast(clientAdapter->add(new QueryI(_communicator, _database), queryId));
}

RegistryPrx
RegistryI::setupRegistry(const Ice::ObjectAdapterPtr& clientAdapter)
{
    Identity registryId;
    registryId.category = _instanceName;
    registryId.name = "Registry";
    if(!_master)
    {
        registryId.name += "-" + _replicaName;
    }
    RegistryPrx proxy = RegistryPrx::uncheckedCast(clientAdapter->add(this, registryId));
    _wellKnownObjects->add(proxy, Registry::ice_staticId());
    return proxy;
}

InternalRegistryPrx
RegistryI::setupInternalRegistry(const Ice::ObjectAdapterPtr& registryAdapter)
{
    Identity internalRegistryId;
    internalRegistryId.category = _instanceName;
    internalRegistryId.name = "InternalRegistry-" + _replicaName;
    assert(_reaper);
    ObjectPtr internalRegistry = new InternalRegistryI(this, _database, _reaper, _wellKnownObjects, _session);
    Ice::ObjectPrx proxy = registryAdapter->add(internalRegistry, internalRegistryId);
    _wellKnownObjects->add(proxy, InternalRegistry::ice_staticId());

    InternalRegistryPrx registry = InternalRegistryPrx::uncheckedCast(proxy);
    _database->getReplicaCache().setInternalRegistry(registry);
    return registry;
}

void
RegistryI::setupNullPermissionsVerifier(const Ice::ObjectAdapterPtr& registryAdapter)
{
    Identity nullPermVerifId;
    nullPermVerifId.category = _instanceName;
    nullPermVerifId.name = "NullPermissionsVerifier";
    _nullPermissionsVerifier = Glacier2::PermissionsVerifierPrx::uncheckedCast(
        registryAdapter->add(new NullPermissionsVerifierI(), nullPermVerifId)->ice_collocationOptimized(true));

    Identity nullSSLPermVerifId;
    nullSSLPermVerifId.category = _instanceName;
    nullSSLPermVerifId.name = "NullSSLPermissionsVerifier";
    _nullSSLPermissionsVerifier = Glacier2::SSLPermissionsVerifierPrx::uncheckedCast(
        registryAdapter->add(new NullSSLPermissionsVerifierI(), nullSSLPermVerifId)->ice_collocationOptimized(true));
}

bool
RegistryI::setupUserAccountMapper(const Ice::ObjectAdapterPtr& registryAdapter)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    //
    // Setup file user account mapper object if the property is set.
    //
    string userAccountFileProperty = properties->getProperty("IceGrid.Registry.UserAccounts");
    if(!userAccountFileProperty.empty())
    {
        try
        {
            Identity mapperId;
            mapperId.category = _instanceName;
            mapperId.name = "RegistryUserAccountMapper";
            if(!_master)
            {
                mapperId.name += "-" + _replicaName;
            }
            registryAdapter->add(new FileUserAccountMapperI(userAccountFileProperty), mapperId);
            _wellKnownObjects->add(registryAdapter->createProxy(mapperId), UserAccountMapper::ice_staticId());
        }
        catch(const std::string& msg)
        {
            Error out(_communicator->getLogger());
            out << msg;
            return false;
        }
    }
    return true;
}

void
RegistryI::setupClientSessionFactory(const Ice::ObjectAdapterPtr& registryAdapter,
                                     const Ice::ObjectAdapterPtr& sessionManagerAdapter,
                                     const IceGrid::LocatorPrx& locator,
                                     bool nowarn)
{
    _waitQueue = new WaitQueue(); // Used for for session allocation timeout.
    _waitQueue->start();
    
    assert(_reaper);
    _clientSessionFactory = new ClientSessionFactory(sessionManagerAdapter, _database, _waitQueue, _reaper);

    if(sessionManagerAdapter && _master) // Slaves don't support client session manager objects.
    {
        Identity clientSessionMgrId;
        clientSessionMgrId.category = _instanceName;
        clientSessionMgrId.name = "SessionManager";
        Identity sslClientSessionMgrId;
        sslClientSessionMgrId.category = _instanceName;
        sslClientSessionMgrId.name = "SSLSessionManager";

        sessionManagerAdapter->add(new ClientSessionManagerI(_clientSessionFactory), clientSessionMgrId);
        sessionManagerAdapter->add(new ClientSSLSessionManagerI(_clientSessionFactory), sslClientSessionMgrId);

        _wellKnownObjects->add(sessionManagerAdapter->createProxy(clientSessionMgrId), 
                               Glacier2::SessionManager::ice_staticId());
        
        _wellKnownObjects->add(sessionManagerAdapter->createProxy(sslClientSessionMgrId), 
                               Glacier2::SSLSessionManager::ice_staticId());
    }

    Ice::PropertiesPtr properties = _communicator->getProperties();

    _clientVerifier = getPermissionsVerifier(registryAdapter,
                                             locator,
                                             "IceGrid.Registry.PermissionsVerifier",
                                             properties->getProperty("IceGrid.Registry.CryptPasswords"), 
                                             nowarn);

    _sslClientVerifier = getSSLPermissionsVerifier(locator, 
                                                   "IceGrid.Registry.SSLPermissionsVerifier", 
                                                   nowarn);
}

void
RegistryI::setupAdminSessionFactory(const Ice::ObjectAdapterPtr& registryAdapter, 
                                    const Ice::ObjectAdapterPtr& sessionManagerAdapter,
                                    const IceGrid::LocatorPrx& locator,
                                    bool nowarn)
{
    assert(_reaper);
    _adminSessionFactory = new AdminSessionFactory(sessionManagerAdapter, _database, _reaper, this);

    if(sessionManagerAdapter)
    {
        Identity adminSessionMgrId;
        adminSessionMgrId.category = _instanceName;
        adminSessionMgrId.name = "AdminSessionManager";
        Identity sslAdmSessionMgrId;
        sslAdmSessionMgrId.category = _instanceName;
        sslAdmSessionMgrId.name = "AdminSSLSessionManager";
        if(!_master)
        {
            adminSessionMgrId.name += "-" + _replicaName;
            sslAdmSessionMgrId.name += "-" + _replicaName;
        }

        sessionManagerAdapter->add(new AdminSessionManagerI(_adminSessionFactory), adminSessionMgrId);
        sessionManagerAdapter->add(new AdminSSLSessionManagerI(_adminSessionFactory), sslAdmSessionMgrId);
        
        _wellKnownObjects->add(sessionManagerAdapter->createProxy(adminSessionMgrId), 
                               Glacier2::SessionManager::ice_staticId());
        _wellKnownObjects->add(sessionManagerAdapter->createProxy(sslAdmSessionMgrId), 
                               Glacier2::SSLSessionManager::ice_staticId());
    }

    Ice::PropertiesPtr properties = _communicator->getProperties();

    _adminVerifier = getPermissionsVerifier(registryAdapter,
                                            locator,
                                            "IceGrid.Registry.AdminPermissionsVerifier",
                                            properties->getProperty("IceGrid.Registry.AdminCryptPasswords"),
                                            nowarn);

    _sslAdminVerifier =
        getSSLPermissionsVerifier(locator, 
                                  "IceGrid.Registry.AdminSSLPermissionsVerifier", 
                                  nowarn);
}

void
RegistryI::stop()
{
    _session.destroy();
    
    //
    // We destroy the topics before to shutdown the communicator to
    // ensure that there will be no more invocations on IceStorm once
    // it's shutdown.
    //
    _database->destroyTopics();

    try
    {
        _communicator->shutdown();
        _communicator->waitForShutdown();
    }
    catch(const Ice::LocalException& ex)
    {
        Warning out(_communicator->getLogger());
        out << "unexpected exception while shutting down registry communicator:\n" << ex;
    }    

    if(_reaper)
    {
        _reaper->terminate();
        _reaper->getThreadControl().join();
        _reaper = 0;
    }

    if(_waitQueue)
    {
        _waitQueue->destroy();
        _waitQueue = 0;
    }

    if(_iceStorm)
    {
        _iceStorm->stop();
        _iceStorm = 0;
    }

    _wellKnownObjects = 0;
    _clientSessionFactory = 0;
    _adminSessionFactory = 0;
    _database = 0;
}

SessionPrx
RegistryI::createSession(const string& user, const string& password, const Current& current)
{
    if(!_master)
    {
        PermissionDeniedException ex;
        ex.reason = "client session creation is only allowed with the master registry.";
        throw ex;
    }

    assert(_reaper && _clientSessionFactory);

    if(!_clientVerifier)
    {
        PermissionDeniedException ex;
        ex.reason = "no permissions verifier configured, use the property\n";
        ex.reason += "`IceGrid.Registry.PermissionsVerifier' to configure\n";
        ex.reason += "a permissions verifier.";
        throw ex;
    }

    if(user.empty())
    {
        PermissionDeniedException ex;
        ex.reason = "empty user id";
        throw ex;
    }

    try
    {
        string reason;
        if(!_clientVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            PermissionDeniedException exc;
            exc.reason = reason;
            throw exc;
        }
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with client permission verifier:\n" << ex;
        }

        PermissionDeniedException exc;
        exc.reason = "internal server error";
        throw exc;
    }

    SessionIPtr session = _clientSessionFactory->createSessionServant(user, 0);
    Ice::ObjectPrx proxy = session->registerWithServantLocator(_sessionServantLocator, current.con);
    if(_sessionTimeout > 0)
    {
        _reaper->add(new SessionReapable<SessionI>(_traceLevels->logger, session), _sessionTimeout);
    }
    return SessionPrx::uncheckedCast(proxy);
}

AdminSessionPrx
RegistryI::createAdminSession(const string& user, const string& password, const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if(!_adminVerifier)
    {
        PermissionDeniedException ex;
        ex.reason = "no admin permissions verifier configured, use the property\n";
        ex.reason += "`IceGrid.Registry.AdminPermissionsVerifier' to configure\n";
        ex.reason += "a permissions verifier.";
        throw ex;
    }

    if(user.empty())
    {
        PermissionDeniedException ex;
        ex.reason = "empty user id";
        throw ex;
    }

    try
    {
        string reason;
        if(!_adminVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            PermissionDeniedException exc;
            exc.reason = reason;
            throw exc;
        }
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with admin permission verifier:\n" << ex;
        }

        PermissionDeniedException exc;
        exc.reason = "internal server error";
        throw exc;
    }

    AdminSessionIPtr session = _adminSessionFactory->createSessionServant(user);
    Ice::ObjectPrx proxy = session->registerWithServantLocator(_sessionServantLocator, current.con, this);
    if(_sessionTimeout > 0)
    {
        _reaper->add(new SessionReapable<AdminSessionI>(_traceLevels->logger, session), _sessionTimeout);
    }
    return AdminSessionPrx::uncheckedCast(proxy); 
}

SessionPrx
RegistryI::createSessionFromSecureConnection(const Current& current)
{
    if(!_master)
    {
        PermissionDeniedException ex;
        ex.reason = "client session creation is only allowed with the master registry.";
        throw ex;
    }
    
    assert(_reaper && _clientSessionFactory);

    if(!_sslClientVerifier)
    {
        PermissionDeniedException ex;
        ex.reason = "no ssl permissions verifier configured, use the property\n";
        ex.reason += "`IceGrid.Registry.SSLPermissionsVerifier' to configure\n";
        ex.reason += "a permissions verifier.";
        throw ex;
    }

    string userDN;
    Glacier2::SSLInfo info = getSSLInfo(current.con, userDN);
    if(userDN.empty())
    {
        PermissionDeniedException ex;
        ex.reason = "empty user DN";
        throw ex;
    }

    try
    {
        string reason;
        if(!_sslClientVerifier->authorize(info, reason, current.ctx))
        {
            PermissionDeniedException exc;
            exc.reason = reason;
            throw exc;
        }
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with SSL client permission verifier:\n" << ex;
        }

        PermissionDeniedException exc;
        exc.reason = "internal server error";
        throw exc;
    }

    SessionIPtr session = _clientSessionFactory->createSessionServant(userDN, 0);
    Ice::ObjectPrx proxy = session->registerWithServantLocator(_sessionServantLocator, current.con);
    if(_sessionTimeout > 0)
    {
        _reaper->add(new SessionReapable<SessionI>(_traceLevels->logger, session), _sessionTimeout);
    }
    return SessionPrx::uncheckedCast(proxy);
}

AdminSessionPrx
RegistryI::createAdminSessionFromSecureConnection(const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if(!_sslAdminVerifier)
    {
        PermissionDeniedException ex;
        ex.reason = "no ssl admin permissions verifier configured, use the property\n";
        ex.reason += "`IceGrid.Registry.AdminSSLPermissionsVerifier' to configure\n";
        ex.reason += "a permissions verifier.";
        throw ex;
    }

    string userDN;
    Glacier2::SSLInfo info = getSSLInfo(current.con, userDN);
    try
    {
        string reason;
        if(!_sslAdminVerifier->authorize(info, reason, current.ctx))
        {
            PermissionDeniedException exc;
            exc.reason = reason;
            throw exc;
        }
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with SSL admin permission verifier:\n" << ex;
        }

        PermissionDeniedException exc;
        exc.reason = "internal server error";
        throw exc;
    }
    
    //
    // We let the connection access the administrative interface.
    //
    AdminSessionIPtr session = _adminSessionFactory->createSessionServant(userDN);
    Ice::ObjectPrx proxy = session->registerWithServantLocator(_sessionServantLocator, current.con, this);
    if(_sessionTimeout > 0)
    {
        _reaper->add(new SessionReapable<AdminSessionI>(_traceLevels->logger, session), _sessionTimeout);
    }
    return AdminSessionPrx::uncheckedCast(proxy);
}

int
RegistryI::getSessionTimeout(const Ice::Current& current) const
{
    return _sessionTimeout;
}

string
RegistryI::getName() const
{
    return _replicaName;
}

RegistryInfo
RegistryI::getInfo() const
{
    return _platform.getRegistryInfo();
}

void
RegistryI::waitForShutdown()
{
    assert(_clientAdapter);
    _clientAdapter->waitForDeactivate();
}

void
RegistryI::shutdown()
{
    assert(_clientAdapter);
    _clientAdapter->deactivate();
}

void
RegistryI::setupThreadPool(const PropertiesPtr& properties, const string& name, int size, int sizeMax)
{
    if(properties->getPropertyAsIntWithDefault(name + ".Size", 0) < size)
    {
        ostringstream os;
        os << size;
        properties->setProperty(name + ".Size", os.str());
    }
    else
    {
        size = properties->getPropertyAsInt(name + ".Size");
    }

    if(sizeMax > 0 && properties->getPropertyAsIntWithDefault(name + ".SizeMax", 0) < sizeMax)
    {
        if(size >= sizeMax)
        {
            sizeMax = size * 10;
        }
        
        ostringstream os;
        os << sizeMax;
        properties->setProperty(name + ".SizeMax", os.str());
    }
}

Glacier2::PermissionsVerifierPrx
RegistryI::getPermissionsVerifier(const ObjectAdapterPtr& adapter, 
                                  const IceGrid::LocatorPrx& locator,
                                  const string& verifierProperty,
                                  const string& passwordsProperty,
                                  bool nowarn)
{
    //
    // Get the permissions verifier, or create a default one if no
    // verifier is specified.
    //

    ObjectPrx verifier;
    string verifierPropertyValue = _communicator->getProperties()->getProperty(verifierProperty);
    if(!verifierPropertyValue.empty())
    {
        try
        {
            verifier = _communicator->propertyToProxy(verifierProperty);
            assert(_nullPermissionsVerifier);
            if(verifier->ice_getIdentity() == _nullPermissionsVerifier->ice_getIdentity())
            {
                verifier = _nullPermissionsVerifier;
            }
        }
        catch(const LocalException& ex)
        {
            Error out(_communicator->getLogger());
            out << "permissions verifier `" + verifierPropertyValue + "' is invalid:\n" << ex;
            return 0;
        }
    }
    else if(!passwordsProperty.empty())
    {
        ifstream passwordFile(passwordsProperty.c_str());
        if(!passwordFile)
        {
            Error out(_communicator->getLogger());
            string err = strerror(errno);
            out << "cannot open `" + passwordsProperty + "' for reading: " + err;
            return 0;
        }

        map<string, string> passwords;

        while(true)
        {
            string userId;
            passwordFile >> userId;
            if(!passwordFile)
            {
                break;
            }

            string password;
            passwordFile >> password;
            if(!passwordFile)
            {
                break;
            }

            assert(!userId.empty());
            assert(!password.empty());
            passwords.insert(make_pair(userId, password));
        }

        verifier = adapter->addWithUUID(new CryptPermissionsVerifierI(passwords));
    }
    else
    {
        return 0;
    }

    assert(verifier);

    Glacier2::PermissionsVerifierPrx verifierPrx;
    try
    {
        //
        // Set the permission verifier proxy locator to the internal
        // locator. We can't use the "public" locator, this could lead
        // to deadlocks if there's not enough threads in the client
        // thread pool anymore.
        //
        verifierPrx = Glacier2::PermissionsVerifierPrx::checkedCast(verifier->ice_locator(locator));
        if(!verifierPrx)
        {
            Error out(_communicator->getLogger());
            out << "permissions verifier `" + verifierProperty + "' is invalid";
            return 0;
        }    
    }
    catch(const LocalException& ex)
    {
        if(!nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact permissions verifier `" + verifierProperty + "':\n" << ex;
        }
        verifierPrx = Glacier2::PermissionsVerifierPrx::uncheckedCast(verifier->ice_locator(locator));
    }
    return verifierPrx;
}

Glacier2::SSLPermissionsVerifierPrx
RegistryI::getSSLPermissionsVerifier(const IceGrid::LocatorPrx& locator, const string& verifierProperty, bool nowarn)
{
    //
    // Get the permissions verifier, or create a default one if no
    // verifier is specified.
    //
    ObjectPrx verifier;
    string verifierPropertyValue = _communicator->getProperties()->getProperty(verifierProperty);
    if(!verifierPropertyValue.empty())
    {
        try
        {
            verifier = _communicator->propertyToProxy(verifierProperty);
            assert(_nullSSLPermissionsVerifier);
            if(verifier->ice_getIdentity() == _nullSSLPermissionsVerifier->ice_getIdentity())
            {
                verifier = _nullSSLPermissionsVerifier;
            }
        }
        catch(const LocalException& ex)
        {
            Error out(_communicator->getLogger());
            out << "permissions verifier `" + verifierPropertyValue + "' is invalid:\n" << ex;
            return 0;
        }
    }
    else
    {
        return 0;
    }

    Glacier2::SSLPermissionsVerifierPrx verifierPrx;
    try
    {
        //
        // Set the permission verifier proxy locator to the internal
        // locator. We can't use the "public" locator, this could lead
        // to deadlocks if there's not enough threads in the client
        // thread pool anymore.
        //
        verifierPrx = Glacier2::SSLPermissionsVerifierPrx::checkedCast(verifier->ice_locator(locator));
        if(!verifierPrx)
        {
            Error out(_communicator->getLogger());
            out << "permissions verifier `" + verifierProperty + "' is invalid";
            return 0;
        }    
    }
    catch(const LocalException& ex)
    {
        if(!nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact permissions verifier `" + verifierProperty + "':\n" << ex; 
        }
        verifierPrx = Glacier2::SSLPermissionsVerifierPrx::uncheckedCast(verifier->ice_locator(locator));
    }
    return verifierPrx;
}

Glacier2::SSLInfo
RegistryI::getSSLInfo(const ConnectionPtr& connection, string& userDN)
{
    Glacier2::SSLInfo sslinfo;
    try
    {
        IceSSL::ConnectionInfo info = IceSSL::getConnectionInfo(connection);
        sslinfo.remotePort = ntohs(info.remoteAddr.sin_port);
        sslinfo.remoteHost = IceInternal::inetAddrToString(info.remoteAddr.sin_addr);
        sslinfo.localPort = ntohs(info.localAddr.sin_port);
        sslinfo.localHost = IceInternal::inetAddrToString(info.localAddr.sin_addr);

        sslinfo.cipher = info.cipher;

        if(!info.certs.empty())
        {
            sslinfo.certs.resize(info.certs.size());
            for(unsigned int i = 0; i < info.certs.size(); ++i)
            {
                sslinfo.certs[i] = info.certs[i]->encode();
            }
            userDN = info.certs[0]->getSubjectDN();
        }
    }
    catch(const IceSSL::ConnectionInvalidException&)
    {
        PermissionDeniedException exc;
        exc.reason = "not ssl connection";
        throw exc;
    }
    catch(const IceSSL::CertificateEncodingException&)
    {
        PermissionDeniedException exc;
        exc.reason = "certificate encoding exception";
        throw exc;
    }

    return sslinfo;
}

NodePrxSeq
RegistryI::registerReplicas(const InternalRegistryPrx& internalRegistry, 
                            const InternalRegistryPrxSeq& replicas,
                            const NodePrxSeq& dbNodes)
{
    set<NodePrx> nodes;
    nodes.insert(dbNodes.begin(), dbNodes.end());

    for(InternalRegistryPrxSeq::const_iterator r = replicas.begin(); r != replicas.end(); ++r)
    {
        if((*r)->ice_getIdentity() != internalRegistry->ice_getIdentity())
        {
            string replicaName;
            if(_traceLevels && _traceLevels->replica > 1)
            {
                replicaName = (*r)->ice_getIdentity().name;
                const string prefix("InternalRegistry-");
                string::size_type pos = replicaName.find(prefix);
                if(pos != string::npos)
                {
                    replicaName = replicaName.substr(prefix.size());
                }

                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "creating replica `" << replicaName << "' session";
            }

            try
            {
                (*r)->registerWithReplica(internalRegistry);
                NodePrxSeq nds = (*r)->getNodes();
                nodes.insert(nds.begin(), nds.end());

                if(_traceLevels && _traceLevels->replica > 1)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                    out << "replica `" << replicaName << "' session created";
                }
            }
            catch(const Ice::LocalException& ex)
            {
                //
                // Clear the proxy from the database if we can't
                // contact the replica.
                //
                try
                {
                    _database->removeObject((*r)->ice_getIdentity());
                }
                catch(const ObjectNotRegisteredException&)
                {
                }

                if(_traceLevels && _traceLevels->replica > 1)
                {
                    Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                    out << "replica `" << replicaName << "' session creation failed:\n" << ex;
                }
            }
        }
    }
    
#ifdef _RWSTD_NO_MEMBER_TEMPLATES
    NodePrxSeq result;
    for(set<NodePrx>::iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
        result.push_back(*p);
    }
    return result;
#else
    return NodePrxSeq(nodes.begin(), nodes.end());
#endif
}

void
RegistryI::registerNodes(const InternalRegistryPrx& internalRegistry, const NodePrxSeq& nodes)
{
    const string prefix("Node-");

    for(NodePrxSeq::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
        assert((*p)->ice_getIdentity().name.find(prefix) != string::npos);
        try
        {
            _database->getNode((*p)->ice_getIdentity().name.substr(prefix.size()))->setProxy(*p);
        }
        catch(const NodeNotExistException&)
        {
            //
            // Ignore, if nothing's deployed on the node we won't need
            // to contact it for locator requests so we don't need to
            // keep its proxy.
            //
            try
            {
                _database->removeInternalObject((*p)->ice_getIdentity());
            }
            catch(const ObjectNotRegisteredException&)
            {
            }
        }
    }
}
