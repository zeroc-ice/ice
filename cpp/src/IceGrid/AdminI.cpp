// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/AdminI.h>
#include <IceGrid/DescriptorVisitor.h>
#include <IceGrid/DescriptorUtil.h>
#include <IceGrid/Registry.h>
#include <Ice/LoggerUtil.h>
#include <Ice/TraceUtil.h>
#include <Ice/SliceChecksums.h>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class Deployer : public DescriptorVisitor
{
public:

    Deployer(const NodeRegistryPtr&, 
	     const ApplicationRegistryPtr&, 
	     const ServerRegistryPtr&,
	     const AdapterRegistryPtr&,
	     const ObjectRegistryPtr&,
	     const Ice::LoggerPtr&);

    Deployer(const Deployer&);

protected:

    friend class Cleaner;

    void exception(const string&);
    void exception(const string&, const Ice::LocalException&);

    const NodeRegistryPtr _nodeRegistry;
    const ApplicationRegistryPtr _applicationRegistry;
    const ServerRegistryPtr _serverRegistry;
    const AdapterRegistryPtr _adapterRegistry;
    const ObjectRegistryPtr _objectRegistry;
    const Ice::LoggerPtr _logger;
};

class Cleaner : public DescriptorVisitor
{
public:

    Cleaner(const NodeRegistryPtr&, 
	    const ApplicationRegistryPtr&, 
	    const ServerRegistryPtr&,
	    const AdapterRegistryPtr&,
	    const ObjectRegistryPtr&,
	    const Ice::LoggerPtr&);

    Cleaner(const Deployer&);
    Cleaner(const Cleaner&);

protected:

    void exception(const string&);
    void exception(const string&, const Ice::LocalException&);

    const NodeRegistryPtr _nodeRegistry;
    const ApplicationRegistryPtr _applicationRegistry;
    const ServerRegistryPtr _serverRegistry;
    const AdapterRegistryPtr _adapterRegistry;
    const ObjectRegistryPtr _objectRegistry;
    const Ice::LoggerPtr _logger;
};

class ApplicationDeployer : public Deployer
{
public:

    ApplicationDeployer(const NodeRegistryPtr&, 
			const ApplicationRegistryPtr&, 
			const ServerRegistryPtr&,
			const AdapterRegistryPtr&,
			const ObjectRegistryPtr&,
			const Ice::LoggerPtr&);
    
    ApplicationDeployer(const Deployer&);
    
    void deploy(const ApplicationDescriptorPtr&);

private:

    virtual bool visitApplicationStart(const ApplicationWrapper&, const ApplicationDescriptorPtr&);
    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);

    ApplicationDescriptorPtr _deployed;
};

class ServerDeployer : public Deployer
{
public:

    ServerDeployer(const NodeRegistryPtr&,
		   const ApplicationRegistryPtr&,
		   const ServerRegistryPtr&, 
		   const AdapterRegistryPtr&, 
		   const ObjectRegistryPtr&,
		   const Ice::LoggerPtr&);

    ServerDeployer(const Deployer&);    

    void deploy(const ServerDescriptorPtr&, const string& = string());

private:

    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual void visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual bool visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr&);
    virtual void visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor&);
    virtual bool visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor&);
    virtual void visitObject(const ObjectWrapper&, const ObjectDescriptor&);

    ServerDescriptorPtr _deployed;

    string _currentNodeName;
    NodePrx _currentNode;
    ServerPrx _currentServer;
    PropertyDescriptorSeq _currentProperties;
    string _backup;
};

class ApplicationCleaner : public Cleaner
{
public:

    ApplicationCleaner(const NodeRegistryPtr&, 
		       const ApplicationRegistryPtr&, 
		       const ServerRegistryPtr&, 
		       const AdapterRegistryPtr&,
		       const ObjectRegistryPtr&,
		       const Ice::LoggerPtr&);
    
    ApplicationCleaner(const Deployer&);

    void clean(const ApplicationDescriptorPtr&);

private:

    virtual void visitApplicationEnd(const ApplicationWrapper&, const ApplicationDescriptorPtr&);
    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);
};

class ServerCleaner : public Cleaner
{
public:

    ServerCleaner(const NodeRegistryPtr&, 
		  const ApplicationRegistryPtr&, 
		  const ServerRegistryPtr&, 
		  const AdapterRegistryPtr&,
		  const ObjectRegistryPtr&,
		  const Ice::LoggerPtr&);
    
    ServerCleaner(const Deployer&);
    ServerCleaner(const Cleaner&);

    void clean(const ServerDescriptorPtr&, const string& = string());

private:
    
    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual void visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr&);
    virtual bool visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr&);
    virtual bool visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor&);
    virtual void visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor&);
    virtual void visitObject(const ObjectWrapper&, const ObjectDescriptor&);

    string _currentNodeName;
    ServerPrx _currentServer;
    string _backup;
};

class ApplicationUpdater : public Deployer
{
public:

    ApplicationUpdater(const NodeRegistryPtr&, 
		       const ApplicationRegistryPtr&, 
		       const ServerRegistryPtr&,
		       const AdapterRegistryPtr&,
		       const ObjectRegistryPtr&,
		       const Ice::LoggerPtr&);
    
    ApplicationUpdater(const Deployer&);
    
    void update(const ApplicationDescriptorPtr&);
    
private:
    
    virtual bool visitApplicationStart(const ApplicationWrapper&, const ApplicationDescriptorPtr&);
    virtual void visitApplicationEnd(const ApplicationWrapper&, const ApplicationDescriptorPtr&);
    virtual bool visitServerStart(const ServerWrapper&, const ServerDescriptorPtr&);

    ApplicationDescriptorPtr _deployed;
    ApplicationDescriptorPtr _added;
    ApplicationDescriptorPtr _removed;
    map<string, pair<string, string> > _backup;
};

}

Deployer::Deployer(const NodeRegistryPtr& nodeRegistry, 
		   const ApplicationRegistryPtr& applicationRegistry,
		   const ServerRegistryPtr& serverRegistry, 
		   const AdapterRegistryPtr& adapterRegistry,
		   const ObjectRegistryPtr& objectRegistry,
		   const Ice::LoggerPtr& logger) :
    _nodeRegistry(nodeRegistry),
    _applicationRegistry(applicationRegistry),
    _serverRegistry(serverRegistry),
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry),
    _logger(logger)
{
}

Deployer::Deployer(const Deployer& deployer) : 
    _nodeRegistry(deployer._nodeRegistry),
    _applicationRegistry(deployer._applicationRegistry),
    _serverRegistry(deployer._serverRegistry),
    _adapterRegistry(deployer._adapterRegistry),
    _objectRegistry(deployer._objectRegistry),
    _logger(deployer._logger)
{
}

void
Deployer::exception(const string& msg)
{
    DeploymentException ex;
    ex.reason = msg;
    throw ex;
}

void
Deployer::exception(const string& msg, const Ice::LocalException& ex)
{
    ostringstream os;
    os << msg << "\nException: " << ex;
    DeploymentException e;
    e.reason = os.str();
    throw e;
}

Cleaner::Cleaner(const NodeRegistryPtr& nodeRegistry, 
		 const ApplicationRegistryPtr& applicationRegistry,
		 const ServerRegistryPtr& serverRegistry, 
		 const AdapterRegistryPtr& adapterRegistry,
		 const ObjectRegistryPtr& objectRegistry,
		 const Ice::LoggerPtr& logger) :
    _nodeRegistry(nodeRegistry),
    _applicationRegistry(applicationRegistry),
    _serverRegistry(serverRegistry),
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry),
    _logger(logger)
{
}

Cleaner::Cleaner(const Deployer& deployer) : 
    _nodeRegistry(deployer._nodeRegistry),
    _applicationRegistry(deployer._applicationRegistry),
    _serverRegistry(deployer._serverRegistry),
    _adapterRegistry(deployer._adapterRegistry),
    _objectRegistry(deployer._objectRegistry),
    _logger(deployer._logger)
{
}

Cleaner::Cleaner(const Cleaner& cleaner) : 
    _nodeRegistry(cleaner._nodeRegistry),
    _applicationRegistry(cleaner._applicationRegistry),
    _serverRegistry(cleaner._serverRegistry),
    _adapterRegistry(cleaner._adapterRegistry),
    _objectRegistry(cleaner._objectRegistry),
    _logger(cleaner._logger)
{
}

void
Cleaner::exception(const string& msg)
{
    Ice::Warning out(_logger);
    out << msg;
}

void
Cleaner::exception(const string& msg, const Ice::LocalException& ex)
{
    Ice::Warning out(_logger);
    out << msg << "\nException: " << ex;
}

ApplicationDeployer::ApplicationDeployer(const NodeRegistryPtr& nodeRegistry, 
					 const ApplicationRegistryPtr& applicationRegistry,
					 const ServerRegistryPtr& serverRegistry, 
					 const AdapterRegistryPtr& adapterRegistry,
					 const ObjectRegistryPtr& objectRegistry,
					 const Ice::LoggerPtr& logger) :
    Deployer(nodeRegistry, applicationRegistry, serverRegistry, adapterRegistry, objectRegistry, logger)
{
}

ApplicationDeployer::ApplicationDeployer(const Deployer& deployer) : 
    Deployer(deployer)
{
}

void
ApplicationDeployer::deploy(const ApplicationDescriptorPtr& descriptor)
{
    try
    {
	ApplicationWrapper(descriptor).visit(*this);
    }
    catch(const DeploymentException&)
    {
	if(_deployed)
	{
	    ApplicationCleaner cleaner(*this);
	    cleaner.clean(_deployed);
	}
	throw;
    }
}

bool
ApplicationDeployer::visitApplicationStart(const ApplicationWrapper&, const ApplicationDescriptorPtr& application)
{
    try
    {
	_applicationRegistry->add(application->name);
	_deployed = new ApplicationDescriptor();
	_deployed->name = application->name;
    }
    catch(const ApplicationExistsException&)
    {
	exception("application `" + application->name + "' already exists");
    }
    return true;
}

bool
ApplicationDeployer::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    ServerDeployer deployer(*this);
    deployer.deploy(server);
    _deployed->servers.push_back(server);
    return false;
}

ApplicationUpdater::ApplicationUpdater(const NodeRegistryPtr& nodeRegistry, 
				       const ApplicationRegistryPtr& applicationRegistry,
				       const ServerRegistryPtr& serverRegistry, 
				       const AdapterRegistryPtr& adapterRegistry,
				       const ObjectRegistryPtr& objectRegistry,
				       const Ice::LoggerPtr& logger) :
    Deployer(nodeRegistry, applicationRegistry, serverRegistry, adapterRegistry, objectRegistry, logger)
{
}

ApplicationUpdater::ApplicationUpdater(const Deployer& deployer) : 
    Deployer(deployer)
{
}

void
ApplicationUpdater::update(const ApplicationDescriptorPtr& descriptor)
{
    try
    {
	ApplicationWrapper(descriptor).visit(*this);
    }
    catch(const DeploymentException&)
    {
	if(_added)
	{
	    for(ServerDescriptorSeq::const_iterator p = _added->servers.begin(); p != _added->servers.end(); ++p)
	    {
		//
		// If there's a backup directory for this server and it's on the same node we pass
		// the backup directory to backup the databases.
		//
		if(_backup[(*p)->name].first == (*p)->node)
		{
		    ServerCleaner(*this).clean(*p, _backup[(*p)->name].second);
		}
		else
		{
		    ServerCleaner(*this).clean(*p);
		}
	    }
	}
	if(_removed)
	{
	    for(ServerDescriptorSeq::const_iterator p = _removed->servers.begin(); p != _removed->servers.end(); ++p)
	    {
		try
		{
		    ServerDeployer(*this).deploy(*p, _backup[(*p)->name].second);
		}
		catch(DeploymentException& ex)
		{
		    cerr << "failed to add back updated server:\n" << ex << endl;
		}
	    }
	}
	throw;
    }

    //
    // Destroy all the temporary backup directories.
    //
    for(map<string, pair<string, string> >::const_iterator p = _backup.begin(); p != _backup.end(); ++p)
    {
	if(!p->second.first.empty())
	{
	    _nodeRegistry->findByName(p->second.first)->destroyTmpDir(p->second.second);
	}
    }
}

bool
ApplicationUpdater::visitApplicationStart(const ApplicationWrapper&, const ApplicationDescriptorPtr& application)
{
    try
    {
	_deployed = _applicationRegistry->getDescriptor(application->name);
	_added = new ApplicationDescriptor();
	_removed = new ApplicationDescriptor();
    }
    catch(const ApplicationNotExistException&)
    {
	exception("application `" + application->name + "' doesn't exists");
    }
    return true;
}

void
ApplicationUpdater::visitApplicationEnd(const ApplicationWrapper&, const ApplicationDescriptorPtr& application)
{
    //
    // Remove servers which don't exist anymore.
    // 
    for(ServerDescriptorSeq::const_iterator p = _deployed->servers.begin(); p != _deployed->servers.end(); ++p)
    {
	bool found = false;
	for(ServerDescriptorSeq::const_iterator q = application->servers.begin(); q != application->servers.end(); ++q)
	{
	    if((*p)->name == (*q)->name)
	    {
		found = true;
		break;
	    }
	}
	if(!found)
	{
	    ServerCleaner(*this).clean(*p);
	}
    }
}

bool
ApplicationUpdater::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    for(ServerDescriptorSeq::const_iterator p = _deployed->servers.begin(); p != _deployed->servers.end(); ++p)
    {
	if((*p)->name == server->name)
	{
	    if(equal(*p, server))
	    {
		//
		// Nothing to do, the descriptors are the same.
		//
		return false;
	    }

	    _backup[server->name].second = _nodeRegistry->findByName((*p)->node)->createTmpDir();
	    _backup[server->name].first = (*p)->node;

	    ServerCleaner(*this).clean(*p, _backup[server->name].second);
	    _removed->servers.push_back(*p);

	    if(server->node == ((*p)->node))
	    {
		ServerDeployer(*this).deploy(server, _backup[server->name].second);
	    }
	    else
	    {
		ServerDeployer(*this).deploy(server);
	    }

	    _added->servers.push_back(server);
	    return false;
	}
    }

    ServerDeployer(*this).deploy(server, _backup[server->name].second);
    _added->servers.push_back(server);
    return false;
}

ServerDeployer::ServerDeployer(const NodeRegistryPtr& nodeRegistry, 
			       const ApplicationRegistryPtr& applicationRegistry,
			       const ServerRegistryPtr& serverRegistry, 
			       const AdapterRegistryPtr& adapterRegistry,
			       const ObjectRegistryPtr& objectRegistry,
			       const Ice::LoggerPtr& logger) :
    Deployer(nodeRegistry, applicationRegistry, serverRegistry, adapterRegistry, objectRegistry, logger)
{
}

ServerDeployer::ServerDeployer(const Deployer& deployer) : 
    Deployer(deployer)
{
}

void
ServerDeployer::deploy(const ServerDescriptorPtr& descriptor, const string& backup)
{
    _backup = backup;
    try
    {
	ServerWrapper(descriptor).visit(*this);
    }
    catch(const DeploymentException&)
    {
	if(_deployed)
	{
	    ServerCleaner cleaner(*this);
	    cleaner.clean(_deployed);
	}
	throw;
    }

}

bool
ServerDeployer::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    try
    {
	try
	{
	    _currentNodeName = server->node;
	    _currentNode = _nodeRegistry->findByName(server->node);
	    _currentServer = _currentNode->createServer(server->name, server);
	    _currentProperties.clear();
	    PropertyDescriptor prop;
	    prop.name = "# Server configuration";
	    _currentProperties.push_back(prop);
	    prop.name = "Ice.ProgramName";
	    prop.value = server->name;
	    _currentProperties.push_back(prop);
	    copy(server->properties.begin(), server->properties.end(), back_inserter(_currentProperties));
	    _serverRegistry->add(server->name, _currentServer, server);

	    if(JavaIceBoxDescriptorPtr::dynamicCast(server))
	    {
		_deployed = new JavaIceBoxDescriptor();
	    }
	    else if(CppIceBoxDescriptorPtr::dynamicCast(server))
	    {
		_deployed = new CppIceBoxDescriptor();
	    }
	    else if(JavaServerDescriptorPtr::dynamicCast(server))
	    {
		_deployed = new JavaServerDescriptor();
	    }
	    else
	    {
		_deployed = new ServerDescriptor();
	    }
	    _deployed->node = server->node;
	    _deployed->name = server->name;

	    if(!server->application.empty())
	    {
		_applicationRegistry->registerServer(server->application, server->name);
		_deployed->application = server->application;
	    }
	}
	catch(const ServerExistsException&)
	{
	    _currentServer->destroy();
	    _currentServer = 0;
	    exception("server `" + server->name + "' already exists");
	}
	catch(const ApplicationNotExistException&)
	{
	    exception("application `" + server->application + "' doesn't exist");
	}
	catch(const NodeNotExistException&)
	{
	    exception("couldn't find node `" + server->node + "'");
	}

	_currentServer->setActivationMode(server->activation);
	_currentServer->setExePath(server->exe);
	_currentServer->setPwd(server->pwd);
	_currentServer->setEnvs(server->envs);
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + server->node + "'", ex);
    }

    return true;
}

void
ServerDeployer::visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    assert(_currentServer);

    try
    {
	Ice::StringSeq options;
	
	JavaServerDescriptorPtr javaDesc = JavaServerDescriptorPtr::dynamicCast(server);
	if(javaDesc)
	{
	    copy(javaDesc->jvmOptions.begin(), javaDesc->jvmOptions.end(), back_inserter(options));
	    options.push_back("-ea");
	    options.push_back(javaDesc->className);
	    copy(javaDesc->options.begin(), javaDesc->options.end(), back_inserter(options));
	}
	else
	{
	    options = server->options;
	}

	CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(server);
	JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(server);
	
	const ServiceDescriptorSeq& services = 
	    cppIceBox ? cppIceBox->services : (javaIceBox ? javaIceBox->services : ServiceDescriptorSeq());

	if(!services.empty())
	{
	    PropertyDescriptor prop;
	    prop.name = "IceBox.LoadOrder";
 	    for(ServiceDescriptorSeq::const_iterator p = services.begin(); p != services.end(); ++p)
	    {
		prop.value += (*p)->name + " ";
	    }
	    _currentProperties.push_back(prop);
	}
	
	if(!_currentProperties.empty())
	{
	    string path = _currentServer->addConfigFile("config", _currentProperties);
	    options.push_back("--Ice.Config=" + path);

	    _deployed->properties = _currentProperties;
	}
	
	_currentServer->setOptions(options);
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }
}

bool
ServerDeployer::visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr& service)
{
    assert(_currentServer);

    try
    {
	string path = _currentServer->addConfigFile("config_" + service->name, service->properties);
	
	ServiceDescriptorPtr svc = new ServiceDescriptor();
	svc->name = service->name;
	svc->properties = service->properties;	
	if(JavaIceBoxDescriptorPtr::dynamicCast(_deployed))
	{	    
	    JavaIceBoxDescriptorPtr::dynamicCast(_deployed)->services.push_back(svc);
	}
	else
	{
	    CppIceBoxDescriptorPtr::dynamicCast(_deployed)->services.push_back(svc);
	}

	PropertyDescriptor prop;
	_currentProperties.push_back(prop);
	prop.name = "#";
	_currentProperties.push_back(prop);
	prop.name = "# Service " + service->name;
	_currentProperties.push_back(prop);
	prop.name = "#";
	_currentProperties.push_back(prop);
	prop.name = "IceBox.Service." + service->name;
	prop.value = service->entry + " --Ice.Config=" + path; 
	_currentProperties.push_back(prop);
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }

    return true;
}

void
ServerDeployer::visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor& dbEnv)
{
    assert(_currentServer);

    try
    {
	string path = _currentServer->addDbEnv(dbEnv, _backup);
	_deployed->dbEnvs.push_back(dbEnv);

	PropertyDescriptor prop;
	_currentProperties.push_back(prop);
	prop.name = "# DbEnv " + dbEnv.name;
	_currentProperties.push_back(prop);
	prop.name = "Freeze.DbEnv." + dbEnv.name + ".DbHome";
	prop.value = path;
	_currentProperties.push_back(prop);	
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }
}

bool
ServerDeployer::visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor& adapter)
{
    assert(_currentServer);

    ServerAdapterPrx serverAdapter;
    try
    {
	serverAdapter = _currentNode->createServerAdapter(_currentServer, adapter.id);
	_adapterRegistry->add(adapter.id, serverAdapter);
	_currentServer->addAdapter(serverAdapter, adapter.registerProcess);

	AdapterDescriptor adpt;
	adpt.id = adapter.id;
	adpt.name = adapter.name;
	adpt.endpoints = adapter.endpoints;
	adpt.registerProcess = adapter.registerProcess;
	_deployed->adapters.push_back(adpt);
	
	PropertyDescriptor prop;
	_currentProperties.push_back(prop);
	prop.name = "# Adapter " + adapter.name;
	_currentProperties.push_back(prop);
	prop.name = adapter.name + ".Endpoints";
	prop.value = adapter.endpoints;
	_currentProperties.push_back(prop);
	prop.name = adapter.name + ".AdapterId";
	prop.value = adapter.id;
	_currentProperties.push_back(prop);
	if(adapter.registerProcess)
	{
	    prop.name = adapter.name + ".RegisterProcess";
	    prop.value = "1";
	    _currentProperties.push_back(prop);
	}
    }
    catch(const AdapterExistsException&)
    {
	serverAdapter->destroy();
	exception("adapter `" + adapter.id + "' already exists");
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }

    return true;
}

void
ServerDeployer::visitObject(const ObjectWrapper&, const ObjectDescriptor& object)
{
    assert(_currentServer);

    try
    {
	_objectRegistry->add(object);
	_deployed->adapters.back().objects.push_back(object);
    }
    catch(const ObjectExistsException&)
    {
	exception("object `" + Ice::identityToString(object.proxy->ice_getIdentity()) + "' already exists");
    }
}

ApplicationCleaner::ApplicationCleaner(const NodeRegistryPtr& nodeRegistry, 
				       const ApplicationRegistryPtr& applicationRegistry,
				       const ServerRegistryPtr& serverRegistry, 
				       const AdapterRegistryPtr& adapterRegistry,
				       const ObjectRegistryPtr& objectRegistry,
				       const Ice::LoggerPtr& logger) :
    Cleaner(nodeRegistry, applicationRegistry, serverRegistry, adapterRegistry, objectRegistry, logger)
{
}

ApplicationCleaner::ApplicationCleaner(const Deployer& deployer) : 
    Cleaner(deployer)
{
}

void
ApplicationCleaner::clean(const ApplicationDescriptorPtr& descriptor)
{
    ApplicationWrapper(descriptor).visit(*this);
}

void
ApplicationCleaner::visitApplicationEnd(const ApplicationWrapper&, const ApplicationDescriptorPtr& application)
{
    try
    {
	_applicationRegistry->remove(application->name);
    }
    catch(const ApplicationNotExistException&)
    {
	exception("application `" + application->name + " doesn't exist");
    }
}

bool
ApplicationCleaner::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    ServerCleaner cleaner(*this);
    cleaner.clean(server);
    return false;
}

ServerCleaner::ServerCleaner(const NodeRegistryPtr& nodeRegistry, 
			     const ApplicationRegistryPtr& applicationRegistry,
			     const ServerRegistryPtr& serverRegistry, 
			     const AdapterRegistryPtr& adapterRegistry,
			     const ObjectRegistryPtr& objectRegistry,
			     const Ice::LoggerPtr& logger) :
    Cleaner(nodeRegistry, applicationRegistry, serverRegistry, adapterRegistry, objectRegistry, logger)
{
}

ServerCleaner::ServerCleaner(const Deployer& deployer) : 
    Cleaner(deployer)
{
}

ServerCleaner::ServerCleaner(const Cleaner& cleaner) : 
    Cleaner(cleaner)
{
}

void
ServerCleaner::clean(const ServerDescriptorPtr& descriptor, const string& backup)
{
    _backup = backup;
    ServerWrapper(descriptor).visit(*this);
}

bool
ServerCleaner::visitServerStart(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    _currentNodeName = server->node;

    try
    {
	_currentServer = _serverRegistry->findByName(server->name);
	_currentServer->setActivationMode(Manual);
	_currentServer->stop();
    }
    catch(const ServerNotExistException&)
    {
	exception("server `" + server->name + "' doesn't exist");
    }
    catch(const Ice::ObjectNotExistException&)
    {
	_currentServer = 0;
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + server->node + "'", ex);
    }

    try
    {
	_serverRegistry->remove(server->name);
    }
    catch(const ServerNotExistException&)
    {
	exception("server `" + server->name + "' doesn't exist");
    }

    try
    {
	if(!server->application.empty())
	{
	    _applicationRegistry->unregisterServer(server->application, server->name);
	}
    }
    catch(const ApplicationNotExistException&)
    {
	exception("application `" + server->application + "' doesn't exist");
    }

    return true;
}

void
ServerCleaner::visitServerEnd(const ServerWrapper&, const ServerDescriptorPtr& server)
{
    if(_currentServer)
    {
	try
	{
	    _currentServer->removeConfigFile("config");
	    _currentServer->destroy();
	}
	catch(const Ice::ObjectNotExistException&)
	{
	}
	catch(const Ice::LocalException& ex)
	{
	    exception("couldn't contact node `" + _currentNodeName + "'", ex);
	}
    }
}

bool
ServerCleaner::visitServiceStart(const ServiceWrapper&, const ServiceDescriptorPtr& service)
{
    if(!_currentServer)
    {
	return true;
    }

    try
    {
	_currentServer->removeConfigFile("config_" + service->name);
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }

    return true;
}

bool
ServerCleaner::visitAdapterStart(const AdapterWrapper&, const AdapterDescriptor& adapter)
{
    AdapterPrx adpt;
    try
    {
	adpt = _adapterRegistry->remove(adapter.id, 0);
	adpt->destroy();
    }
    catch(const AdapterNotExistException&)
    {
	exception("adapter `" + adapter.id + "' doesn't exist");
    }
    catch(const Ice::ObjectNotExistException&)
    {
    }
    catch(const Ice::LocalException& ex)
    {
	exception("couldn't contact node `" + _currentNodeName + "'", ex);
    }

    if(_currentServer)
    {
	try
	{
	    _currentServer->removeAdapter(ServerAdapterPrx::uncheckedCast(adpt));
	}
	catch(const Ice::LocalException& ex)
	{
	    exception("couldn't contact node `" + _currentNodeName + "'", ex);
	}
    }

    return true;
}

void
ServerCleaner::visitDbEnv(const DbEnvWrapper&, const DbEnvDescriptor& dbEnv)
{
    if(_currentServer)
    {
	try
	{
	    _currentServer->removeDbEnv(dbEnv, _backup);
	}
	catch(const Ice::LocalException& ex)
	{
	    exception("couldn't contact node `" + _currentNodeName + "'", ex);
	}
    }
}

void
ServerCleaner::visitObject(const ObjectWrapper&, const ObjectDescriptor& object)
{
    try
    {
	_objectRegistry->remove(object.proxy->ice_getIdentity());
    }
    catch(const ObjectNotExistException&)
    {
	exception("object `" + Ice::identityToString(object.proxy->ice_getIdentity()) + "' doesn't exist");
    }
}

AdminI::AdminI(const CommunicatorPtr& communicator, 
	       const NodeRegistryPtr& nodeRegistry,
	       const ApplicationRegistryPtr& applicationRegistry, 
	       const ServerRegistryPtr& serverRegistry, 
	       const AdapterRegistryPtr& adapterRegistry,
	       const ObjectRegistryPtr& objectRegistry,
	       const RegistryPtr& registry) :
    _communicator(communicator),
    _nodeRegistry(nodeRegistry),
    _applicationRegistry(applicationRegistry),
    _serverRegistry(serverRegistry),
    _adapterRegistry(adapterRegistry),
    _objectRegistry(objectRegistry),
    _registry(registry)
{
}

AdminI::~AdminI()
{
}

void
AdminI::addApplication(const ApplicationDescriptorPtr& descriptor, const Current&)
{
    ApplicationDeployer(_nodeRegistry, 
			_applicationRegistry,
			_serverRegistry, 
			_adapterRegistry, 
			_objectRegistry,
			_communicator->getLogger()).deploy(descriptor);
}

void
AdminI::updateApplication(const ApplicationDescriptorPtr& descriptor, const Current&)
{
    ApplicationUpdater(_nodeRegistry, 
		       _applicationRegistry,
		       _serverRegistry, 
		       _adapterRegistry, 
		       _objectRegistry,
		       _communicator->getLogger()).update(descriptor);
}

void
AdminI::removeApplication(const string& name, const Current&)
{
    ApplicationCleaner(_nodeRegistry, 
		       _applicationRegistry, 
		       _serverRegistry, 
		       _adapterRegistry, 
		       _objectRegistry,
		       _communicator->getLogger()).clean(_applicationRegistry->getDescriptor(name));
}

ApplicationDescriptorPtr
AdminI::getApplicationDescriptor(const string& name, const Current&) const
{
    return _applicationRegistry->getDescriptor(name);
}

Ice::StringSeq
AdminI::getAllApplicationNames(const Current&) const
{
    return _applicationRegistry->getAll();
}

void
AdminI::addServer(const ServerDescriptorPtr& server, const Current&)
{
    if(!server->application.empty())
    {
	DeploymentException ex;
	ex.reason = "You need to update the descriptor of the application `" + server->application + "'" +
	    "to add this server.";
	throw ex;
    }

    ServerDeployer(_nodeRegistry, 
		   _applicationRegistry,
		   _serverRegistry,
		   _adapterRegistry,
		   _objectRegistry,
		   _communicator->getLogger()).deploy(server);
}

void
AdminI::updateServer(const ServerDescriptorPtr& server, const Current&)
{
    ServerDescriptorPtr orig = _serverRegistry->getDescriptor(server->name);
    if(!orig->application.empty())
    {
	DeploymentException ex;
	ex.reason = "You need to update the descriptor of the application `" + orig->application + "'" +
	    "to add this server.";
	throw ex;
    }
    
    if(!equal(orig, server))
    {
	string dir = _nodeRegistry->findByName(orig->node)->createTmpDir();

	ServerCleaner(_nodeRegistry, 
		      _applicationRegistry, 
		      _serverRegistry, 
		      _adapterRegistry,
		      _objectRegistry,
		      _communicator->getLogger()).clean(orig, dir);	

	try
	{
	    if(server->node == orig->node)
	    {
		ServerDeployer(_nodeRegistry, 
			       _applicationRegistry, 
			       _serverRegistry, 
			       _adapterRegistry,
			       _objectRegistry,
			       _communicator->getLogger()).deploy(server, dir);
	    }
	    else
	    {
		ServerDeployer(_nodeRegistry, 
			       _applicationRegistry, 
			       _serverRegistry, 
			       _adapterRegistry,
			       _objectRegistry,
			       _communicator->getLogger()).deploy(server);
	    }
	}
	catch(const DeploymentException&)
	{
	    try
	    {
		ServerDeployer(_nodeRegistry, 
			       _applicationRegistry, 
			       _serverRegistry, 
			       _adapterRegistry,
			       _objectRegistry,
			       _communicator->getLogger()).deploy(orig, dir);
	    }
	    catch(DeploymentException& ex)
	    {
		DeploymentException e;
 		e.reason = "Failed to update and rollback original application: " + ex.reason;
		throw e;
	    }
	}

	_nodeRegistry->findByName(orig->node)->destroyTmpDir(dir);
    }
}

void
AdminI::removeServer(const string& name, const Current&)
{
    ServerDescriptorPtr server = _serverRegistry->getDescriptor(name);
    if(!server->application.empty())
    {
	DeploymentException ex;
	ex.reason = "You need to update the descriptor of the application `" + server->application + "'" +
	    " to remove this server.";
	throw ex;
    }

    ServerCleaner(_nodeRegistry, 
		  _applicationRegistry, 
		  _serverRegistry, 
		  _adapterRegistry,
		  _objectRegistry,
		  _communicator->getLogger()).clean(server);
}

ServerDescriptorPtr
AdminI::getServerDescriptor(const string& name, const Current&) const
{
    return _serverRegistry->getDescriptor(name);
}

ServerState
AdminI::getServerState(const string& name, const Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getState();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

Ice::Int
AdminI::getServerPid(const string& name, const Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getPid();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

bool
AdminI::startServer(const string& name, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->start(Manual);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::stopServer(const string& name, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	server->stop();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::sendSignal(const string& name, const string& signal, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	server->sendSignal(signal);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::writeMessage(const string& name, const string& message, Int fd, const Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	server->writeMessage(message, fd);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}


StringSeq
AdminI::getAllServerNames(const Current&) const
{
    return _serverRegistry->getAll();
}

ServerActivation 
AdminI::getServerActivation(const ::std::string& name, const Ice::Current&) const
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	return server->getActivationMode();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void 
AdminI::setServerActivation(const ::std::string& name, ServerActivation mode, const Ice::Current&)
{
    ServerPrx server = _serverRegistry->findByName(name);
    try
    {
	server->setActivationMode(mode);
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw ServerNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

string 
AdminI::getAdapterEndpoints(const string& id, const Current&) const
{
    AdapterPrx adapter = _adapterRegistry->findById(id);
    try
    {
	return _communicator->proxyToString(adapter->getDirectProxy());
    }
    catch(AdapterNotActiveException&)
    {
	return "";
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw AdapterNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

StringSeq
AdminI::getAllAdapterIds(const Current&) const
{
    return _adapterRegistry->getAll();
}

void 
AdminI::addObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    ObjectDescriptor desc;
    desc.proxy = proxy;
    
    try
    {
	addObjectWithType(proxy, proxy->ice_id(), current);
    }
    catch(const Ice::LocalException&)
    {
	DeploymentException ex;
	ex.reason = "Couldn't invoke on the object to get its interface.";
	throw ex;
    }
}

void 
AdminI::updateObject(const Ice::ObjectPrx& proxy, const ::Ice::Current& current)
{
    _objectRegistry->update(proxy);
}

void 
AdminI::addObjectWithType(const Ice::ObjectPrx& proxy, const string& type, const ::Ice::Current&)
{
    ObjectDescriptor desc;
    desc.proxy = proxy;
    desc.type = type;
    _objectRegistry->add(desc);
}

void 
AdminI::removeObject(const Ice::Identity& id, const Ice::Current&)
{
    _objectRegistry->remove(id);
}

ObjectDescriptor
AdminI::getObjectDescriptor(const Ice::Identity& id, const Ice::Current&) const
{
    return _objectRegistry->getObjectDescriptor(id);
}

ObjectDescriptorSeq
AdminI::getAllObjectDescriptors(const string& expression, const Ice::Current&) const
{
    return _objectRegistry->findAll(expression);
}

bool
AdminI::pingNode(const string& name, const Current&) const
{
    NodePrx node = _nodeRegistry->findByName(name);
    try
    {
	node->ice_ping();
	return true;
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	return false;
    }
}

void
AdminI::shutdownNode(const string& name, const Current&)
{
    NodePrx node = _nodeRegistry->findByName(name);
    try
    {
	node->shutdown();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}

void
AdminI::removeNode(const string& name, const Current&)
{
    //
    // Remove the node servers.
    //
    ServerDescriptorSeq servers = _serverRegistry->getAllDescriptorsOnNode(name);
    for(ServerDescriptorSeq::const_iterator p = servers.begin(); p != servers.end(); ++p)
    {
	ServerCleaner(_nodeRegistry, 
		      _applicationRegistry, 
		      _serverRegistry, 
		      _adapterRegistry,
		      _objectRegistry,
		      _communicator->getLogger()).clean(*p);
    }

    //
    // Remove the node.
    //
    _nodeRegistry->remove(name);
}

string
AdminI::getNodeHostname(const string& name, const Current&) const
{
    NodePrx node = _nodeRegistry->findByName(name);
    try
    {
	return node->getHostname();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	throw NodeNotExistException();
    }
    catch(const Ice::LocalException&)
    {
	throw NodeUnreachableException();
    }
}


StringSeq
AdminI::getAllNodeNames(const Current&) const
{
    return _nodeRegistry->getAll();
}

void
AdminI::shutdown(const Current&)
{
    _registry->shutdown();
}

SliceChecksumDict
AdminI::getSliceChecksums(const Current&) const
{
    return sliceChecksums();
}
