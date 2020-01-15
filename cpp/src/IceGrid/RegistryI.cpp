//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

#include <Ice/UUID.h>
#include <IceUtil/FileUtil.h>
#include <Ice/Ice.h>
#include <Ice/Network.h>
#include <Ice/ProtocolPluginFacade.h> // Just to get the hostname

#include <IceStorm/Service.h>
#include <IceSSL/IceSSL.h>
#include <Glacier2/PermissionsVerifier.h>
#include <Glacier2/NullPermissionsVerifier.h>

#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/ReapThread.h>
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
#include <IceGrid/IceLocatorDiscovery.h>

#include <IceGrid/RegistryAdminRouter.h>

#include <fstream>

#include <sys/types.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace
{

class LookupI final : public IceLocatorDiscovery::Lookup
{
public:

    LookupI(const string& instanceName,
            const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
            const shared_ptr<TraceLevels>& traceLevels) :
        _instanceName(instanceName), _wellKnownObjects(wellKnownObjects), _traceLevels(traceLevels)
    {
    }

    void
    findLocator(string instanceName, shared_ptr<IceLocatorDiscovery::LookupReplyPrx> reply,
                const Ice::Current&) override
    {
        if(!instanceName.empty() && instanceName != _instanceName)
        {
            if(_traceLevels->discovery > 1)
            {
                Trace out(_traceLevels->logger, _traceLevels->discoveryCat);
                out << "ignored discovery lookup for instance name `" << instanceName << "':\nreply = " << reply;
            }
            return; // Ignore.
        }

        if(reply)
        {
            if(_traceLevels->discovery > 0)
            {
                Trace out(_traceLevels->logger, _traceLevels->discoveryCat);
                out << "replying to discovery lookup:\nreply = " << reply;
            }
            reply->foundLocatorAsync(_wellKnownObjects->getLocator());
        }
    }

    shared_ptr<Ice::LocatorPrx>
    getLocator(const Ice::Current&)
    {
        return _wellKnownObjects->getLocator();
    }

private:

    const string _instanceName;
    const shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    const shared_ptr<TraceLevels> _traceLevels;
};

class FinderI final : public Ice::LocatorFinder
{
public:

    FinderI(const shared_ptr<WellKnownObjectsManager>& wellKnownObjects) : _wellKnownObjects(wellKnownObjects)
    {
    }

    shared_ptr<Ice::LocatorPrx>
    getLocator(const Ice::Current&) override
    {
        return _wellKnownObjects->getLocator();
    }

private:

    const shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
};

class ProcessI final : public Process
{
public:

    ProcessI(const shared_ptr<RegistryI>&, const shared_ptr<Process>&);

    void shutdown(const Current&) override;
    void writeMessage(string, int, const Current&) override;

private:

    shared_ptr<RegistryI> _registry;
    shared_ptr<Process> _origProcess;
};

shared_ptr<IPConnectionInfo>
getIPConnectionInfo(const shared_ptr<ConnectionInfo>& info)
{
    for(auto p = info; p; p = p->underlying)
    {
        auto ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
        if(ipInfo)
        {
            return ipInfo;
        }
    }
    return nullptr;
}

ProcessI::ProcessI(const shared_ptr<RegistryI>& registry, const shared_ptr<Process>& origProcess) :
    _registry(registry),
    _origProcess(origProcess)
{
}

void
ProcessI::shutdown(const Current&)
{
    _registry->shutdown();
}

void
ProcessI::writeMessage(string message, int fd, const Current& current)
{
    _origProcess->writeMessage(move(message), move(fd), current);
}

}

RegistryI::RegistryI(const shared_ptr<Communicator>& communicator,
                     const shared_ptr<TraceLevels>& traceLevels,
                     bool nowarn,
                     bool readonly,
                     const string& initFromReplica,
                     const string& collocatedNodeName) :
    _communicator(communicator),
    _traceLevels(traceLevels),
    _nowarn(nowarn),
    _readonly(readonly),
    _initFromReplica(initFromReplica),
    _collocatedNodeName(collocatedNodeName),
    _platform("IceGrid.Registry", communicator, traceLevels)
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
    auto properties = _communicator->getProperties();

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
            if(properties->getPropertyAsInt("IceGrid.Registry.SessionFilters") <= 0)
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
            if(properties->getPropertyAsInt("IceGrid.Registry.AdminSessionFilters") <= 0)
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
    if(properties->getProperty("IceGrid.Registry.Internal.MessageSizeMax").empty())
    {
        properties->setProperty("IceGrid.Registry.Internal.MessageSizeMax", "0"); // No limit on internal data exchanged
    }

    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10, true); // Serialize for admin callbacks
    setupThreadPool(properties, "IceGrid.Registry.SessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.AdminSessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _replicaName = properties->getPropertyWithDefault("IceGrid.Registry.ReplicaName", "Master");
    _master = _replicaName == "Master";
    _sessionTimeout = chrono::seconds(properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionTimeout", 30));

    if(!_initFromReplica.empty() && (_initFromReplica == _replicaName || (_master && _initFromReplica == "Master")))
    {
        Error out(_communicator->getLogger());
        out << "invalid --initdb-from-replica option: identical replica";
        return false;
    }

    if(_sessionTimeout > 0s && properties->getProperty("IceGrid.Registry.Client.ACM.Timeout").empty())
    {
        properties->setProperty("IceGrid.Registry.Client.ACM.Timeout", to_string(secondsToInt(_sessionTimeout)));
    }

    if(properties->getProperty("IceGrid.Registry.Server.ACM.Timeout").empty())
    {
        properties->setProperty("IceGrid.Registry.Server.ACM.Timeout", "30");
    }

    if(properties->getProperty("IceGrid.Registry.Internal.ACM.Timeout").empty())
    {
        int nt = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 30);
        int rt = properties->getPropertyAsIntWithDefault("IceGrid.Registry.ReplicaSessionTimeout", 30);
        properties->setProperty("IceGrid.Registry.Internal.ACM.Timeout", to_string(max(nt, rt)));
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
        _communicator->setDefaultLocator(nullptr); // Clear the default locator in case it's set.
    }

    //
    // Get the instance name
    //
    _instanceName = properties->getProperty("IceGrid.InstanceName");
    if(_instanceName.empty())
    {
        _instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
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
    _session = make_unique<ReplicaSessionManager>(_communicator, _instanceName);

    //
    // Create the registry database.
    //
    string dbPath = _communicator->getProperties()->getProperty("IceGrid.Registry.LMDB.Path");
    if(dbPath.empty())
    {
        Ice::Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.LMDB.Path' is not set";
        return false;
    }
    else
    {
        if(!IceUtilInternal::directoryExists(dbPath))
        {
            Ice::SyscallException ex(__FILE__, __LINE__, IceInternal::getSystemErrno());
            Ice::Error out(_communicator->getLogger());
            out << "property `IceGrid.Registry.LMDB.Path' is set to an invalid path:\n" << ex;
            return false;
        }
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

    //
    // Create the reaper thread.
    //
    _reaper = make_shared<ReapThread>();

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
                                                  "");
    const auto topicManager = _iceStorm->getTopicManager();

    try
    {
        _database = Database::create(_registryAdapter, topicManager, _instanceName, _traceLevels, getInfo(), _readonly);
    }
    catch(const IceDB::LMDBException& ex)
    {
        Error out(_communicator->getLogger());
        out << "couldn't open database:\n";
        out << ex;
        return false;
    }
    _wellKnownObjects = make_shared<WellKnownObjectsManager>(_database);

    if(!_initFromReplica.empty())
    {
        Identity id;
        id.category = _instanceName;
        id.name = (_initFromReplica == "Master") ? "Registry" : "Registry-" + _initFromReplica;

        shared_ptr<ObjectPrx> proxy;
        try
        {
            proxy = _database->getObjectProxy(id);
            id.name = "Query";
            auto query = uncheckedCast<IceGrid::QueryPrx>(proxy->ice_identity(id));
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
            long long serial;
            auto registry = checkedCast<IceGrid::InternalRegistryPrx>(proxy);
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
    for(const auto& proxy : proxies)
    {
        nodes.push_back(uncheckedCast<NodePrx>(proxy));
    }

    //
    // NOTE: The internal registry object must be added only once the
    // node/replica proxies are retrieved. Otherwise, if some
    // replica/node register as soon as the internal registry is setup
    // we might clear valid proxies.
    //
    auto internalRegistry = setupInternalRegistry();
    if(_master)
    {
        nodes = registerReplicas(internalRegistry, nodes);
        registerNodes(internalRegistry, nodes);
    }
    else
    {
        auto info = _platform.getInternalReplicaInfo();
        _session->create(_replicaName, info, _database, _wellKnownObjects, internalRegistry);
        registerNodes(internalRegistry, _session->getNodes(nodes));
    }

    _serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    _clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");

    Ice::Identity dummy = { "dummy", "" };
    _wellKnownObjects->addEndpoint("Client", _clientAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Server", _serverAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Internal", _registryAdapter->createDirectProxy(dummy));

    if(!setupUserAccountMapper())
    {
        return false;
    }

    auto query = setupQuery();
    auto registry = setupRegistry();

    setupLocatorRegistry();
    auto internalLocator = setupLocator(registry, query);

    //
    // Create the session servant manager. The session servant manager is responsible
    // for managing sessions servants and to ensure that session servants are only
    // accessed by the connection that created the session. The session servant manager
    // also takes care of providing router servants for admin objects.
    //
    auto serverAdminRouter = make_shared<RegistryServerAdminRouter>(_database);
    auto nodeAdminRouter = make_shared<RegistryNodeAdminRouter>(_collocatedNodeName, _database);
    auto replicaAdminRouter = make_shared<RegistryReplicaAdminRouter>(_replicaName, _database);
    auto adminCallbackRouter = make_shared<AdminCallbackRouter>();

    _servantManager = make_shared<SessionServantManager>(_clientAdapter, _instanceName, true,
                                                         getServerAdminCategory(), serverAdminRouter,
                                                         getNodeAdminCategory(), nodeAdminRouter,
                                                         getReplicaAdminCategory(), replicaAdminRouter,
                                                         adminCallbackRouter);

    _clientAdapter->addServantLocator(_servantManager, "");
    _serverAdapter->addDefaultServant(adminCallbackRouter, "");

    vector<string> verifierProperties;
    verifierProperties.push_back("IceGrid.Registry.PermissionsVerifier");
    verifierProperties.push_back("IceGrid.Registry.SSLPermissionsVerifier");
    verifierProperties.push_back("IceGrid.Registry.AdminPermissionsVerifier");
    verifierProperties.push_back("IceGrid.Registry.AdminSSLPermissionsVerifier");

    Glacier2Internal::setupNullPermissionsVerifier(_communicator, _instanceName, verifierProperties);

    auto sessionAdpt = setupClientSessionFactory(internalLocator);
    auto admSessionAdpt = setupAdminSessionFactory(serverAdminRouter, nodeAdminRouter, replicaAdminRouter,
                                                   internalLocator);

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
    _clientAdapter->add(make_shared<FinderI>(_wellKnownObjects), stringToIdentity("Ice/LocatorFinder"));

    //
    // Setup the discovery object adapter and also add it the lookup
    // servant to receive multicast lookup queries.
    //
    shared_ptr<ObjectAdapter> discoveryAdapter;
    if(properties->getPropertyAsIntWithDefault("IceGrid.Registry.Discovery.Enabled", 1) > 0)
    {
        bool ipv4 = properties->getPropertyAsIntWithDefault("Ice.IPv4", 1) > 0;
        bool preferIPv6 = properties->getPropertyAsInt("Ice.PreferIPv6Address") > 0;
        string address;
        if(ipv4 && !preferIPv6)
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
            auto lookupId = stringToIdentity("IceLocatorDiscovery/Lookup");
            discoveryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Discovery");
            discoveryAdapter->add(make_shared<LookupI>(_instanceName, _wellKnownObjects, _traceLevels), lookupId);
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

void
RegistryI::setupLocatorRegistry()
{
    const bool dynReg = _communicator->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegId = { "LocatorRegistry", _instanceName };
    _serverAdapter->add(make_shared<LocatorRegistryI>(_database, dynReg, _master, *_session), locatorRegId);
}

shared_ptr<IceGrid::LocatorPrx>
RegistryI::setupLocator(const shared_ptr<RegistryPrx>& registry, const shared_ptr<QueryPrx>& query)
{
    auto locator = make_shared<LocatorI>(_communicator, _database, _wellKnownObjects, registry, query);
    Identity locatorId;
    locatorId.category = _instanceName;

    locatorId.name = "Locator";
    _clientAdapter->add(locator, locatorId);

    locatorId.name = "Locator-" + _replicaName;
    _clientAdapter->add(locator, locatorId);

    return uncheckedCast<LocatorPrx>(_registryAdapter->addWithUUID(locator));
}

shared_ptr<QueryPrx>
RegistryI::setupQuery()
{
    Identity queryId = { "Query", _instanceName };
    return uncheckedCast<QueryPrx>(_clientAdapter->add(make_shared<QueryI>(_communicator, _database), move(queryId)));
}

shared_ptr<RegistryPrx>
RegistryI::setupRegistry()
{
    Identity registryId = { "Registry", _instanceName };
    if(!_master)
    {
        registryId.name += "-" + _replicaName;
    }

    auto proxy = uncheckedCast<RegistryPrx>(_clientAdapter->add(shared_from_this(), move(registryId)));
    _wellKnownObjects->add(proxy, Registry::ice_staticId());
    return proxy;
}

shared_ptr<InternalRegistryPrx>
RegistryI::setupInternalRegistry()
{
    assert(_reaper);

    Identity internalRegistryId = { "InternalRegistry-" + _replicaName, _instanceName };

    auto internalRegistry = make_shared<InternalRegistryI>(shared_from_this(), _database, _reaper, _wellKnownObjects,
                                                           *_session);
    auto proxy = _registryAdapter->add(internalRegistry, internalRegistryId);

    _wellKnownObjects->add(proxy, InternalRegistry::ice_staticId());

    //
    // Create Admin
    //
    if(_communicator->getProperties()->getPropertyAsInt("Ice.Admin.Enabled") > 0)
    {
        // Replace Admin facet
        auto origProcess = dynamic_pointer_cast<Process>(_communicator->removeAdminFacet("Process"));
        _communicator->addAdminFacet(make_shared<ProcessI>(shared_from_this(), origProcess), "Process");

        Identity adminId = { "RegistryAdmin-" + _replicaName, _instanceName };
        _communicator->createAdmin(_registryAdapter, move(adminId));
    }

    auto registry = uncheckedCast<InternalRegistryPrx>(proxy);
    _database->getReplicaCache().setInternalRegistry(registry);
    return registry;
}

bool
RegistryI::setupUserAccountMapper()
{
    auto properties = _communicator->getProperties();

    //
    // Setup file user account mapper object if the property is set.
    //
    string userAccountFileProperty = properties->getProperty("IceGrid.Registry.UserAccounts");
    if(!userAccountFileProperty.empty())
    {
        try
        {
            Identity mapperId = { "RegistryUserAccountMapper", _instanceName };
            if(!_master)
            {
                mapperId.name += "-" + _replicaName;
            }

            _registryAdapter->add(make_shared<FileUserAccountMapperI>(userAccountFileProperty), mapperId);
            _wellKnownObjects->add(_registryAdapter->createProxy(mapperId), UserAccountMapper::ice_staticId());
        }
        catch(const string& msg)
        {
            Error out(_communicator->getLogger());
            out << msg;
            return false;
        }
    }
    return true;
}

shared_ptr<ObjectAdapter>
RegistryI::setupClientSessionFactory(const shared_ptr<IceGrid::LocatorPrx>& locator)
{
    auto properties = _communicator->getProperties();

    shared_ptr<ObjectAdapter> adapter;
    shared_ptr<SessionServantManager> servantManager;
    if(!properties->getProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.SessionManager");
        servantManager = make_shared<SessionServantManager>(adapter, _instanceName, false,
                                                            "", nullptr, "", nullptr, "", nullptr, nullptr);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _timer = new IceUtil::Timer;  // Used for session allocation timeout.
    _clientSessionFactory = make_shared<ClientSessionFactory>(servantManager, _database, _timer, _reaper);

    if(servantManager && _master) // Slaves don't support client session manager objects.
    {
        Identity sessionMgrId = { "SessionManager", _instanceName };
        Identity sslSessionMgrId = { "SSLSessionManager", _instanceName };

        adapter->add(make_shared<ClientSessionManagerI>(_clientSessionFactory), sessionMgrId);
        adapter->add(make_shared<ClientSSLSessionManagerI>(_clientSessionFactory), sslSessionMgrId);

        _wellKnownObjects->add(adapter->createProxy(sessionMgrId), Glacier2::SessionManager::ice_staticId());
        _wellKnownObjects->add(adapter->createProxy(sslSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
    }

    if(adapter)
    {
        Ice::Identity dummy = { "dummy", "" };
        _wellKnownObjects->addEndpoint("SessionManager", adapter->createDirectProxy(move(dummy)));
    }

    _clientVerifier = getPermissionsVerifier(locator, "IceGrid.Registry.PermissionsVerifier");
    _sslClientVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.SSLPermissionsVerifier");

    return adapter;
}

shared_ptr<ObjectAdapter>
RegistryI::setupAdminSessionFactory(const shared_ptr<Object>& serverAdminRouter,
                                    const shared_ptr<Object>& nodeAdminRouter,
                                    const shared_ptr<Object>& replicaAdminRouter,
                                    const shared_ptr<IceGrid::LocatorPrx>& locator)
{
    auto properties = _communicator->getProperties();

    shared_ptr<ObjectAdapter> adapter;
    shared_ptr<SessionServantManager> servantManager;
    if(!properties->getProperty("IceGrid.Registry.AdminSessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.AdminSessionManager");
        servantManager = make_shared<SessionServantManager>(adapter, _instanceName, false,
                                                   getServerAdminCategory(), serverAdminRouter,
                                                   getNodeAdminCategory(), nodeAdminRouter,
                                                   getReplicaAdminCategory(), replicaAdminRouter, nullptr);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _adminSessionFactory = make_shared<AdminSessionFactory>(servantManager, _database, _reaper, shared_from_this());

    if(servantManager)
    {
        Identity sessionMgrId = { "AdminSessionManager", _instanceName };
        Identity sslSessionMgrId = { "AdminSSLSessionManager", _instanceName };
        if(!_master)
        {
            sessionMgrId.name += "-" + _replicaName;
            sslSessionMgrId.name += "-" + _replicaName;
        }

        adapter->add(make_shared<AdminSessionManagerI>(_adminSessionFactory), sessionMgrId);
        adapter->add(make_shared<AdminSSLSessionManagerI>(_adminSessionFactory), sslSessionMgrId);

        _wellKnownObjects->add(adapter->createProxy(sessionMgrId), Glacier2::SessionManager::ice_staticId());
        _wellKnownObjects->add(adapter->createProxy(sslSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
    }

    if(adapter)
    {
        Ice::Identity dummy = { "dummy", "" };
        _wellKnownObjects->addEndpoint("AdminSessionManager", adapter->createDirectProxy(move(dummy)));
    }

    _adminVerifier = getPermissionsVerifier(locator, "IceGrid.Registry.AdminPermissionsVerifier");
    _sslAdminVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.AdminSSLPermissionsVerifier");

    return adapter;
}

void
RegistryI::stop()
{
    if(_session)
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
        _reaper->join();
        _reaper = nullptr;
    }

    if(_timer)
    {
        _timer->destroy();
        _timer = nullptr;
    }

    if(_iceStorm)
    {
        _iceStorm->stop();
        _iceStorm = nullptr;
    }

    _wellKnownObjects = nullptr;
    _clientSessionFactory = nullptr;
    _adminSessionFactory = nullptr;
    _database = nullptr;
}

shared_ptr<SessionPrx>
RegistryI::createSession(string user, string password, const Current& current)
{
    if(!_master)
    {
        throw PermissionDeniedException("client session creation is only allowed with the master registry.");
    }

    assert(_reaper && _clientSessionFactory);

    if(!_clientVerifier)
    {
        throw PermissionDeniedException("no permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.PermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    if(user.empty())
    {
        throw PermissionDeniedException("empty user id");
    }

    try
    {
        string reason;
        if(!_clientVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with client permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _clientSessionFactory->createSessionServant(user, nullptr);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapableWithHeartbeat<SessionI>>(_traceLevels->logger, session), _sessionTimeout,
                 current.con);
    return uncheckedCast<SessionPrx>(proxy);
}

shared_ptr<AdminSessionPrx>
RegistryI::createAdminSession(string user, string password, const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if(!_adminVerifier)
    {
        throw PermissionDeniedException("no admin permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.AdminPermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    if(user.empty())
    {
        throw PermissionDeniedException("empty user id");
    }

    try
    {
        string reason;
        if(!_adminVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with admin permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _adminSessionFactory->createSessionServant(user);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapableWithHeartbeat<AdminSessionI>>(_traceLevels->logger, session), _sessionTimeout,
                 current.con);
    return uncheckedCast<AdminSessionPrx>(proxy);
}

shared_ptr<SessionPrx>
RegistryI::createSessionFromSecureConnection(const Current& current)
{
    if(!_master)
    {
        throw PermissionDeniedException("client session creation is only allowed with the master registry.");
    }

    assert(_reaper && _clientSessionFactory);

    if(!_sslClientVerifier)
    {
        throw PermissionDeniedException("no ssl permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.SSLPermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    string userDN;
    Glacier2::SSLInfo info = getSSLInfo(current.con, userDN);
    if(userDN.empty())
    {
        throw PermissionDeniedException("empty user DN");
    }

    try
    {
        string reason;
        if(!_sslClientVerifier->authorize(info, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with SSL client permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _clientSessionFactory->createSessionServant(userDN, 0);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapableWithHeartbeat<SessionI>>(_traceLevels->logger, session), _sessionTimeout,
                 current.con);
    return uncheckedCast<SessionPrx>(proxy);
}

shared_ptr<AdminSessionPrx>
RegistryI::createAdminSessionFromSecureConnection(const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if(!_sslAdminVerifier)
    {
        throw PermissionDeniedException("no ssl admin permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.AdminSSLPermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    string userDN;
    Glacier2::SSLInfo info = getSSLInfo(current.con, userDN);
    try
    {
        string reason;
        if(!_sslAdminVerifier->authorize(info, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch(const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch(const LocalException& ex)
    {
        if(_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with SSL admin permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    //
    // We let the connection access the administrative interface.
    //
    auto session = _adminSessionFactory->createSessionServant(userDN);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapableWithHeartbeat<AdminSessionI>>(_traceLevels->logger, session), _sessionTimeout,
                 current.con);
    return uncheckedCast<AdminSessionPrx>(proxy);
}

int
RegistryI::getSessionTimeout(const Ice::Current&) const
{
    return secondsToInt(_sessionTimeout);
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

shared_ptr<Ice::ObjectPrx>
RegistryI::createAdminCallbackProxy(const Identity& id) const
{
    return _serverAdapter->createProxy(id);
}

shared_ptr<Ice::LocatorPrx>
RegistryI::getLocator()
{
    return _wellKnownObjects->getLocator();
}

shared_ptr<Glacier2::PermissionsVerifierPrx>
RegistryI::getPermissionsVerifier(const shared_ptr<IceGrid::LocatorPrx>& locator,
                                  const string& verifierProperty)
{
    //
    // Get the permissions verifier
    //
    shared_ptr<ObjectPrx> verifier;

    try
    {
        verifier = _communicator->propertyToProxy(verifierProperty);
    }
    catch(const LocalException& ex)
    {
        Error out(_communicator->getLogger());
        out << "permissions verifier `" << _communicator->getProperties()->getProperty(verifierProperty)
            << "' is invalid:\n" << ex;
        return nullptr;
    }

    if(!verifier)
    {
        return nullptr;
    }

    shared_ptr<Glacier2::PermissionsVerifierPrx> verifierPrx;
    try
    {
        //
        // Set the permission verifier proxy locator to the internal
        // locator. We can't use the "public" locator, this could lead
        // to deadlocks if there's not enough threads in the client
        // thread pool anymore.
        //
        verifierPrx = checkedCast<Glacier2::PermissionsVerifierPrx>(verifier->ice_locator(locator));
        if(!verifierPrx)
        {
            Error out(_communicator->getLogger());
            out << "permissions verifier `" << _communicator->getProperties()->getProperty(verifierProperty)
                << "' is invalid";
            return nullptr;
        }
    }
    catch(const LocalException& ex)
    {
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact permissions verifier `"
                << _communicator->getProperties()->getProperty(verifierProperty) << "':\n" << ex;
        }
        verifierPrx = uncheckedCast<Glacier2::PermissionsVerifierPrx>(verifier->ice_locator(locator));
    }
    return verifierPrx;
}

shared_ptr<Glacier2::SSLPermissionsVerifierPrx>
RegistryI::getSSLPermissionsVerifier(const shared_ptr<IceGrid::LocatorPrx>& locator, const string& verifierProperty)
{
    //
    // Get the permissions verifier, or create a default one if no
    // verifier is specified.
    //
    shared_ptr<ObjectPrx> verifier;
    try
    {
        verifier = _communicator->propertyToProxy(verifierProperty);
    }
    catch(const LocalException& ex)
    {
        Error out(_communicator->getLogger());
        out << "ssl permissions verifier `" << _communicator->getProperties()->getProperty(verifierProperty)
            << "' is invalid:\n" << ex;
        return nullptr;
    }

    if(!verifier)
    {
        return nullptr;
    }

    shared_ptr<Glacier2::SSLPermissionsVerifierPrx> verifierPrx;
    try
    {
        //
        // Set the permission verifier proxy locator to the internal
        // locator. We can't use the "public" locator, this could lead
        // to deadlocks if there's not enough threads in the client
        // thread pool anymore.
        //
        verifierPrx = checkedCast<Glacier2::SSLPermissionsVerifierPrx>(verifier->ice_locator(locator));
        if(!verifierPrx)
        {
            Error out(_communicator->getLogger());
            out << "ssl permissions verifier `" << _communicator->getProperties()->getProperty(verifierProperty)
                << "' is invalid";
            return nullptr;
        }
    }
    catch(const LocalException& ex)
    {
        if(!_nowarn)
        {
            Warning out(_communicator->getLogger());
            out << "couldn't contact ssl permissions verifier `"
                << _communicator->getProperties()->getProperty(verifierProperty) << "':\n" << ex;
        }
        verifierPrx = uncheckedCast<Glacier2::SSLPermissionsVerifierPrx>(verifier->ice_locator(locator));
    }
    return verifierPrx;
}

Glacier2::SSLInfo
RegistryI::getSSLInfo(const shared_ptr<Connection>& connection, string& userDN)
{
    Glacier2::SSLInfo sslinfo;
    try
    {
        auto info = dynamic_pointer_cast<IceSSL::ConnectionInfo>(connection->getInfo());
        if(!info)
        {
            throw PermissionDeniedException("not ssl connection");
        }

        auto ipInfo = getIPConnectionInfo(info);
        sslinfo.remotePort = ipInfo->remotePort;
        sslinfo.remoteHost = ipInfo->remoteAddress;
        sslinfo.localPort = ipInfo->localPort;
        sslinfo.localHost = ipInfo->localAddress;
        sslinfo.cipher = info->cipher;
        for(const auto& cert : info->certs)
        {
            sslinfo.certs.push_back(cert->encode());
        }
        if(info->certs.size() > 0)
        {
            userDN = info->certs[0]->getSubjectDN();
        }
    }
    catch(const IceSSL::CertificateEncodingException&)
    {
        throw PermissionDeniedException("certificate encoding exception");
    }
    catch(const Ice::LocalException&)
    {
        throw PermissionDeniedException("connection exception");
    }

    return sslinfo;
}

NodePrxSeq
RegistryI::registerReplicas(const shared_ptr<InternalRegistryPrx>& internalRegistry, const NodePrxSeq& dbNodes)
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
    map<shared_ptr<InternalRegistryPrx>, shared_ptr<RegistryPrx>> replicas;

    for(const auto& p : _database->getObjectsByType(InternalRegistry::ice_staticId()))
    {
        replicas.insert({ uncheckedCast<InternalRegistryPrx>(p), nullptr });
    }

    for(const auto& p : _database->getObjectsByType(Registry::ice_staticId()))
    {
        Ice::Identity id = p->ice_getIdentity();
        const string prefix("Registry-");
        string::size_type pos = id.name.find(prefix);
        if(pos == string::npos)
        {
            continue; // Ignore the master registry proxy.
        }
        id.name = "InternalRegistry-" + id.name.substr(prefix.size());

        auto prx = p->ice_identity(id)->ice_endpoints(Ice::EndpointSeq());
        id.name = "Locator";
        prx = prx->ice_locator(uncheckedCast<Ice::LocatorPrx>(p->ice_identity(id)));

        for(auto q = replicas.begin(); q != replicas.end(); ++q)
        {
            if(q->first->ice_getIdentity() == prx->ice_getIdentity())
            {
                replicas.erase(q);
                break;
            }
        }
        replicas.insert({uncheckedCast<InternalRegistryPrx>(prx), uncheckedCast<RegistryPrx>(p)});
    }

    set<shared_ptr<NodePrx>> nodes;
    nodes.insert(dbNodes.begin(), dbNodes.end());
    map<shared_ptr<InternalRegistryPrx>, future<void>> results;
    for(const auto& registryPrx : replicas)
    {
        if(registryPrx.first->ice_getIdentity() != internalRegistry->ice_getIdentity())
        {
            auto fut = registryPrx.first->registerWithReplicaAsync(internalRegistry);
            results.insert({registryPrx.first, move(fut)});
        }
    }

    for(const auto& result : results)
    {
        const auto& replica = result.first;

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
            result.second.wait();

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
            shared_ptr<RegistryPrx> registry;
            for(const auto& r : replicas)
            {
                if(r.first->ice_getIdentity() == replica->ice_getIdentity())
                {
                    registry = r.second;
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
    for(const auto& node : nodes)
    {
        result.push_back(node);
    }
    return result;
}

void
RegistryI::registerNodes(const shared_ptr<InternalRegistryPrx>&, const NodePrxSeq& nodes)
{
    const string prefix("Node-");

    for(const auto& node : nodes)
    {
        assert(node->ice_getIdentity().name.find(prefix) != string::npos);
        try
        {
            _database->getNode(node->ice_getIdentity().name.substr(prefix.size()))->setProxy(node);
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
                _database->removeInternalObject(node->ice_getIdentity());
            }
            catch(const ObjectNotRegisteredException&)
            {
            }
        }
    }
}
