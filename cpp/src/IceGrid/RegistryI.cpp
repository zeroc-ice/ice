// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/UUID.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <Ice/Network.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname

#include <IceStorm/Service.h>
#include <IceSSL/IceSSL.h>
#include <Glacier2/PermissionsVerifier.h>

#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Discovery.h>
#include <IceGrid/RegistryI.h>
#include <IceGrid/LocatorI.h>
#include <IceGrid/LocatorRegistryI.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/QueryI.h>
#include <IceGrid/SessionI.h>
#include <IceGrid/AdminCallbackRouter.h>
#include <IceGrid/AdminSessionI.h>
#include <IceGrid/InternalRegistryI.h>
#include <IceGrid/SessionServantManager.h>
#include <IceGrid/FileUserAccountMapperI.h>
#include <IceGrid/WellKnownObjectsManager.h>
#include <IceGrid/FileCache.h>

#include <IceGrid/RegistryServerAdminRouter.h>

#include <fstream>

#include <openssl/des.h> // For crypt() passwords

#include <sys/types.h>

// Ignore OS X OpenSSL deprecation warnings
#ifdef __APPLE__
#pragma GCC diagnostic ignored "-Wdeprecated-declarations"
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace
{

class NullPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(const string& /*userId*/, const string& /*password*/, string&, const Current&) const
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

class DefaultServantLocator : public Ice::ServantLocator
{
public:

    DefaultServantLocator(const ObjectPtr& servant) :
        _servant(servant)
    {
    }

    virtual ObjectPtr locate(const Current&, LocalObjectPtr&)
    {
        return _servant;
    }

    virtual void finished(const Current&, const ObjectPtr&, const LocalObjectPtr&)
    {
    }

    virtual void deactivate(const string&)
    {
    }

private:
    ObjectPtr _servant;
};

class LookupI : public IceGrid::Lookup
{
public:
    
    LookupI(const std::string& instanceName, const WellKnownObjectsManagerPtr& wellKnownObjects) : 
        _instanceName(instanceName), _wellKnownObjects(wellKnownObjects)
    {
    }
    
    virtual void
    findLocator(const string& instanceName, const LookupReplyPrx& reply, const Ice::Current&)
    {
        if(!instanceName.empty() && instanceName != _instanceName)
        {
            return; // Ignore.
        }

        if(reply)
        {
            reply->begin_foundLocator(_wellKnownObjects->getLocator());
        }
    }

    virtual IceGrid::LocatorPrx
    getLocator(const Ice::Current&)
    {
        return _wellKnownObjects->getLocator();
    }

private:

    const string _instanceName;
    const WellKnownObjectsManagerPtr _wellKnownObjects;
};

class FinderI : public Ice::LocatorFinder
{
public:
    
    FinderI(const WellKnownObjectsManagerPtr& wellKnownObjects) : _wellKnownObjects(wellKnownObjects)
    {
    }
    
    virtual Ice::LocatorPrx
    getLocator(const Ice::Current&)
    {
        return _wellKnownObjects->getLocator();
    }

private:

    const WellKnownObjectsManagerPtr _wellKnownObjects;
};

}

RegistryI::RegistryI(const CommunicatorPtr& communicator,
                     const TraceLevelsPtr& traceLevels,
                     bool nowarn,
                     bool readonly,
                     const string& initFromReplica) : 
    _communicator(communicator),
    _traceLevels(traceLevels),
    _nowarn(nowarn),
    _readonly(readonly),
    _initFromReplica(initFromReplica),
    _platform("IceGrid.Registry", communicator, traceLevels)
{
}

RegistryI::~RegistryI()
{
}

bool
RegistryI::start()
{
    try
    {
        if(!startImpl())
        {
            stop();
            return false;
        }
    }
    catch(...)
    {
        stop();
        throw;
    }
    return true;
}

bool
RegistryI::startImpl()
{
    assert(_communicator);
    PropertiesPtr properties = _communicator->getProperties();

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
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "session manager endpoints `IceGrid.Registry.SessionManager.Endpoints' enabled";
            if(properties->getPropertyAsInt("IceGrid.Registry.SessionFilters") == 0)
            {
                out << " (with Glacier2 filters disabled)";
            }
        }
    }

    if(!properties->getProperty("IceGrid.Registry.AdminSessionManager.Endpoints").empty())
    {
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "administrative session manager endpoints `IceGrid.Registry.AdminSessionManager.Endpoints' enabled";
            if(properties->getPropertyAsInt("IceGrid.Registry.AdminSessionFilters") == 0)
            {
                out << " (with Glacier2 filters disabled)";
            }
        }
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IceGrid.Registry.Client.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Server.AdapterId", "");
    properties->setProperty("IceGrid.Registry.SessionManager.AdapterId", "");
    properties->setProperty("IceGrid.Registry.AdminSessionManager.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Internal.AdapterId", "");

    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10, true); // Serialize for admin callbacks
    setupThreadPool(properties, "IceGrid.Registry.SessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.AdminSessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _replicaName = properties->getPropertyWithDefault("IceGrid.Registry.ReplicaName", "Master");
    _master = _replicaName == "Master";
    _sessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionTimeout", 30);

    if(!_initFromReplica.empty() && (_initFromReplica == _replicaName || (_master && _initFromReplica == "Master")))
    {
        Error out(_communicator->getLogger());
        out << "invalid --initdb-from-replica option: identical replica";
        return false; 
    }

    if(_sessionTimeout > 0 && properties->getProperty("IceGrid.Registry.Client.ACM.Timemout").empty())
    {
        ostringstream os;
        os << _sessionTimeout;
        properties->setProperty("IceGrid.Registry.Client.ACM.Timeout", os.str());
    }

    if(properties->getProperty("IceGrid.Registry.Server.ACM.Timeout").empty())
    {
        properties->setProperty("IceGrid.Registry.Server.ACM.Timeout", "30");
    }

    if(properties->getProperty("IceGrid.Registry.Internal.ACM.Timeout").empty())
    {
        int nt = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 30);
        int rt = properties->getPropertyAsIntWithDefault("IceGrid.Registry.ReplicaSessionTimeout", 30);
        ostringstream os;
        os << std::max(nt, rt);
        properties->setProperty("IceGrid.Registry.Internal.ACM.Timeout", os.str());
    }

    properties->setProperty("Ice.ACM.Server.Close", "3"); // Close on invocation and idle.
    
    if(!_master && !_communicator->getDefaultLocator())
    {
        Error out(_communicator->getLogger());
        out << "property `Ice.Default.Locator' is not set";
        return false;
    }
    else if(_master)
    {
        _communicator->setDefaultLocator(0); // Clear the default locator in case it's set.
    }

    //
    // Get the instance name
    //
    _instanceName = properties->getProperty("IceGrid.InstanceName");    
    if(_instanceName.empty())
    {
        _instanceName = properties->getProperty("IceGridDiscovery.InstanceName");
    }
    if(_instanceName.empty() && _communicator->getDefaultLocator())
    {
        _instanceName = _communicator->getDefaultLocator()->ice_getIdentity().category;
    }
    if(_instanceName.empty())
    {
        _instanceName = "IceGrid";
    }

    //
    // Create the replica session manager
    //
    _session.reset(new ReplicaSessionManager(_communicator, _instanceName));

    //
    // Create the registry database.
    //
    string dbPath = _communicator->getProperties()->getProperty("IceGrid.Registry.Data");
    if(dbPath.empty())
    {
        Ice::Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Data' is not set";
        return false;
    }
    else
    {
        if(!IceUtilInternal::directoryExists(dbPath))
        {
            Ice::SyscallException ex(__FILE__, __LINE__);
            ex.error = IceInternal::getSystemErrno();

            Ice::Error out(_communicator->getLogger());
            out << "property `IceGrid.Registry.Data' is set to an invalid path:\n" << ex;
            return false;
        }
    }
    _communicator->getProperties()->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
    const string envName = "Registry";
    Freeze::ConnectionPtr connection = Freeze::createConnection(_communicator, envName);
    
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

    //
    // Create the reaper thread.
    //
    _reaper = new ReapThread();
    _reaper->start();

    //
    // Create the internal registry object adapter.
    //
    _registryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Internal");
    _registryAdapter->activate();

    //
    // Create the internal IceStorm service.
    //
    Identity registryTopicManagerId;
    registryTopicManagerId.category = _instanceName;
    registryTopicManagerId.name = "RegistryTopicManager";
    _iceStorm = IceStormInternal::Service::create(_communicator, 
                                                  _registryAdapter, 
                                                  _registryAdapter, 
                                                  "IceGrid.Registry", 
                                                  registryTopicManagerId,
                                                  envName);
    const IceStorm::TopicManagerPrx topicManager = _iceStorm->getTopicManager();

    _database = new Database(_registryAdapter, topicManager, _instanceName, _traceLevels, getInfo(), connection, 
                             "Registry", _readonly);
    _wellKnownObjects = new WellKnownObjectsManager(_database);

    if(!_initFromReplica.empty())
    {
        Ice::Identity id;
        id.category = _instanceName;
        id.name = (_initFromReplica == "Master") ? "Registry" : "Registry-" + _initFromReplica;
        Ice::ObjectPrx proxy;
        try
        {
            proxy = _database->getObjectProxy(id);
            id.name = "Query";
            IceGrid::QueryPrx query = IceGrid::QueryPrx::uncheckedCast(proxy->ice_identity(id));
            if(query)
            {
                id.name = "InternalRegistry-" + _initFromReplica;
                try
                {
                    proxy = query->findObjectById(id);
                }
                catch(const Ice::Exception&)
                {
                }
            }
        }
        catch(const ObjectNotRegisteredException&)
        {
            id.name = "InternalRegistry-" + _initFromReplica;
            try
            {
                proxy = _database->getObjectProxy(id);
            }
            catch(const ObjectNotRegisteredException&)
            {
            }
        }

        // If we still didn't find the replica proxy, check with the
        // locator or the IceGrid.Registry.ReplicaEndpoints properties
        // if we can find it.
        if(!proxy)
        {
            id.name = "InternalRegistry-" + (_initFromReplica.empty() ? "Master" : _initFromReplica);
            proxy = _session->findInternalRegistryForReplica(id);
        }

        if(!proxy)
        {
            Error out(_communicator->getLogger());
            out << "couldn't find replica `" << _initFromReplica << "' to\n";
            out << "initialize the database (specify the replica endpoints in the endpoints of\n";
            out << "the `Ice.Default.Locator' proxy property to allow finding the replica)";
            return false;
        }

        try
        {
            Ice::Long serial;
            IceGrid::InternalRegistryPrx registry = IceGrid::InternalRegistryPrx::checkedCast(proxy);
	    ApplicationInfoSeq applications = registry->getApplications(serial);
            _database->syncApplications(applications, serial);
	    AdapterInfoSeq adapters = registry->getAdapters(serial);
            _database->syncAdapters(adapters, serial);
	    ObjectInfoSeq objects = registry->getObjects(serial);
            _database->syncObjects(objects, serial);
        }
        catch(const Ice::OperationNotExistException&)
        {
            Error out(_communicator->getLogger());
            out << "couldn't initialize database from replica `" << _initFromReplica << "':\n";
            out << "replica doesn't support this functionality (IceGrid < 3.5.1)";
            return false;
        }
        catch(const Ice::Exception& ex)
        {
            Error out(_communicator->getLogger());
            out << "couldn't initialize database from replica `" << _initFromReplica << "':\n";
            out << ex;
            return false;
        }
    }

    //
    // Get proxies for nodes that we were connected with on last
    // shutdown.
    //
    NodePrxSeq nodes;
    ObjectProxySeq proxies = _database->getInternalObjectsByType(Node::ice_staticId());
    for(ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        nodes.push_back(NodePrx::uncheckedCast(*p));
    }

    //
    // NOTE: The internal registry object must be added only once the
    // node/replica proxies are retrieved. Otherwise, if some
    // replica/node register as soon as the internal registry is setup
    // we might clear valid proxies.
    //
    InternalRegistryPrx internalRegistry = setupInternalRegistry();
    if(_master)
    {
        nodes = registerReplicas(internalRegistry, nodes);
        registerNodes(internalRegistry, nodes);
    }
    else
    {
        InternalReplicaInfoPtr info = _platform.getInternalReplicaInfo();
        _session->create(_replicaName, info, _database, _wellKnownObjects, internalRegistry);
        registerNodes(internalRegistry, _session->getNodes(nodes));
    }

    _serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    _clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");

    Ice::Identity dummy;
    dummy.name = "dummy";
    _wellKnownObjects->addEndpoint("Client", _clientAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Server", _serverAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Internal", _registryAdapter->createDirectProxy(dummy));

    setupNullPermissionsVerifier();
    if(!setupUserAccountMapper())
    {
        return false;
    }

    QueryPrx query = setupQuery();
    RegistryPrx registry = setupRegistry();

    Ice::LocatorRegistryPrx locatorRegistry = setupLocatorRegistry();
    LocatorPrx internalLocator = setupLocator(locatorRegistry, registry, query);

    //
    // Create the session servant manager. The session servant manager is responsible
    // for managing sessions servants and to ensure that session servants are only 
    // accessed by the connection that created the session. The session servant manager
    // also takes care of providing the router servant for server admin objects.
    //
    ObjectPtr serverAdminRouter = new RegistryServerAdminRouter(_database);
    AdminCallbackRouterPtr adminCallbackRouter = new AdminCallbackRouter;

    _servantManager = new SessionServantManager(_clientAdapter, _instanceName, true, getServerAdminCategory(), 
                                                serverAdminRouter, adminCallbackRouter);

    _clientAdapter->addServantLocator(_servantManager, "");
    _serverAdapter->addServantLocator(new DefaultServantLocator(adminCallbackRouter), "");
    
    ObjectAdapterPtr sessionAdpt = setupClientSessionFactory(internalLocator);
    ObjectAdapterPtr admSessionAdpt = setupAdminSessionFactory(serverAdminRouter, internalLocator);

    _wellKnownObjects->finish();
    if(_master)
    {
        _wellKnownObjects->registerAll();
    }
    else
    {
        _session->registerAllWellKnownObjects();
    }

    //
    // Add the locator finder object to the client adapter.
    //
    _clientAdapter->add(new FinderI(_wellKnownObjects), _communicator->stringToIdentity("Ice/LocatorFinder"));

    //
    // Setup the discovery object adapter and also add it the lookup
    // servant to receive multicast lookup queries.
    //
    Ice::ObjectAdapterPtr discoveryAdapter;
    if(properties->getPropertyAsIntWithDefault("IceGrid.Registry.Discovery.Enabled", 1) > 0)
    {
        bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        string address;
        if(ipv4)
        {
            address = properties->getPropertyWithDefault("IceGrid.Registry.Discovery.Address", "239.255.0.1");
        }
        else
        {
            address = properties->getPropertyWithDefault("IceGrid.Registry.Discovery.Address", "ff15::1");
        }
        int port = properties->getPropertyAsIntWithDefault("IceGrid.Registry.Discovery.Port", 4061);
        string interface = properties->getProperty("IceGrid.Registry.Discovery.Interface");
        if(properties->getProperty("IceGrid.Registry.Discovery.Endpoints").empty())
        {
            ostringstream os;
            os << "udp -h \"" << address << "\" -p " << port;
            if(!interface.empty())
            {
                os << " --interface \"" << interface << "\"";
            }
            properties->setProperty("IceGrid.Registry.Discovery.Endpoints", os.str());
        }

        try
        {
            Ice::Identity lookupId = _communicator->stringToIdentity("IceGrid/Lookup");
            discoveryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Discovery");
            discoveryAdapter->add(new LookupI(_instanceName, _wellKnownObjects), lookupId);
        }
        catch(const Ice::LocalException& ex)
        {
            if(!_nowarn)
            {
                Warning out(_communicator->getLogger());
                out << "failed to join the multicast group for IceGrid discovery:\n";
                out << "endpoints = " << properties->getProperty("IceGrid.Registry.Discovery.Endpoints") << "\n";
                out << ex;
            }
        }
    }
    
    //
    // We are ready to go!
    //
    _serverAdapter->activate();
    _clientAdapter->activate();
    if(discoveryAdapter)
    {
        discoveryAdapter->activate();
    }
  
    if(sessionAdpt)
    {
        sessionAdpt->activate();
    }
    if(admSessionAdpt)
    {
        admSessionAdpt->activate();
    }

    return true;
}

Ice::LocatorRegistryPrx
RegistryI::setupLocatorRegistry()
{
    bool dynReg = _communicator->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegId;
    locatorRegId.category = _instanceName;
    locatorRegId.name = "LocatorRegistry-" + _replicaName;
    ObjectPrx obj = _serverAdapter->add(new LocatorRegistryI(_database, dynReg, _master, *_session), locatorRegId);
    return LocatorRegistryPrx::uncheckedCast(obj);
}

IceGrid::LocatorPrx
RegistryI::setupLocator(const Ice::LocatorRegistryPrx& locatorRegistry,
                        const RegistryPrx& registry,
                        const QueryPrx& query)
{
    LocatorPtr locator = new LocatorI(_communicator, _database, locatorRegistry, registry, query);
    Identity locatorId;
    locatorId.category = _instanceName;

    locatorId.name = "Locator";
    _clientAdapter->add(locator, locatorId);

    locatorId.name = "Locator-" + _replicaName;
    _clientAdapter->add(locator, locatorId);
    
    return LocatorPrx::uncheckedCast(_registryAdapter->addWithUUID(locator));
}

QueryPrx
RegistryI::setupQuery()
{
    Identity queryId;
    queryId.category = _instanceName;
    queryId.name = "Query";
    return QueryPrx::uncheckedCast(_clientAdapter->add(new QueryI(_communicator, _database), queryId));
}

RegistryPrx
RegistryI::setupRegistry()
{
    Identity registryId;
    registryId.category = _instanceName;
    registryId.name = "Registry";
    if(!_master)
    {
        registryId.name += "-" + _replicaName;
    }
    RegistryPrx proxy = RegistryPrx::uncheckedCast(_clientAdapter->add(this, registryId));
    _wellKnownObjects->add(proxy, Registry::ice_staticId());
    return proxy;
}

InternalRegistryPrx
RegistryI::setupInternalRegistry()
{
    Identity internalRegistryId;
    internalRegistryId.category = _instanceName;
    internalRegistryId.name = "InternalRegistry-" + _replicaName;
    assert(_reaper);
    ObjectPtr internalRegistry = new InternalRegistryI(this, _database, _reaper, _wellKnownObjects, *_session);
    Ice::ObjectPrx proxy = _registryAdapter->add(internalRegistry, internalRegistryId);
    _wellKnownObjects->add(proxy, InternalRegistry::ice_staticId());

    InternalRegistryPrx registry = InternalRegistryPrx::uncheckedCast(proxy);
    _database->getReplicaCache().setInternalRegistry(registry);
    return registry;
}

void
RegistryI::setupNullPermissionsVerifier()
{
    Identity nullPermVerifId;
    nullPermVerifId.category = _instanceName;
    nullPermVerifId.name = "NullPermissionsVerifier";
    _nullPermissionsVerifier = Glacier2::PermissionsVerifierPrx::uncheckedCast(
        _registryAdapter->add(new NullPermissionsVerifierI(), nullPermVerifId));

    Identity nullSSLPermVerifId;
    nullSSLPermVerifId.category = _instanceName;
    nullSSLPermVerifId.name = "NullSSLPermissionsVerifier";
    _nullSSLPermissionsVerifier = Glacier2::SSLPermissionsVerifierPrx::uncheckedCast(
        _registryAdapter->add(new NullSSLPermissionsVerifierI(), nullSSLPermVerifId));
}

bool
RegistryI::setupUserAccountMapper()
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
            _registryAdapter->add(new FileUserAccountMapperI(userAccountFileProperty), mapperId);
            _wellKnownObjects->add(_registryAdapter->createProxy(mapperId), UserAccountMapper::ice_staticId());
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

Ice::ObjectAdapterPtr
RegistryI::setupClientSessionFactory(const IceGrid::LocatorPrx& locator)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    Ice::ObjectAdapterPtr adapter;
    SessionServantManagerPtr servantManager;
    if(!properties->getProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.SessionManager");
        servantManager = new SessionServantManager(adapter, _instanceName, false, "", 0, 0);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _timer = new IceUtil::Timer();  // Used for session allocation timeout.
    _clientSessionFactory = new ClientSessionFactory(servantManager, _database, _timer, _reaper);

    if(servantManager && _master) // Slaves don't support client session manager objects.
    {
        Identity sessionMgrId;
        sessionMgrId.category = _instanceName;
        sessionMgrId.name = "SessionManager";
        Identity sslSessionMgrId;
        sslSessionMgrId.category = _instanceName;
        sslSessionMgrId.name = "SSLSessionManager";

        adapter->add(new ClientSessionManagerI(_clientSessionFactory), sessionMgrId);
        adapter->add(new ClientSSLSessionManagerI(_clientSessionFactory), sslSessionMgrId);

        _wellKnownObjects->add(adapter->createProxy(sessionMgrId), Glacier2::SessionManager::ice_staticId());
        _wellKnownObjects->add(adapter->createProxy(sslSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
    }

    if(adapter)
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        _wellKnownObjects->addEndpoint("SessionManager", adapter->createDirectProxy(dummy));
    }

    _clientVerifier = getPermissionsVerifier(locator,
                                             "IceGrid.Registry.PermissionsVerifier",
                                             properties->getProperty("IceGrid.Registry.CryptPasswords"));

    _sslClientVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.SSLPermissionsVerifier");

    return adapter;
}

Ice::ObjectAdapterPtr
RegistryI::setupAdminSessionFactory(const Ice::ObjectPtr& router, const IceGrid::LocatorPrx& locator)
{
    Ice::PropertiesPtr properties = _communicator->getProperties();

    Ice::ObjectAdapterPtr adapter;
    SessionServantManagerPtr servantManager;
    if(!properties->getProperty("IceGrid.Registry.AdminSessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.AdminSessionManager");
        servantManager = new SessionServantManager(adapter, _instanceName, false, getServerAdminCategory(), router, 0);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _adminSessionFactory = new AdminSessionFactory(servantManager, _database, _reaper, this);

    if(servantManager)
    {
        Identity sessionMgrId;
        sessionMgrId.category = _instanceName;
        sessionMgrId.name = "AdminSessionManager";
        Identity sslSessionMgrId;
        sslSessionMgrId.category = _instanceName;
        sslSessionMgrId.name = "AdminSSLSessionManager";
        if(!_master)
        {
            sessionMgrId.name += "-" + _replicaName;
            sslSessionMgrId.name += "-" + _replicaName;
        }

        adapter->add(new AdminSessionManagerI(_adminSessionFactory), sessionMgrId);
        adapter->add(new AdminSSLSessionManagerI(_adminSessionFactory), sslSessionMgrId);
        
        _wellKnownObjects->add(adapter->createProxy(sessionMgrId), Glacier2::SessionManager::ice_staticId());
        _wellKnownObjects->add(adapter->createProxy(sslSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
    }

    if(adapter)
    {
        Ice::Identity dummy;
        dummy.name = "dummy";
        _wellKnownObjects->addEndpoint("AdminSessionManager", adapter->createDirectProxy(dummy));
    }

    _adminVerifier = getPermissionsVerifier(locator,
                                            "IceGrid.Registry.AdminPermissionsVerifier",
                                            properties->getProperty("IceGrid.Registry.AdminCryptPasswords"));

    _sslAdminVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.AdminSSLPermissionsVerifier");

    return adapter;
}

void
RegistryI::stop()
{
    if(_session.get())
    {
        _session->destroy();
    }

    //
    // We destroy the topics before to shutdown the communicator to
    // ensure that there will be no more invocations on IceStorm once
    // it's shutdown.
    //
    if(_database)
    {
        _database->destroy();
    }

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

    if(_timer)
    {
        _timer->destroy();
        _timer = 0;
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
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        PermissionDeniedException exc;
        exc.reason = ex.reason;
        throw exc;
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
    Ice::ObjectPrx proxy = session->_register(_servantManager, current.con);
    _reaper->add(new SessionReapable<SessionI>(_traceLevels->logger, session), _sessionTimeout);
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
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        PermissionDeniedException exc;
        exc.reason = ex.reason;
        throw exc;
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
    Ice::ObjectPrx proxy = session->_register(_servantManager, current.con);
    _reaper->add(new SessionReapable<AdminSessionI>(_traceLevels->logger, session), _sessionTimeout);
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
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        PermissionDeniedException exc;
        exc.reason = ex.reason;
        throw exc;
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
    Ice::ObjectPrx proxy = session->_register(_servantManager, current.con);
    _reaper->add(new SessionReapable<SessionI>(_traceLevels->logger, session), _sessionTimeout);
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
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        PermissionDeniedException exc;
        exc.reason = ex.reason;
        throw exc;
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
    Ice::ObjectPrx proxy = session->_register(_servantManager, current.con);
    _reaper->add(new SessionReapable<AdminSessionI>(_traceLevels->logger, session), _sessionTimeout);
    return AdminSessionPrx::uncheckedCast(proxy);
}

int
RegistryI::getSessionTimeout(const Ice::Current& /*current*/) const
{
    return _sessionTimeout;
}

int
RegistryI::getACMTimeout(const Ice::Current& current) const
{
    return current.con->getACM().timeout;
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

Ice::ObjectPrx 
RegistryI::createAdminCallbackProxy(const Identity& id) const
{
    return _serverAdapter->createProxy(id);
}

Ice::LocatorPrx 
RegistryI::getLocator()
{
    return _wellKnownObjects->getLocator();
}

Glacier2::PermissionsVerifierPrx
RegistryI::getPermissionsVerifier(const IceGrid::LocatorPrx& locator,
                                  const string& verifierProperty,
                                  const string& passwordsProperty)
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
            try
            {
                verifier = _communicator->propertyToProxy(verifierProperty);
            }
            catch(const ProxyParseException&)
            {
                //
                // Check if the property is just the identity of the null permissions verifier
                // (the identity might contain spaces which would prevent it to be parsed as a
                // proxy).
                //
                if(_communicator->stringToIdentity(verifierPropertyValue) ==
                   _nullPermissionsVerifier->ice_getIdentity())
                {
                    verifier = _communicator->stringToProxy("\"" + verifierPropertyValue + "\"");
                }
            }

            if(!verifier)
            {
                Error out(_communicator->getLogger());
                out << "permissions verifier `" + verifierPropertyValue + "' is invalid";
                return 0;
            }
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
        IceUtilInternal::ifstream passwordFile(passwordsProperty);
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

        verifier = _registryAdapter->addWithUUID(new CryptPermissionsVerifierI(passwords));
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
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact permissions verifier `" + verifierProperty + "':\n" << ex;
        }
        verifierPrx = Glacier2::PermissionsVerifierPrx::uncheckedCast(verifier->ice_locator(locator));
    }
    return verifierPrx;
}

Glacier2::SSLPermissionsVerifierPrx
RegistryI::getSSLPermissionsVerifier(const IceGrid::LocatorPrx& locator, const string& verifierProperty)
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
            try
            {
                verifier = _communicator->propertyToProxy(verifierProperty);
            }
            catch(const ProxyParseException&)
            {
                //
                // Check if the property is just the identity of the null permissions verifier
                // (the identity might contain spaces which would prevent it to be parsed as a
                // proxy).
                //
                if(_communicator->stringToIdentity(verifierPropertyValue) ==
                   _nullSSLPermissionsVerifier->ice_getIdentity())
                {
                    verifier = _communicator->stringToProxy("\"" + verifierPropertyValue + "\"");
                }
            }

            if(!verifier)
            {
                Error out(_communicator->getLogger());
                out << "ssl permissions verifier `" + verifierPropertyValue + "' is invalid";
                return 0;
            }
            assert(_nullSSLPermissionsVerifier);
            if(verifier->ice_getIdentity() == _nullSSLPermissionsVerifier->ice_getIdentity())
            {
                verifier = _nullSSLPermissionsVerifier;
            }
        }
        catch(const LocalException& ex)
        {
            Error out(_communicator->getLogger());
            out << "ssl permissions verifier `" + verifierPropertyValue + "' is invalid:\n" << ex;
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
            out << "ssl permissions verifier `" + verifierProperty + "' is invalid";
            return 0;
        }    
    }
    catch(const LocalException& ex)
    {
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact ssl permissions verifier `" + verifierProperty + "':\n" << ex; 
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
        IceSSL::ConnectionInfoPtr info = IceSSL::ConnectionInfoPtr::dynamicCast(connection->getInfo());
        if(!info)
        {
            PermissionDeniedException exc;
            exc.reason = "not ssl connection";
            throw exc;
        }

        sslinfo.remotePort = info->remotePort;
        sslinfo.remoteHost = info->remoteAddress;
        sslinfo.localPort = info->localPort;
        sslinfo.localHost = info->localAddress;
        sslinfo.cipher = info->cipher;
        sslinfo.certs = info->certs;
        if(info->certs.size() > 0)
        {
            userDN = IceSSL::Certificate::decode(info->certs[0])->getSubjectDN();
        }
    }
    catch(const IceSSL::CertificateEncodingException&)
    {
        PermissionDeniedException exc;
        exc.reason = "certificate encoding exception";
        throw exc;
    }
    catch(const Ice::LocalException&)
    {
        PermissionDeniedException exc;
        exc.reason = "connection exception";
        throw exc;
    }

    return sslinfo;
}

NodePrxSeq
RegistryI::registerReplicas(const InternalRegistryPrx& internalRegistry, const NodePrxSeq& dbNodes)
{
    //
    // Get proxies for slaves that we we connected with on last
    // shutdown.
    //
    // We first get the internal registry proxies and then also check
    // the public registry proxies. If we find public registry
    // proxies, we use indirect proxies setup with a locator using the
    // public proxy in preference over the internal proxy which might
    // contain stale endpoints if the slave was restarted. IceGrid
    // version <= 3.5.0 also kept the internal proxy in the database
    // instead of the public proxy.
    //
    map<InternalRegistryPrx, RegistryPrx> replicas;
    Ice::ObjectProxySeq proxies = _database->getObjectsByType(InternalRegistry::ice_staticId());
    for(ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        replicas.insert(pair<InternalRegistryPrx, RegistryPrx>(InternalRegistryPrx::uncheckedCast(*p), RegistryPrx()));
    }

    proxies = _database->getObjectsByType(Registry::ice_staticId());
    for(ObjectProxySeq::const_iterator p = proxies.begin(); p != proxies.end(); ++p)
    {
        Ice::Identity id = (*p)->ice_getIdentity();
        const string prefix("Registry-");
        string::size_type pos = id.name.find(prefix);
        if(pos == string::npos)
        {
            continue; // Ignore the master registry proxy.
        }
        id.name = "InternalRegistry-" + id.name.substr(prefix.size());
        
        Ice::ObjectPrx prx = (*p)->ice_identity(id)->ice_endpoints(Ice::EndpointSeq());
        id.name = "Locator";
        prx = prx->ice_locator(Ice::LocatorPrx::uncheckedCast((*p)->ice_identity(id)));

        for(map<InternalRegistryPrx, RegistryPrx>::iterator q = replicas.begin(); q != replicas.end(); ++q)
        {
            if(q->first->ice_getIdentity() == prx->ice_getIdentity()) 
            {
                replicas.erase(q);
                break;
            }
        }
        replicas.insert(pair<InternalRegistryPrx, RegistryPrx>(InternalRegistryPrx::uncheckedCast(prx), 
                                                               RegistryPrx::uncheckedCast(*p)));
    }

    set<NodePrx> nodes;
    nodes.insert(dbNodes.begin(), dbNodes.end());
    vector<Ice::AsyncResultPtr> results;
    for(map<InternalRegistryPrx, RegistryPrx>::const_iterator r = replicas.begin(); r != replicas.end(); ++r)
    {
        if(r->first->ice_getIdentity() != internalRegistry->ice_getIdentity())
        {
            results.push_back(r->first->begin_registerWithReplica(internalRegistry));
        }
    }

    for(vector<Ice::AsyncResultPtr>::const_iterator p = results.begin(); p != results.end(); ++p)
    {
        InternalRegistryPrx replica = InternalRegistryPrx::uncheckedCast((*p)->getProxy());

        string replicaName;
        if(_traceLevels && _traceLevels->replica > 1)
        {
            replicaName = replica->ice_getIdentity().name;
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
            replica->end_registerWithReplica(*p);
            
            NodePrxSeq nds = replica->getNodes();
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
            RegistryPrx registry;
            for(map<InternalRegistryPrx, RegistryPrx>::const_iterator q = replicas.begin(); q != replicas.end(); ++q)
            {
                if(q->first->ice_getIdentity() == replica->ice_getIdentity()) 
                {
                    registry = q->second;
                    break;
                }
            }
	    ObjectInfoSeq infos;
            if(registry)
            {
                try
                {
		    infos.push_back(_database->getObjectInfo(registry->ice_getIdentity()));
                }
                catch(const ObjectNotRegisteredException&)
                {
                }
            }
            try
            {
	        infos.push_back(_database->getObjectInfo(replica->ice_getIdentity()));
            }
            catch(const ObjectNotRegisteredException&)
            {
            }
	    _database->removeRegistryWellKnownObjects(infos);

            if(_traceLevels && _traceLevels->replica > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "replica `" << replicaName << "' session creation failed:\n" << ex;
            }
        }
    }
    
    NodePrxSeq result;
    for(set<NodePrx>::iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
        result.push_back(*p);
    }
    return result;
}

void
RegistryI::registerNodes(const InternalRegistryPrx& /*internalRegistry*/, const NodePrxSeq& nodes)
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
