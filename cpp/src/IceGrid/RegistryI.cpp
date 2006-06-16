// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/DisableWarnings.h>
#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <Ice/Network.h>

#include <IceStorm/Service.h>
#include <IceSSL/Plugin.h>
#include <Glacier2/PermissionsVerifier.h>

#include <IceGrid/TraceLevels.h>
#include <IceGrid/Database.h>
#include <IceGrid/ReapThread.h>
#include <IceGrid/Topics.h>

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

#include <fstream>

#include <openssl/des.h> // For crypt() passwords

#include <sys/types.h>
#include <sys/stat.h>

#ifdef _WIN32
#   include <direct.h>
#   define S_ISDIR(mode) ((mode) & _S_IFDIR)
#   define S_ISREG(mode) ((mode) & _S_IFREG)
#else
#   include <unistd.h>
#endif

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class SessionReapable : public Reapable
{
public:

    SessionReapable(const ObjectAdapterPtr& adapter, 
		    const ObjectPtr& session, 
		    const Identity& id) : 
	_adapter(adapter),
	_servant(session),
	_session(dynamic_cast<BaseSessionI*>(_servant.get())),
	_id(id)
    {
    }

    virtual ~SessionReapable()
    {
    }
	
    virtual IceUtil::Time
    timestamp() const
    {
	return _session->timestamp();
    }

    virtual void
    destroy(bool destroy)
    {
	try
	{
	    //
	    // Invoke on the servant directly instead of the
	    // proxy. Invoking on the proxy might not always work if the
	    // communicator is being shutdown/destroyed. We have to create
	    // a fake "current" because the session destroy methods needs
	    // the adapter and object identity to unregister the servant
	    // from the adapter.
	    //
	    Current current;
	    if(!destroy)
	    {
		current.adapter = _adapter;
		current.id = _id;
	    }
	    _session->destroy(current);
	}
	catch(const ObjectNotExistException&)
	{
	}
	catch(const LocalException& ex)
	{
	    Warning out(_adapter->getCommunicator()->getLogger());
	    out << "unexpected exception while reaping node session:\n" << ex;
	}
    }

private:

    const ObjectAdapterPtr _adapter;
    const ObjectPtr _servant;
    BaseSessionI* _session;
    const Identity _id;
};

class NullPermissionsVerifierI : public Glacier2::PermissionsVerifier
{
public:

    bool checkPermissions(const string& userId, const string& password, string&, const Current&) const
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

RegistryI::RegistryI(const CommunicatorPtr& communicator) : _communicator(communicator)
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

    if(!properties->getProperty("IceGrid.Registry.Admin.Endpoints").empty())
    {
	if(!nowarn)
	{
	    Warning out(_communicator->getLogger());
	    out << "administrative endpoints `IceGrid.Registry.Admin.Endpoints' enabled";
	}
    }

    properties->setProperty("Ice.PrintProcessId", "0");
    properties->setProperty("Ice.ServerIdleTime", "0");
    properties->setProperty("IceGrid.Registry.Client.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Server.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Admin.AdapterId", "");
    properties->setProperty("IceGrid.Registry.Internal.AdapterId", "");

    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Admin.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _traceLevels = new TraceLevels(properties, _communicator->getLogger(), false);

    //
    // Create the object adapters.
    //
    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    ObjectAdapterPtr clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");
    ObjectAdapterPtr adminAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Admin");
    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Internal");
    registryAdapter->activate();

    //
    // Get the instance name
    //
    const string instanceNameProperty = "IceGrid.InstanceName";
    string instanceName = properties->getPropertyWithDefault(instanceNameProperty, "IceGrid");
    

    //
    // Add a default servant locator to the client object adapter. The
    // default servant ensure that request on session objects are from
    // the same connection as the connection that created the session.
    //
    _sessionServantLocator = new SessionServantLocatorI(clientAdapter, instanceName);
    clientAdapter->addServantLocator(_sessionServantLocator, "");    

    //
    // Start the reaper threads.
    //
    int nodeSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 10);
    _nodeReaper = new ReapThread(nodeSessionTimeout);
    _nodeReaper->start();

    //
    // TODO: Deprecate AdminSessionTimeout?
    //
    int admSessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.AdminSessionTimeout", 10);
    int sessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionTimeout", admSessionTimeout);
    if(sessionTimeout != nodeSessionTimeout)
    {
	_clientReaper = new ReapThread(sessionTimeout);
	_clientReaper->start();
    }
    else
    {
	_clientReaper = _nodeReaper;
    }

    //
    // Setup the wait queue (used for allocation request timeouts).
    //
    _waitQueue = new WaitQueue();
    _waitQueue->start();
    
    //
    // Create the internal registries (node, server, adapter, object).
    //
    const string envName = "Registry";
    properties->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
    _database = new Database(registryAdapter, envName, instanceName, nodeSessionTimeout, _traceLevels);

    //
    // Create the locator registry and locator interfaces.
    //
    bool dynamicReg = properties->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegistryId = _communicator->stringToIdentity(instanceName + "/" + IceUtil::generateUUID());
    ObjectPrx regPrx = serverAdapter->add(new LocatorRegistryI(_database, dynamicReg), locatorRegistryId);

    Identity locatorId = _communicator->stringToIdentity(instanceName + "/Locator");
    clientAdapter->add(new LocatorI(_communicator, _database, LocatorRegistryPrx::uncheckedCast(regPrx)), locatorId);

    LocatorPrx internalLocatorPrx = LocatorPrx::uncheckedCast(
	registryAdapter->addWithUUID(new LocatorI(_communicator, _database, 
						  LocatorRegistryPrx::uncheckedCast(regPrx))));

    //
    // Create the internal IceStorm service and the registry and node
    // topics.
    //
    _iceStorm = IceStorm::Service::create(_communicator, 
					  registryAdapter, 
					  registryAdapter, 
					  "IceGrid.Registry", 
 					  _communicator->stringToIdentity(instanceName + "/TopicManager"),
					  "Registry");

    NodeObserverTopicPtr nodeTopic = new NodeObserverTopic(_iceStorm->getTopicManager());
    NodeObserverPrx nodeObserver = NodeObserverPrx::uncheckedCast(registryAdapter->addWithUUID(nodeTopic));

    RegistryObserverTopicPtr regTopic = new RegistryObserverTopic(_iceStorm->getTopicManager());
    RegistryObserverPrx registryObserver = RegistryObserverPrx::uncheckedCast(registryAdapter->addWithUUID(regTopic));

    _database->setObservers(registryObserver, nodeObserver);

    //
    // Register the internal and public registry interfaces.
    //
    Identity registryId = _communicator->stringToIdentity(instanceName + "/Registry");
    clientAdapter->add(this, registryId);
    
    Identity internalRegistryId = _communicator->stringToIdentity(instanceName + "/InternalRegistry");
    ObjectPtr internalRegistry = new InternalRegistryI(_database, _nodeReaper, nodeObserver, nodeSessionTimeout);
    registryAdapter->add(internalRegistry, internalRegistryId);

    //
    // Create the query, admin, client session manager and admin
    // session manager interfaces. The session manager interfaces are
    // supposed to be used by Glacier2 to create client or admin
    // sessions (Glacier2 needs access to the admin endpoints to
    // invoke on these interfaces).
    //
    Identity queryId = _communicator->stringToIdentity(instanceName + "/Query");
    clientAdapter->add(new QueryI(_communicator, _database), queryId);

    Identity adminId = _communicator->stringToIdentity(instanceName + "/Admin");
    ObjectPtr admin = new AdminI(_database, this, 0);
    adminAdapter->add(admin, adminId);

    _clientSessionFactory = new ClientSessionFactory(adminAdapter, _database, sessionTimeout, _waitQueue);

    Identity clientSessionMgrId = _communicator->stringToIdentity(instanceName + "/SessionManager");
    adminAdapter->add(new ClientSessionManagerI(_clientSessionFactory), clientSessionMgrId);

    Identity sslClientSessionMgrId = _communicator->stringToIdentity(instanceName + "/SSLSessionManager");
    adminAdapter->add(new ClientSSLSessionManagerI(_clientSessionFactory), sslClientSessionMgrId);

    _adminSessionFactory = new AdminSessionFactory(adminAdapter, _database, sessionTimeout, regTopic, nodeTopic, this);

    Identity adminSessionMgrId = _communicator->stringToIdentity(instanceName + "/AdminSessionManager");
    adminAdapter->add(new AdminSessionManagerI(_adminSessionFactory), adminSessionMgrId);

    Identity sslAdmSessionMgrId = _communicator->stringToIdentity(instanceName + "/AdminSSLSessionManager");
    adminAdapter->add(new AdminSSLSessionManagerI(_adminSessionFactory), sslAdmSessionMgrId);

    //
    // Setup null permissions verifier object, client and admin permissions verifiers.
    //
    Identity nullPermVerifId = _communicator->stringToIdentity(instanceName + "/NullPermissionsVerifier");
    registryAdapter->add(new NullPermissionsVerifierI(), nullPermVerifId);
    addWellKnownObject(registryAdapter->createProxy(nullPermVerifId), Glacier2::PermissionsVerifier::ice_staticId());

    _clientVerifier = getPermissionsVerifier(registryAdapter,
					     internalLocatorPrx,
					     properties->getProperty("IceGrid.Registry.PermissionsVerifier"),
					     properties->getProperty("IceGrid.Registry.CryptPasswords"), 
					     nowarn);

    _adminVerifier = getPermissionsVerifier(registryAdapter,
					    internalLocatorPrx,
					    properties->getProperty("IceGrid.Registry.AdminPermissionsVerifier"),
					    properties->getProperty("IceGrid.Registry.AdminCryptPasswords"),
					    nowarn);

    _sslClientVerifier = getSSLPermissionsVerifier(
	internalLocatorPrx, properties->getProperty("IceGrid.Registry.SSLPermissionsVerifier"), nowarn);

    _sslAdminVerifier = getSSLPermissionsVerifier(
	internalLocatorPrx, properties->getProperty("IceGrid.Registry.AdminSSLPermissionsVerifier"), nowarn);

    //
    // Setup file user account mapper object if the property is set.
    //
    string userAccountFileProperty = properties->getProperty("IceGrid.Registry.UserAccounts");
    if(!userAccountFileProperty.empty())
    {
	try
	{
	    Identity mapperId = _communicator->stringToIdentity(instanceName + "/RegistryUserAccountMapper");
	    registryAdapter->add(new FileUserAccountMapperI(userAccountFileProperty), mapperId);
	    addWellKnownObject(registryAdapter->createProxy(mapperId), UserAccountMapper::ice_staticId());
	}
	catch(const std::string& msg)
	{
	    Error out(_communicator->getLogger());
	    out << msg;
	    return false;
	}
    }

    //
    // Register well known objects with the object registry.
    //
    addWellKnownObject(clientAdapter->createProxy(queryId), Query::ice_staticId());
    addWellKnownObject(clientAdapter->createProxy(registryId), Registry::ice_staticId());

    addWellKnownObject(adminAdapter->createProxy(adminId), Admin::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(clientSessionMgrId), Glacier2::SessionManager::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(adminSessionMgrId), Glacier2::SessionManager::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(sslClientSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(sslAdmSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());

    addWellKnownObject(registryAdapter->createProxy(internalRegistryId), InternalRegistry::ice_staticId());

    //
    // We are ready to go!
    //
    serverAdapter->activate();
    clientAdapter->activate();
    adminAdapter->activate();
    
    return true;
}

void
RegistryI::stop()
{
    _nodeReaper->terminate();
    _nodeReaper->getThreadControl().join();

    if(_nodeReaper != _clientReaper)
    {
	_clientReaper->terminate();
	_clientReaper->getThreadControl().join();
    }

    _nodeReaper = 0;
    _clientReaper = 0;

    _waitQueue->destroy();
    _waitQueue = 0;

    _iceStorm->stop();
    _iceStorm = 0;

    _database->destroy();
    _database = 0;
}

SessionPrx
RegistryI::createSession(const string& user, const string& password, const Current& current)
{
    if(!_clientVerifier)
    {
	PermissionDeniedException ex;
	ex.reason = "no permissions verifier configured, use the property\n";
	ex.reason += "`IceGrid.Registry.PermissionsVerifier' to configure\n";
	ex.reason += "a permissions verifier.";
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
    session->setServantLocator(_sessionServantLocator);
    SessionPrx proxy = SessionPrx::uncheckedCast(_sessionServantLocator->add(session, current.con));
    _clientReaper->add(new SessionReapable(current.adapter, session, proxy->ice_getIdentity()));
    return proxy;    
}

AdminSessionPrx
RegistryI::createAdminSession(const string& user, const string& password, const Current& current)
{
    if(!_adminVerifier)
    {
	PermissionDeniedException ex;
	ex.reason = "no admin permissions verifier configured, use the property\n";
	ex.reason += "`IceGrid.Registry.AdminPermissionsVerifier' to configure\n";
	ex.reason += "a permissions verifier.";
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
    ObjectPrx admin = _sessionServantLocator->add(new AdminI(_database, this, session), current.con);
    session->setAdmin(AdminPrx::uncheckedCast(admin));
    session->setServantLocator(_sessionServantLocator);
    AdminSessionPrx proxy = AdminSessionPrx::uncheckedCast(_sessionServantLocator->add(session, current.con));
    _clientReaper->add(new SessionReapable(current.adapter, session, proxy->ice_getIdentity()));
    return proxy;    
}

SessionPrx
RegistryI::createSessionFromSecureConnection(const Current& current)
{
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
    session->setServantLocator(_sessionServantLocator);
    SessionPrx proxy = SessionPrx::uncheckedCast(_sessionServantLocator->add(session, current.con));
    _clientReaper->add(new SessionReapable(current.adapter, session, proxy->ice_getIdentity()));
    return proxy;
}

AdminSessionPrx
RegistryI::createAdminSessionFromSecureConnection(const Current& current)
{
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
    ObjectPrx admin = _sessionServantLocator->add(new AdminI(_database, this, session), current.con);
    session->setAdmin(AdminPrx::uncheckedCast(admin));
    session->setServantLocator(_sessionServantLocator);
    AdminSessionPrx proxy = AdminSessionPrx::uncheckedCast(_sessionServantLocator->add(session, current.con));
    _clientReaper->add(new SessionReapable(current.adapter, session, proxy->ice_getIdentity()));
    return proxy;    
}

void
RegistryI::shutdown()
{
    _communicator->shutdown();
}

void
RegistryI::addWellKnownObject(const ObjectPrx& proxy, const string& type)
{
    assert(_database);
    try
    {
	_database->removeObject(proxy->ice_getIdentity());
    }
    catch(const IceGrid::ObjectNotRegisteredException&)
    {
    }
    ObjectInfo info;
    info.proxy = proxy;
    info.type = type;
    _database->addObject(info);
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
				  const LocatorPrx& locator,
				  const string& verifierProperty,
				  const string& passwordsProperty,
				  bool nowarn)
{
    //
    // Get the permissions verifier, or create a default one if no
    // verifier is specified.
    //

    ObjectPrx verifier;
    if(!verifierProperty.empty())
    {
	try
	{
	    verifier = _communicator->stringToProxy(verifierProperty);
	}
	catch(const LocalException& ex)
	{
	    Error out(_communicator->getLogger());
	    out << "permissions verifier `" + verifierProperty + "' is invalid:\n" << ex;
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
RegistryI::getSSLPermissionsVerifier(const LocatorPrx& locator, const string& verifierProperty, bool nowarn)
{
    //
    // Get the permissions verifier, or create a default one if no
    // verifier is specified.
    //
    if(verifierProperty.empty())
    {
	return 0;
    }
    
    ObjectPrx verifier;
    try
    {
	verifier = _communicator->stringToProxy(verifierProperty);
    }
    catch(const LocalException& ex)
    {
	Error out(_communicator->getLogger());
	out << "permissions verifier `" + verifierProperty + "' is invalid:\n" << ex;
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
