// Copyright (c) ZeroC, Inc.

#include "RegistryI.h"
#include "../Glacier2Lib/NullPermissionsVerifier.h"
#include "../Ice/FileUtil.h"
#include "../Ice/Network.h"
#include "../Ice/ProtocolPluginFacade.h" // Just to get the hostname
#include "../Ice/SSL/SSLUtil.h"
#include "../IceStorm/Service.h"
#include "AdminCallbackRouter.h"
#include "AdminI.h"
#include "AdminSessionI.h"
#include "Database.h"
#include "FileCache.h"
#include "FileUserAccountMapperI.h"
#include "Glacier2/PermissionsVerifier.h"
#include "Ice/Ice.h"
#include "Ice/UUID.h"
#include "InternalRegistryI.h"
#include "LocatorI.h"
#include "LocatorRegistryI.h"
#include "Lookup.h"
#include "QueryI.h"
#include "ReapThread.h"
#include "RegistryAdminRouter.h"
#include "SessionI.h"
#include "SessionServantManager.h"
#include "TraceLevels.h"
#include "WellKnownObjectsManager.h"

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
        LookupI(
            string instanceName,
            const shared_ptr<WellKnownObjectsManager>& wellKnownObjects,
            const shared_ptr<TraceLevels>& traceLevels)
            : _instanceName(std::move(instanceName)),
              _wellKnownObjects(wellKnownObjects),
              _traceLevels(traceLevels)
        {
        }

        void
        findLocator(string instanceName, optional<IceLocatorDiscovery::LookupReplyPrx> reply, const Ice::Current&) final
        {
            if (!instanceName.empty() && instanceName != _instanceName)
            {
                if (_traceLevels->discovery > 1)
                {
                    Trace out(_traceLevels->logger, _traceLevels->discoveryCat);
                    out << "ignored discovery lookup for instance name '" << instanceName << "':\nreply = " << reply;
                }
                return; // Ignore.
            }

            if (reply)
            {
                if (_traceLevels->discovery > 0)
                {
                    Trace out(_traceLevels->logger, _traceLevels->discoveryCat);
                    out << "replying to discovery lookup:\nreply = " << reply;
                }
                reply->foundLocatorAsync(_wellKnownObjects->getLocator(), nullptr); // don't wait for response
            }
        }

    private:
        const string _instanceName;
        const shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
        const shared_ptr<TraceLevels> _traceLevels;
    };

    class FinderI final : public Ice::LocatorFinder
    {
    public:
        FinderI(shared_ptr<WellKnownObjectsManager> wellKnownObjects) : _wellKnownObjects(std::move(wellKnownObjects))
        {
        }

        optional<Ice::LocatorPrx> getLocator(const Ice::Current&) final { return _wellKnownObjects->getLocator(); }

    private:
        const shared_ptr<WellKnownObjectsManager> _wellKnownObjects;
    };

    class ProcessI final : public Process
    {
    public:
        ProcessI(shared_ptr<RegistryI>, shared_ptr<Process>);

        void shutdown(const Current&) final;
        void writeMessage(string, int, const Current&) final;

    private:
        shared_ptr<RegistryI> _registry;
        shared_ptr<Process> _origProcess;
    };

    shared_ptr<IPConnectionInfo> getIPConnectionInfo(const ConnectionInfoPtr& info)
    {
        for (auto p = info; p; p = p->underlying)
        {
            auto ipInfo = dynamic_pointer_cast<IPConnectionInfo>(p);
            if (ipInfo)
            {
                return ipInfo;
            }
        }
        return nullptr;
    }

    ProcessI::ProcessI(shared_ptr<RegistryI> registry, shared_ptr<Process> origProcess)
        : _registry(std::move(registry)),
          _origProcess(std::move(origProcess))
    {
    }

    void ProcessI::shutdown(const Current&) { _registry->shutdown(); }

    void ProcessI::writeMessage(string message, int fd, const Current& current)
    {
        _origProcess->writeMessage(message, fd, current);
    }
}

RegistryI::RegistryI(
    const CommunicatorPtr& communicator,
    const shared_ptr<TraceLevels>& traceLevels,
    bool readonly,
    string initFromReplica,
    string collocatedNodeName)
    : _communicator(communicator),
      _traceLevels(traceLevels),
      _readonly(readonly),
      _initFromReplica(std::move(initFromReplica)),
      _collocatedNodeName(std::move(collocatedNodeName)),
      _platform("IceGrid.Registry", communicator, traceLevels)
{
}

bool
RegistryI::start()
{
    try
    {
        if (!startImpl())
        {
            stop();
            return false;
        }
    }
    catch (...)
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
    if (properties->getIceProperty("IceGrid.Registry.Client.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Client.Endpoints' is not set";
        return false;
    }

    if (properties->getIceProperty("IceGrid.Registry.Server.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Server.Endpoints' is not set";
        return false;
    }

    if (properties->getIceProperty("IceGrid.Registry.Internal.Endpoints").empty())
    {
        Error out(_communicator->getLogger());
        out << "property `IceGrid.Registry.Internal.Endpoints' is not set";
        return false;
    }

    if (!properties->getIceProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        Warning out(_communicator->getLogger());
        out << "session manager endpoints `IceGrid.Registry.SessionManager.Endpoints' enabled";
        if (properties->getIcePropertyAsInt("IceGrid.Registry.SessionFilters") <= 0)
        {
            out << " (with Glacier2 filters disabled)";
        }
    }

    if (!properties->getIceProperty("IceGrid.Registry.AdminSessionManager.Endpoints").empty())
    {
        Warning out(_communicator->getLogger());
        out << "administrative session manager endpoints `IceGrid.Registry.AdminSessionManager.Endpoints' enabled";
        if (properties->getIcePropertyAsInt("IceGrid.Registry.AdminSessionFilters") <= 0)
        {
            out << " (with Glacier2 filters disabled)";
        }
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IceGrid.Registry.Client.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Server.AdapterId", "");
    properties->setProperty("IceGrid.Registry.SessionManager.AdapterId", "");

    // Turn-off the inactivity timeout for incoming connections from Glacier2 router.
    properties->setProperty("IceGrid.Registry.SessionManager.Connection.InactivityTimeout", "0");
    properties->setProperty("IceGrid.Registry.AdminSessionManager.Connection.InactivityTimeout", "0");

    properties->setProperty("IceGrid.Registry.AdminSessionManager.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Internal.AdapterId", "");
    if (properties->getIceProperty("IceGrid.Registry.Internal.MessageSizeMax").empty())
    {
        properties->setProperty("IceGrid.Registry.Internal.MessageSizeMax", "0"); // No limit on internal data exchanged
    }

    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10, true); // Serialize for admin callbacks
    setupThreadPool(properties, "IceGrid.Registry.SessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.AdminSessionManager.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _replicaName = properties->getIceProperty("IceGrid.Registry.ReplicaName");
    _master = _replicaName == "Master";

    if (!_initFromReplica.empty() && (_initFromReplica == _replicaName || (_master && _initFromReplica == "Master")))
    {
        Error out(_communicator->getLogger());
        out << "invalid --initdb-from-replica option: identical replica";
        return false;
    }

    if (!_master && !_communicator->getDefaultLocator())
    {
        Error out(_communicator->getLogger());
        out << "property `Ice.Default.Locator' is not set";
        return false;
    }
    else if (_master)
    {
        _communicator->setDefaultLocator(nullopt); // Clear the default locator in case it's set.
    }

    //
    // Get the instance name
    //
    _instanceName = properties->getIceProperty("IceGrid.InstanceName");
    if (_instanceName.empty()) // not set explicitly
    {
        _instanceName = properties->getProperty("IceLocatorDiscovery.InstanceName");
    }
    if (_instanceName.empty() && _communicator->getDefaultLocator())
    {
        _instanceName = _communicator->getDefaultLocator()->ice_getIdentity().category;
    }
    if (_instanceName.empty())
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
    string dbPath = _communicator->getProperties()->getIceProperty("IceGrid.Registry.LMDB.Path");
    if (dbPath.empty())
    {
        Ice::Error out(_communicator->getLogger());
        out << "property 'IceGrid.Registry.LMDB.Path' is not set";
        return false;
    }
    else
    {
        if (!IceInternal::directoryExists(dbPath))
        {
            Ice::Error out(_communicator->getLogger());
            out << "property 'IceGrid.Registry.LMDB.Path' is set to an invalid path: " << dbPath;
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
        string endpoints = properties->getIceProperty("IceGrid.Registry.Client.Endpoints");
        string strPrx = _instanceName + "/Locator:" + endpoints;
        _communicator->stringToProxy(strPrx)->ice_invocationTimeout(5s)->ice_ping();

        Error out(_communicator->getLogger());
        out << "an IceGrid registry is already running and listening on the client endpoints '" << endpoints << "'";
        return false;
    }
    catch (const Ice::LocalException&)
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
    _iceStorm = IceStormInternal::Service::create(
        _communicator,
        _registryAdapter,
        _registryAdapter,
        "IceGrid.Registry",
        registryTopicManagerId);

    try
    {
        _database = Database::create(
            _registryAdapter,
            _iceStorm->getTopicManager(),
            _instanceName,
            _traceLevels,
            getInfo(),
            _readonly);
    }
    catch (const IceDB::LMDBException& ex)
    {
        Error out(_communicator->getLogger());
        out << "couldn't open database:\n";
        out << ex;
        return false;
    }
    _wellKnownObjects = make_shared<WellKnownObjectsManager>(_database);

    if (!_initFromReplica.empty())
    {
        Ice::Identity id{(_initFromReplica == "Master") ? "Registry" : "Registry-" + _initFromReplica, _instanceName};

        optional<ObjectPrx> proxy;
        try
        {
            proxy = _database->getObjectProxy(id);
            assert(proxy);
            id.name = "Query";
            auto query = proxy->ice_identity<IceGrid::QueryPrx>(id);
            id.name = "InternalRegistry-" + _initFromReplica;
            try
            {
                proxy = query->findObjectById(id);
            }
            catch (const Ice::Exception&)
            {
            }
        }
        catch (const ObjectNotRegisteredException&)
        {
            id.name = "InternalRegistry-" + _initFromReplica;
            try
            {
                proxy = _database->getObjectProxy(id);
            }
            catch (const ObjectNotRegisteredException&)
            {
            }
        }

        // If we still didn't find the replica proxy, check with the locator or the IceGrid.Registry.ReplicaEndpoints
        // properties if we can find it.
        if (!proxy)
        {
            id.name = "InternalRegistry-" + (_initFromReplica.empty() ? "Master" : _initFromReplica);
            proxy = _session->findInternalRegistryForReplica(id);
        }

        if (!proxy)
        {
            Error out(_communicator->getLogger());
            out << "couldn't find replica '" << _initFromReplica << "' to\n";
            out << "initialize the database (specify the replica endpoints in the endpoints of\n";
            out << "the `Ice.Default.Locator' proxy property to allow finding the replica)";
            return false;
        }

        try
        {
            int64_t serial;
            auto registry = uncheckedCast<IceGrid::InternalRegistryPrx>(*proxy);
            ApplicationInfoSeq applications = registry->getApplications(serial);
            _database->syncApplications(applications, serial);
            AdapterInfoSeq adapters = registry->getAdapters(serial);
            _database->syncAdapters(adapters, serial);
            ObjectInfoSeq objects = registry->getObjects(serial);
            _database->syncObjects(objects, serial);
        }
        catch (const Ice::OperationNotExistException&)
        {
            Error out(_communicator->getLogger());
            out << "couldn't initialize database from replica '" << _initFromReplica << "':\n";
            out << "replica doesn't support this functionality (IceGrid < 3.5.1)";
            return false;
        }
        catch (const Ice::Exception& ex)
        {
            Error out(_communicator->getLogger());
            out << "couldn't initialize database from replica '" << _initFromReplica << "':\n";
            out << ex;
            return false;
        }
    }

    //
    // Get proxies for nodes that we were connected with on last
    // shutdown.
    //
    NodePrxSeq nodes;
    for (const auto& proxy : _database->getInternalObjectsByType(string{Node::ice_staticId()}))
    {
        assert(proxy);
        nodes.push_back(uncheckedCast<NodePrx>(proxy));
    }

    //
    // NOTE: The internal registry object must be added only once the
    // node/replica proxies are retrieved. Otherwise, if some
    // replica/node register as soon as the internal registry is setup
    // we might clear valid proxies.
    //
    InternalRegistryPrx internalRegistry = setupInternalRegistry();
    if (_master)
    {
        registerNodes(registerReplicas(internalRegistry, nodes));
    }
    else
    {
        _session
            ->create(_replicaName, _platform.getInternalReplicaInfo(), _database, _wellKnownObjects, internalRegistry);
        registerNodes(_session->getNodes(nodes));
    }

    _serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    _clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");

    Ice::Identity dummy = {"dummy", ""};
    _wellKnownObjects->addEndpoint("Client", _clientAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Server", _serverAdapter->createDirectProxy(dummy));
    _wellKnownObjects->addEndpoint("Internal", _registryAdapter->createDirectProxy(dummy));

    //
    // Setup file user account mapper object if the property is set.
    //
    string userAccountFileProperty = properties->getIceProperty("IceGrid.Registry.UserAccounts");
    if (!userAccountFileProperty.empty())
    {
        try
        {
            Identity mapperId{
                _master ? "RegistryUserAccountMapper" : "RegistryUserAccountMapper-" + _replicaName,
                _instanceName};
            _registryAdapter->add(make_shared<FileUserAccountMapperI>(userAccountFileProperty), mapperId);
            _wellKnownObjects->add(_registryAdapter->createProxy(mapperId), string{UserAccountMapper::ice_staticId()});
        }
        catch (const Ice::Exception& ex)
        {
            Error out(_communicator->getLogger());
            out << "unable to setup file user account mapper:\n" << ex;
            return false;
        }
    }

    setupLocatorRegistry();
    LocatorPrx internalLocator = setupLocator(setupRegistry(), setupQuery());

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

    _servantManager = make_shared<SessionServantManager>(
        _clientAdapter,
        _instanceName,
        true,
        getServerAdminCategory(),
        serverAdminRouter,
        getNodeAdminCategory(),
        nodeAdminRouter,
        getReplicaAdminCategory(),
        replicaAdminRouter,
        adminCallbackRouter);

    _clientAdapter->addServantLocator(_servantManager, "");
    _serverAdapter->addDefaultServant(adminCallbackRouter, "");

    vector<string> verifierProperties;
    verifierProperties.emplace_back("IceGrid.Registry.PermissionsVerifier");
    verifierProperties.emplace_back("IceGrid.Registry.SSLPermissionsVerifier");
    verifierProperties.emplace_back("IceGrid.Registry.AdminPermissionsVerifier");
    verifierProperties.emplace_back("IceGrid.Registry.AdminSSLPermissionsVerifier");

    try
    {
        Glacier2Internal::setupNullPermissionsVerifier(_communicator, _instanceName, verifierProperties);
    }
    catch (const std::exception& ex)
    {
        Error out(_communicator->getLogger());
        out << "unable to setup null permissions verifier:\n" << ex;
        return false;
    }

    auto sessionAdpt = setupClientSessionFactory(internalLocator);
    auto admSessionAdpt =
        setupAdminSessionFactory(serverAdminRouter, nodeAdminRouter, replicaAdminRouter, internalLocator);

    _wellKnownObjects->finish();
    if (_master)
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
    ObjectAdapterPtr discoveryAdapter;
    if (properties->getIcePropertyAsInt("IceGrid.Registry.Discovery.Enabled") > 0)
    {
        bool ipv4 = properties->getIcePropertyAsInt("Ice.IPv4") > 0;
        bool preferIPv6 = properties->getIcePropertyAsInt("Ice.PreferIPv6Address") > 0;
        string address = properties->getIceProperty("IceGrid.Registry.Discovery.Address");
        if (address.empty())
        {
            address = ipv4 && !preferIPv6 ? "239.255.0.1" : "ff15::1";
        }
        int port = properties->getIcePropertyAsInt("IceGrid.Registry.Discovery.Port");
        string interface = properties->getIceProperty("IceGrid.Registry.Discovery.Interface");
        if (properties->getIceProperty("IceGrid.Registry.Discovery.Endpoints").empty())
        {
            ostringstream os;
            os << "udp -h \"" << address << "\" -p " << port;
            if (!interface.empty())
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
        catch (const Ice::LocalException& ex)
        {
            Warning out(_communicator->getLogger());
            out << "failed to join the multicast group for IceGrid discovery:\n";
            out << "endpoints = " << properties->getIceProperty("IceGrid.Registry.Discovery.Endpoints") << "\n";
            out << ex;
        }
    }

    //
    // We are ready to go!
    //
    _serverAdapter->activate();
    _clientAdapter->activate();
    if (discoveryAdapter)
    {
        discoveryAdapter->activate();
    }

    if (sessionAdpt)
    {
        sessionAdpt->activate();
    }
    if (admSessionAdpt)
    {
        admSessionAdpt->activate();
    }

    return true;
}

void
RegistryI::setupLocatorRegistry()
{
    const bool dynReg = _communicator->getProperties()->getIcePropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegId = {"LocatorRegistry", _instanceName};
    _serverAdapter->add(make_shared<LocatorRegistryI>(_database, dynReg, _master, *_session), locatorRegId);
}

IceGrid::LocatorPrx
RegistryI::setupLocator(RegistryPrx registry, QueryPrx query)
{
    auto locator =
        make_shared<LocatorI>(_communicator, _database, _wellKnownObjects, std::move(registry), std::move(query));

    _clientAdapter->add(locator, Identity{"Locator", _instanceName});
    _clientAdapter->add(locator, Identity{"Locator-" + _replicaName, _instanceName});

    return _registryAdapter->addWithUUID<LocatorPrx>(locator);
}

QueryPrx
RegistryI::setupQuery()
{
    return _clientAdapter->add<QueryPrx>(
        make_shared<QueryI>(_communicator, _database),
        Identity{"Query", _instanceName});
}

RegistryPrx
RegistryI::setupRegistry()
{
    auto proxy = _clientAdapter->add<RegistryPrx>(
        shared_from_this(),
        Identity{_master ? "Registry" : "Registry-" + _replicaName, _instanceName});
    _wellKnownObjects->add(proxy, string{Registry::ice_staticId()});
    return proxy;
}

InternalRegistryPrx
RegistryI::setupInternalRegistry()
{
    assert(_reaper);

    Identity internalRegistryId = {"InternalRegistry-" + _replicaName, _instanceName};

    auto internalRegistry =
        make_shared<InternalRegistryI>(shared_from_this(), _database, _reaper, _wellKnownObjects, *_session);
    auto registry = _registryAdapter->add<InternalRegistryPrx>(internalRegistry, internalRegistryId);

    _wellKnownObjects->add(registry, string{InternalRegistry::ice_staticId()});

    //
    // Create Admin
    //
    if (_communicator->getProperties()->getIcePropertyAsInt("Ice.Admin.Enabled") > 0)
    {
        // Replace Admin facet
        auto origProcess = dynamic_pointer_cast<Process>(_communicator->removeAdminFacet("Process"));
        _communicator->addAdminFacet(make_shared<ProcessI>(shared_from_this(), origProcess), "Process");
        _communicator->createAdmin(_registryAdapter, Identity{"RegistryAdmin-" + _replicaName, _instanceName});
    }

    _database->getReplicaCache().setInternalRegistry(registry);
    return registry;
}

ObjectAdapterPtr
RegistryI::setupClientSessionFactory(const IceGrid::LocatorPrx& locator)
{
    auto properties = _communicator->getProperties();

    ObjectAdapterPtr adapter;
    shared_ptr<SessionServantManager> servantManager;
    if (!properties->getIceProperty("IceGrid.Registry.SessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.SessionManager");
        servantManager = make_shared<
            SessionServantManager>(adapter, _instanceName, false, "", nullptr, "", nullptr, "", nullptr, nullptr);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _timer = make_shared<IceInternal::Timer>(); // Used for session allocation timeout.
    _clientSessionFactory = make_shared<ClientSessionFactory>(servantManager, _database, _timer, _reaper);

    if (servantManager && _master) // Slaves don't support client session manager objects.
    {
        ObjectPrx sessionManager = adapter->add(
            make_shared<ClientSessionManagerI>(_clientSessionFactory),
            Identity{"SessionManager", _instanceName});

        ObjectPrx sslSessionManager = adapter->add(
            make_shared<ClientSSLSessionManagerI>(_clientSessionFactory),
            Identity{"SSLSessionManager", _instanceName});

        _wellKnownObjects->add(sessionManager, string{Glacier2::SessionManager::ice_staticId()});
        _wellKnownObjects->add(sslSessionManager, string{Glacier2::SSLSessionManager::ice_staticId()});
    }

    if (adapter)
    {
        _wellKnownObjects->addEndpoint("SessionManager", adapter->createDirectProxy(Ice::Identity{"dummy", ""}));
    }

    _clientVerifier = getPermissionsVerifier(locator, "IceGrid.Registry.PermissionsVerifier");
    _sslClientVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.SSLPermissionsVerifier");

    return adapter;
}

ObjectAdapterPtr
RegistryI::setupAdminSessionFactory(
    const ObjectPtr& serverAdminRouter,
    const ObjectPtr& nodeAdminRouter,
    const ObjectPtr& replicaAdminRouter,
    const IceGrid::LocatorPrx& locator)
{
    auto properties = _communicator->getProperties();

    ObjectAdapterPtr adapter;
    shared_ptr<SessionServantManager> servantManager;
    if (!properties->getIceProperty("IceGrid.Registry.AdminSessionManager.Endpoints").empty())
    {
        adapter = _communicator->createObjectAdapter("IceGrid.Registry.AdminSessionManager");
        servantManager = make_shared<SessionServantManager>(
            adapter,
            _instanceName,
            false,
            getServerAdminCategory(),
            serverAdminRouter,
            getNodeAdminCategory(),
            nodeAdminRouter,
            getReplicaAdminCategory(),
            replicaAdminRouter,
            nullptr);
        adapter->addServantLocator(servantManager, "");
    }

    assert(_reaper);
    _adminSessionFactory = make_shared<AdminSessionFactory>(servantManager, _database, _reaper, shared_from_this());

    if (servantManager)
    {
        Identity sessionMgrId = {"AdminSessionManager", _instanceName};
        Identity sslSessionMgrId = {"AdminSSLSessionManager", _instanceName};
        if (!_master)
        {
            sessionMgrId.name += "-" + _replicaName;
            sslSessionMgrId.name += "-" + _replicaName;
        }

        adapter->add(make_shared<AdminSessionManagerI>(_adminSessionFactory), sessionMgrId);
        adapter->add(make_shared<AdminSSLSessionManagerI>(_adminSessionFactory), sslSessionMgrId);

        _wellKnownObjects->add(adapter->createProxy(sessionMgrId), string{Glacier2::SessionManager::ice_staticId()});
        _wellKnownObjects->add(
            adapter->createProxy(sslSessionMgrId),
            string{Glacier2::SSLSessionManager::ice_staticId()});
    }

    if (adapter)
    {
        _wellKnownObjects->addEndpoint("AdminSessionManager", adapter->createDirectProxy(Ice::Identity{"dummy", ""}));
    }

    _adminVerifier = getPermissionsVerifier(locator, "IceGrid.Registry.AdminPermissionsVerifier");
    _sslAdminVerifier = getSSLPermissionsVerifier(locator, "IceGrid.Registry.AdminSSLPermissionsVerifier");

    return adapter;
}

void
RegistryI::stop()
{
    if (_session)
    {
        _session->destroy();
    }

    //
    // We destroy the topics before to shutdown the communicator to
    // ensure that there will be no more invocations on IceStorm once
    // it's shutdown.
    //
    if (_database)
    {
        _database->destroy();
    }

    try
    {
        _communicator->shutdown();
        _communicator->waitForShutdown();
    }
    catch (const Ice::LocalException& ex)
    {
        Warning out(_communicator->getLogger());
        out << "unexpected exception while shutting down registry communicator:\n" << ex;
    }

    if (_reaper)
    {
        _reaper->terminate();
        _reaper->join();
        _reaper = nullptr;
    }

    if (_timer)
    {
        _timer->destroy();
        _timer = nullptr;
    }

    if (_iceStorm)
    {
        _iceStorm->stop();
        _iceStorm = nullptr;
    }

    _wellKnownObjects = nullptr;
    _clientSessionFactory = nullptr;
    _adminSessionFactory = nullptr;
    _database = nullptr;
}

optional<SessionPrx>
RegistryI::createSession(string user, string password, const Current& current)
{
    if (!_master)
    {
        throw PermissionDeniedException("client session creation is only allowed with the master registry.");
    }

    assert(_reaper && _clientSessionFactory);

    if (!_clientVerifier)
    {
        throw PermissionDeniedException("no permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.PermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    if (user.empty())
    {
        throw PermissionDeniedException("empty user id");
    }

    try
    {
        string reason;
        if (!_clientVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch (const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch (const LocalException& ex)
    {
        if (_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with client permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _clientSessionFactory->createSessionServant(user);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapable<SessionI>>(_traceLevels->logger, session), 0s, current.con);
    return uncheckedCast<SessionPrx>(proxy);
}

optional<AdminSessionPrx>
RegistryI::createAdminSession(string user, string password, const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if (!_adminVerifier)
    {
        throw PermissionDeniedException("no admin permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.AdminPermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    if (user.empty())
    {
        throw PermissionDeniedException("empty user id");
    }

    try
    {
        string reason;
        if (!_adminVerifier->checkPermissions(user, password, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch (const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch (const LocalException& ex)
    {
        if (_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with admin permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _adminSessionFactory->createSessionServant(user);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapable<AdminSessionI>>(_traceLevels->logger, session), 0s, current.con);
    return uncheckedCast<AdminSessionPrx>(proxy);
}

optional<SessionPrx>
RegistryI::createSessionFromSecureConnection(const Current& current)
{
    if (!_master)
    {
        throw PermissionDeniedException("client session creation is only allowed with the master registry.");
    }

    assert(_reaper && _clientSessionFactory);

    if (!_sslClientVerifier)
    {
        throw PermissionDeniedException("no ssl permissions verifier configured, use the property\n"
                                        "`IceGrid.Registry.SSLPermissionsVerifier' to configure\n"
                                        "a permissions verifier.");
    }

    string userDN;
    Glacier2::SSLInfo info = getSSLInfo(current.con, userDN);
    if (userDN.empty())
    {
        throw PermissionDeniedException("empty user DN");
    }

    try
    {
        string reason;
        if (!_sslClientVerifier->authorize(info, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch (const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch (const LocalException& ex)
    {
        if (_traceLevels && _traceLevels->session > 0)
        {
            Trace out(_traceLevels->logger, _traceLevels->sessionCat);
            out << "exception while verifying password with SSL client permission verifier:\n" << ex;
        }

        throw PermissionDeniedException("internal server error");
    }

    auto session = _clientSessionFactory->createSessionServant(userDN);
    auto proxy = session->_register(_servantManager, current.con);
    _reaper->add(make_shared<SessionReapable<SessionI>>(_traceLevels->logger, session), 0s, current.con);
    return uncheckedCast<SessionPrx>(proxy);
}

optional<AdminSessionPrx>
RegistryI::createAdminSessionFromSecureConnection(const Current& current)
{
    assert(_reaper && _adminSessionFactory);

    if (!_sslAdminVerifier)
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
        if (!_sslAdminVerifier->authorize(info, reason, current.ctx))
        {
            throw PermissionDeniedException(reason);
        }
    }
    catch (const Glacier2::PermissionDeniedException& ex)
    {
        throw PermissionDeniedException(ex.reason);
    }
    catch (const LocalException& ex)
    {
        if (_traceLevels && _traceLevels->session > 0)
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
    _reaper->add(make_shared<SessionReapable<AdminSessionI>>(_traceLevels->logger, session), 0s, current.con);
    return uncheckedCast<AdminSessionPrx>(proxy);
}

int
RegistryI::getSessionTimeout(const Ice::Current&) const
{
    // getSessionTimeout is called by clients that create sessions (resource allocation sessions aka client sessions and
    // admin sessions) directly using the IceGrid::Registry interface. These sessions are hosted by the
    // IceGrid.Registry.Client object adapter.

    // A Glacier2 client that creates a session using the Glacier2::SessionManager can't call this operation since it
    // doesn't have access to the IceGrid::Registry interface.
    PropertiesPtr properties = _communicator->getProperties();

    int serverIdleTimeout = properties->getIcePropertyAsInt("Ice.Connection.Server.IdleTimeout");
    return properties->getPropertyAsIntWithDefault("IceGrid.Registry.Client.Connection.IdleTimeout", serverIdleTimeout);
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

optional<Glacier2::PermissionsVerifierPrx>
RegistryI::getPermissionsVerifier(const IceGrid::LocatorPrx& locator, const string& verifierProperty)
{
    // Get the permissions verifier
    optional<Glacier2::PermissionsVerifierPrx> verifier;
    try
    {
        verifier = _communicator->propertyToProxy<Glacier2::PermissionsVerifierPrx>(verifierProperty);
    }
    catch (const LocalException& ex)
    {
        Error out(_communicator->getLogger());
        out << "permissions verifier '" << _communicator->getProperties()->getProperty(verifierProperty)
            << "' is invalid:\n"
            << ex;
    }

    if (verifier)
    {
        // Set the permission verifier proxy locator to the internal locator. We can't use the "public" locator, this
        // could lead to deadlocks if there's not enough threads in the client thread pool anymore.
        return verifier->ice_locator(locator);
    }
    else
    {
        return nullopt;
    }
}

optional<Glacier2::SSLPermissionsVerifierPrx>
RegistryI::getSSLPermissionsVerifier(const IceGrid::LocatorPrx& locator, const string& verifierProperty)
{
    // Get the permissions verifier.
    optional<Glacier2::SSLPermissionsVerifierPrx> verifier;
    try
    {
        verifier = _communicator->propertyToProxy<Glacier2::SSLPermissionsVerifierPrx>(verifierProperty);
    }
    catch (const LocalException& ex)
    {
        Error out(_communicator->getLogger());
        out << "ssl permissions verifier '" << _communicator->getProperties()->getProperty(verifierProperty)
            << "' is invalid:\n"
            << ex;
    }

    if (verifier)
    {
        // Set the permission verifier proxy locator to the internal locator. We can't use the "public" locator, this
        // could lead to deadlocks if there's not enough threads in the client thread pool anymore.
        return verifier->ice_locator(locator);
    }
    else
    {
        return nullopt;
    }
}

Glacier2::SSLInfo
RegistryI::getSSLInfo(const ConnectionPtr& connection, string& userDN)
{
    Glacier2::SSLInfo sslinfo;
    try
    {
        auto info = dynamic_pointer_cast<Ice::SSL::ConnectionInfo>(connection->getInfo());
        if (!info)
        {
            throw PermissionDeniedException("not ssl connection");
        }

        auto ipInfo = getIPConnectionInfo(info);
        sslinfo.remotePort = ipInfo->remotePort;
        sslinfo.remoteHost = ipInfo->remoteAddress;
        sslinfo.localPort = ipInfo->localPort;
        sslinfo.localHost = ipInfo->localAddress;
        if (info->peerCertificate)
        {
            sslinfo.certs.push_back(Ice::SSL::encodeCertificate(info->peerCertificate));
            userDN = Ice::SSL::getSubjectName(info->peerCertificate);
        }
    }
    catch (const Ice::SSL::CertificateEncodingException&)
    {
        throw PermissionDeniedException("certificate encoding exception");
    }
    catch (const Ice::LocalException&)
    {
        throw PermissionDeniedException("connection exception");
    }

    return sslinfo;
}

NodePrxSeq
RegistryI::registerReplicas(const InternalRegistryPrx& internalRegistry, const NodePrxSeq& dbNodes)
{
    // Get proxies for slaves that we we connected with on last  shutdown.
    //
    // We first get the internal registry proxies and then also check
    // the public registry proxies. If we find public registry
    // proxies, we use indirect proxies setup with a locator using the
    // public proxy in preference over the internal proxy which might
    // contain stale endpoints if the slave was restarted. IceGrid
    // version <= 3.5.0 also kept the internal proxy in the database
    // instead of the public proxy.
    //
    map<InternalRegistryPrx, optional<RegistryPrx>> replicas;

    for (const auto& p : _database->getObjectsByType(string{InternalRegistry::ice_staticId()}))
    {
        assert(p);
        replicas[uncheckedCast<InternalRegistryPrx>(*p)] = nullopt;
    }

    for (const auto& p : _database->getObjectsByType(string{Registry::ice_staticId()}))
    {
        assert(p);
        Ice::Identity id = p->ice_getIdentity();
        const string prefix("Registry-");
        string::size_type pos = id.name.find(prefix);
        if (pos == string::npos)
        {
            continue; // Ignore the master registry proxy.
        }
        id.name = "InternalRegistry-" + id.name.substr(prefix.size());

        auto prx = p->ice_identity<InternalRegistryPrx>(id)->ice_endpoints(Ice::EndpointSeq());
        id.name = "Locator";
        prx = prx->ice_locator(p->ice_identity<Ice::LocatorPrx>(id));

        for (auto q = replicas.begin(); q != replicas.end(); ++q)
        {
            if (q->first->ice_getIdentity() == prx->ice_getIdentity())
            {
                replicas.erase(q);
                break;
            }
        }
        replicas[prx] = uncheckedCast<RegistryPrx>(p);
    }

    set<optional<NodePrx>> nodes;
    nodes.insert(dbNodes.begin(), dbNodes.end());
    map<InternalRegistryPrx, future<void>> results;
    for (const auto& registryPrx : replicas)
    {
        if (registryPrx.first->ice_getIdentity() != internalRegistry->ice_getIdentity())
        {
            results.insert({registryPrx.first, registryPrx.first->registerWithReplicaAsync(internalRegistry)});
        }
    }

    for (const auto& result : results)
    {
        const auto& replica = result.first;

        string replicaName;
        if (_traceLevels && _traceLevels->replica > 1)
        {
            replicaName = replica->ice_getIdentity().name;
            const string prefix("InternalRegistry-");
            string::size_type pos = replicaName.find(prefix);
            if (pos != string::npos)
            {
                replicaName = replicaName.substr(prefix.size());
            }

            Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
            out << "creating replica '" << replicaName << "' session";
        }

        try
        {
            result.second.wait();

            NodePrxSeq nds = replica->getNodes();
            nodes.insert(nds.begin(), nds.end());

            if (_traceLevels && _traceLevels->replica > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "replica '" << replicaName << "' session created";
            }
        }
        catch (const Ice::LocalException& ex)
        {
            // Clear the proxy from the database if we can't contact the replica.
            optional<RegistryPrx> registry;
            for (const auto& r : replicas)
            {
                if (r.first->ice_getIdentity() == replica->ice_getIdentity())
                {
                    registry = r.second;
                    break;
                }
            }
            ObjectInfoSeq infos;
            if (registry)
            {
                try
                {
                    infos.push_back(_database->getObjectInfo(registry->ice_getIdentity()));
                }
                catch (const ObjectNotRegisteredException&)
                {
                }
            }
            try
            {
                infos.push_back(_database->getObjectInfo(replica->ice_getIdentity()));
            }
            catch (const ObjectNotRegisteredException&)
            {
            }
            _database->removeRegistryWellKnownObjects(infos);

            if (_traceLevels && _traceLevels->replica > 1)
            {
                Ice::Trace out(_traceLevels->logger, _traceLevels->replicaCat);
                out << "replica '" << replicaName << "' session creation failed:\n" << ex;
            }
        }
    }

    NodePrxSeq result;
    for (const auto& node : nodes)
    {
        result.push_back(node);
    }
    return result;
}

void
RegistryI::registerNodes(const NodePrxSeq& nodes)
{
    const string prefix("Node-");

    for (const auto& node : nodes)
    {
        assert(node->ice_getIdentity().name.find(prefix) != string::npos);
        try
        {
            _database->getNode(node->ice_getIdentity().name.substr(prefix.size()))->setProxy(*node);
        }
        catch (const NodeNotExistException&)
        {
            // Ignore, if nothing's deployed on the node we won't need to contact it for locator requests so we don't
            // need to keep its proxy.
            try
            {
                _database->removeInternalObject(node->ice_getIdentity());
            }
            catch (const ObjectNotRegisteredException&)
            {
            }
        }
    }
}
