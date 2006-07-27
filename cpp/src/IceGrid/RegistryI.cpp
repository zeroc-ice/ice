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

    setupThreadPool(properties, "Ice.ThreadPool.Client", 1, 100);
    setupThreadPool(properties, "IceGrid.Registry.Client.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Server.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Admin.ThreadPool", 1, 10);
    setupThreadPool(properties, "IceGrid.Registry.Internal.ThreadPool", 1, 100);

    _traceLevels = new TraceLevels(properties, _communicator->getLogger(), false);

    string replicaName = properties->getProperty("IceGrid.Registry.ReplicaName");

    //
    // Create the internal registry object adapter and activate it.
    //
    ObjectAdapterPtr registryAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Internal");
    registryAdapter->activate();

    //
    // Get the instance name
    //
    if(replicaName.empty())
    {
	const string instanceNameProperty = "IceGrid.InstanceName";
	_instanceName = properties->getPropertyWithDefault(instanceNameProperty, "IceGrid");    
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
    // Start the internal reaper thread.
    //
    int timeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.NodeSessionTimeout", 10);
    timeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.InternalSessionTimeout", timeout);
    _internalReaper = new ReapThread(timeout);
    _internalReaper->start();

    //
    // Create the registry database.
    //
    properties->setProperty("Freeze.DbEnv.Registry.DbHome", dbPath);
    properties->setProperty("Freeze.DbEnv.Registry.DbPrivate", "0");

    //
    // Create the internal IceStorm service.
    //
    _iceStorm = IceStorm::Service::create(_communicator, 
					  registryAdapter, 
					  registryAdapter, 
					  "IceGrid.Registry", 
 					  _communicator->stringToIdentity(_instanceName + "/RegistryTopicManager"),
					  "Registry");

    _database = new Database(registryAdapter, _iceStorm->getTopicManager(), _instanceName, timeout, _traceLevels);

    InternalRegistryPrx internalRegistry;
    if(replicaName.empty())
    {
	_database->initMaster();
	internalRegistry = setupInternalRegistry(registryAdapter, replicaName);
	setupNullPermissionsVerifier(registryAdapter);
	if(!setupUserAccountMapper(registryAdapter))
	{
	    return false;
	}

	NodePrxSeq nodes = registerReplicas(internalRegistry);
	registerNodes(internalRegistry, nodes);
    }
    else
    {
	internalRegistry = setupInternalRegistry(registryAdapter, replicaName);
	_session.create(replicaName, _database, internalRegistry);

	registerNodes(internalRegistry, _session.getNodes());
    }

    ObjectAdapterPtr serverAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Server");
    ObjectAdapterPtr clientAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Client");
    ObjectAdapterPtr adminAdapter = _communicator->createObjectAdapter("IceGrid.Registry.Admin");

    _database->setClientProxy(clientAdapter->createDirectProxy(_communicator->stringToIdentity("dummy")));
    _database->setServerProxy(serverAdapter->createDirectProxy(_communicator->stringToIdentity("dummy")));

    if(replicaName.empty())
    {
	LocatorPrx internalLocator = setupLocator(clientAdapter, serverAdapter, registryAdapter);
	setupQuery(clientAdapter);
	setupAdmin(adminAdapter);
	setupRegistry(clientAdapter);	

	//
	// TODO: Deprecate AdminSessionTimeout?
	//
	int sessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.AdminSessionTimeout", 10);
	_sessionTimeout = properties->getPropertyAsIntWithDefault("IceGrid.Registry.SessionTimeout", sessionTimeout);
	_clientReaper = new ReapThread(_sessionTimeout);
	_clientReaper->start();

	//
	// Add a default servant locator to the client object adapter. The
	// default servant ensure that request on session objects are from
	// the same connection as the connection that created the session.
	//
	_sessionServantLocator = new SessionServantLocatorI(clientAdapter, _instanceName);
	clientAdapter->addServantLocator(_sessionServantLocator, "");    
	
	setupClientSessionFactory(registryAdapter, adminAdapter, internalLocator, nowarn);
	setupAdminSessionFactory(registryAdapter, adminAdapter, internalLocator, nowarn);

	//
	// Register all the replicated well-known objects with all the
	// known client and server endpoints.
	//
	_database->updateReplicatedWellKnownObjects();
    }
    else
    {
	//
	// NOTE: we don't initialize the database here, it's
	// initialized by the observer when the replica registers with
	// the master.
	//
	setupLocator(clientAdapter, serverAdapter, 0);
	setupQuery(clientAdapter);
    }

    //
    // We are ready to go!
    //
    serverAdapter->activate();
    clientAdapter->activate();
    adminAdapter->activate();

    if(!replicaName.empty())
    {
	_session.activate();
    }

    return true;
}

LocatorPrx
RegistryI::setupLocator(const Ice::ObjectAdapterPtr& clientAdapter, 
			const Ice::ObjectAdapterPtr& serverAdapter,
			const Ice::ObjectAdapterPtr& registryAdapter)
{
    //
    // Create the locator registry and locator interfaces.
    //
    bool dynamicReg = _communicator->getProperties()->getPropertyAsInt("IceGrid.Registry.DynamicRegistration") > 0;
    Identity locatorRegistryId = _communicator->stringToIdentity(_instanceName + "/" + IceUtil::generateUUID());
    ObjectPrx regPrx = serverAdapter->add(new LocatorRegistryI(_database, dynamicReg), locatorRegistryId);

    Identity locatorId = _communicator->stringToIdentity(_instanceName + "/Locator");
    clientAdapter->add(new LocatorI(_communicator, _database, LocatorRegistryPrx::uncheckedCast(regPrx)), locatorId);

    if(registryAdapter)
    {
	return LocatorPrx::uncheckedCast(registryAdapter->addWithUUID(
					     new LocatorI(_communicator, 
							  _database, 
							  LocatorRegistryPrx::uncheckedCast(regPrx))));
    }
    else
    {
	return 0;
    }
}

void
RegistryI::setupQuery(const Ice::ObjectAdapterPtr& clientAdapter)
{
    Identity queryId = _communicator->stringToIdentity(_instanceName + "/Query");
    clientAdapter->add(new QueryI(_communicator, _database), queryId);

    // 
    // We don't register the IceGrid::Query object well-known
    // object. This is taken care of by the database. 
    //
//    addWellKnownObject(clientAdapter->createProxy(queryId), Query::ice_staticId());
}

void
RegistryI::setupAdmin(const Ice::ObjectAdapterPtr& adminAdapter)
{
    Identity adminId = _communicator->stringToIdentity(_instanceName + "/Admin");
    ObjectPtr admin = new AdminI(_database, this, 0);
    adminAdapter->add(admin, adminId);
    addWellKnownObject(adminAdapter->createProxy(adminId), Admin::ice_staticId());
}

void
RegistryI::setupRegistry(const Ice::ObjectAdapterPtr& clientAdapter)
{
    Identity registryId = _communicator->stringToIdentity(_instanceName + "/Registry");
    clientAdapter->add(this, registryId);
    addWellKnownObject(clientAdapter->createProxy(registryId), Registry::ice_staticId());    
}

InternalRegistryPrx
RegistryI::setupInternalRegistry(const Ice::ObjectAdapterPtr& registryAdapter, const string& replicaName)
{
    Identity internalRegistryId = _communicator->stringToIdentity(_instanceName + "/InternalRegistry");
    if(!replicaName.empty())
    {
	internalRegistryId.name += "-" + replicaName;
    }
    ObjectPtr internalRegistry = new InternalRegistryI(_database, _internalReaper, _session);
    Ice::ObjectPrx proxy = registryAdapter->add(internalRegistry, internalRegistryId);
    addWellKnownObject(proxy, InternalRegistry::ice_staticId());
    return InternalRegistryPrx::uncheckedCast(proxy);
}

void
RegistryI::setupNullPermissionsVerifier(const Ice::ObjectAdapterPtr& registryAdapter)
{
    Identity nullPermVerifId = _communicator->stringToIdentity(_instanceName + "/NullPermissionsVerifier");
    registryAdapter->add(new NullPermissionsVerifierI(), nullPermVerifId);
    addWellKnownObject(registryAdapter->createProxy(nullPermVerifId), Glacier2::PermissionsVerifier::ice_staticId());
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
	    Identity mapperId = _communicator->stringToIdentity(_instanceName + "/RegistryUserAccountMapper");
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
    return true;
}

void
RegistryI::setupClientSessionFactory(const Ice::ObjectAdapterPtr& registryAdapter,
				     const Ice::ObjectAdapterPtr& adminAdapter,
				     const Ice::LocatorPrx& locator,
				     bool nowarn)
{
    _waitQueue = new WaitQueue(); // Used for for session allocation timeout.
    _waitQueue->start();
    
    _clientSessionFactory = new ClientSessionFactory(adminAdapter, _database, _waitQueue);

    Identity clientSessionMgrId = _communicator->stringToIdentity(_instanceName + "/SessionManager");
    adminAdapter->add(new ClientSessionManagerI(_clientSessionFactory), clientSessionMgrId);

    Identity sslClientSessionMgrId = _communicator->stringToIdentity(_instanceName + "/SSLSessionManager");
    adminAdapter->add(new ClientSSLSessionManagerI(_clientSessionFactory), sslClientSessionMgrId);

    Ice::PropertiesPtr properties = _communicator->getProperties();

    _clientVerifier = getPermissionsVerifier(registryAdapter,
					     locator,
					     properties->getProperty("IceGrid.Registry.PermissionsVerifier"),
					     properties->getProperty("IceGrid.Registry.CryptPasswords"), 
					     nowarn);

    _sslClientVerifier = getSSLPermissionsVerifier(locator, 
						   properties->getProperty("IceGrid.Registry.SSLPermissionsVerifier"), 
						   nowarn);

    addWellKnownObject(adminAdapter->createProxy(clientSessionMgrId), Glacier2::SessionManager::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(sslClientSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
}

void
RegistryI::setupAdminSessionFactory(const Ice::ObjectAdapterPtr& registryAdapter, 
				    const Ice::ObjectAdapterPtr& adminAdapter,
				    const Ice::LocatorPrx& locator,
				    bool nowarn)
{
    _adminSessionFactory = new AdminSessionFactory(adminAdapter, _database, this);

    Identity adminSessionMgrId = _communicator->stringToIdentity(_instanceName + "/AdminSessionManager");
    adminAdapter->add(new AdminSessionManagerI(_adminSessionFactory), adminSessionMgrId);

    Identity sslAdmSessionMgrId = _communicator->stringToIdentity(_instanceName + "/AdminSSLSessionManager");
    adminAdapter->add(new AdminSSLSessionManagerI(_adminSessionFactory), sslAdmSessionMgrId);

    Ice::PropertiesPtr properties = _communicator->getProperties();

    _adminVerifier = getPermissionsVerifier(registryAdapter,
					    locator,
					    properties->getProperty("IceGrid.Registry.AdminPermissionsVerifier"),
					    properties->getProperty("IceGrid.Registry.AdminCryptPasswords"),
					    nowarn);

    _sslAdminVerifier =
	getSSLPermissionsVerifier(locator, 
				  properties->getProperty("IceGrid.Registry.AdminSSLPermissionsVerifier"), 
				  nowarn);

    addWellKnownObject(adminAdapter->createProxy(adminSessionMgrId), Glacier2::SessionManager::ice_staticId());
    addWellKnownObject(adminAdapter->createProxy(sslAdmSessionMgrId), Glacier2::SSLSessionManager::ice_staticId());
}

void
RegistryI::stop()
{
    _session.destroy();

    _database->clearTopics();

    if(_clientReaper)
    {
	_clientReaper->terminate();
	_clientReaper->getThreadControl().join();
	_clientReaper = 0;
    }

    _internalReaper->terminate();
    _internalReaper->getThreadControl().join();
    _internalReaper = 0;

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

    _database->destroy();
    _database = 0;
}

SessionPrx
RegistryI::createSession(const string& user, const string& password, const Current& current)
{
    assert(_clientReaper && _clientSessionFactory);

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
    assert(_clientReaper && _adminSessionFactory);

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
    assert(_clientReaper && _clientSessionFactory);

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
    assert(_clientReaper && _adminSessionFactory);

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

int
RegistryI::getSessionTimeout(const Ice::Current& current) const
{
    return _sessionTimeout;
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
    ObjectInfo info;
    info.proxy = proxy;
    info.type = type;
    _database->addObject(info, true);
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

NodePrxSeq
RegistryI::registerReplicas(const InternalRegistryPrx& internalRegistry)
{
    set<NodePrx> nodes;
    InternalRegistryPrxSeq replicas = internalRegistry->getReplicas();
    for(InternalRegistryPrxSeq::const_iterator r = replicas.begin(); r != replicas.end(); ++r)
    {
	if((*r)->ice_getIdentity() != internalRegistry->ice_getIdentity())
	{
	    try
	    {
		(*r)->registerWithReplica(internalRegistry);
		NodePrxSeq nds = (*r)->getNodes();
		nodes.insert(nds.begin(), nds.end());
	    }
	    catch(const Ice::LocalException&)
	    {
		// TODO: Cleanup the database?
	    }
	}
    }
    
    if(nodes.empty())
    {
	NodePrxSeq nds = internalRegistry->getNodes();
	nodes.insert(nds.begin(), nds.end());
    }

    return NodePrxSeq(nodes.begin(), nodes.end());
}

void
RegistryI::registerNodes(const InternalRegistryPrx& internalRegistry, const NodePrxSeq& nodes)
{
    for(NodePrxSeq::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
	try
	{
	    NodePrx::uncheckedCast(*p)->registerWithReplica(internalRegistry);
	}
	catch(const Ice::LocalException&)
	{
	    // TODO: Cleanup the database?
	}
    }
}
