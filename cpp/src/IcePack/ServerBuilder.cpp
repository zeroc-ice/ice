// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <IcePack/ServerBuilder.h>
#include <IcePack/ServiceBuilder.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/ServerFactory.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

//
// Server registration task.
//
class RegisterServerTask : public Task
{
public:

    RegisterServerTask(const ServerRegistryPrx& registry, const string& name, const ServerBuilder& builder) :
	_registry(registry),
	_name(name),
	_builder(builder)
    {
    }

    virtual void 
    execute()
    {
	try
	{
	    _registry->add(_name, _builder.getServer());
	}
	catch(const ServerExistsException& lex)
	{
	    ostringstream os;
	    os << "couldn't add the server to the registry:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the server registry:\n" << lex << endl;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    throw ex;
	}
    }

    virtual void 
    undo()
    {
	try
	{
	    _registry->remove(_name);
	}
	catch(const ServerNotExistException& lex)
	{
	    ostringstream os;
	    os << "couldn't remove the server from the registry:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the server registry:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    throw ex;
	}
    }

private:

    ServerRegistryPrx _registry;
    string _name;
    const ServerBuilder& _builder;
};

//
// Adapter registration task.
//
class RegisterServerAdapterTask : public Task
{
public:

    RegisterServerAdapterTask(const AdapterRegistryPrx& registry, const string& id, const ServerBuilder& builder) :
	_registry(registry),
	_id(id),
	_builder(builder)
    {
    }

    virtual void 
    execute()
    {
	try
	{
	    _registry->add(_id, _builder.getServerAdapter(_id));
	}
	catch(const AdapterExistsException& lex)
	{
	    ostringstream os;
	    os << "couldn't add the adapter to the registry:\n" << lex;

	    AdapterDeploymentException ex;
	    ex.id = _id;
	    ex.reason = os.str();
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the adapter registry: " << lex << endl;

	    AdapterDeploymentException ex;
	    ex.id = _id;
	    ex.reason = os.str();
	    throw ex;
	}
    }

    virtual void 
    undo()
    {
	try
	{
	    _registry->remove(_id);
	}
	catch(const AdapterNotExistException& lex)
	{
	    ostringstream os;
	    os << "couldn't remove the adapter from the registry:\n" << lex;

	    AdapterDeploymentException ex;
	    ex.id = _id;
	    ex.reason = os.str();
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the adapter registry:\n" << lex;

	    AdapterDeploymentException ex;
	    ex.id = _id;
	    ex.reason = os.str();
	    throw ex;
	}
    }

private:

    AdapterRegistryPrx _registry;
    string _id;
    const ServerBuilder& _builder;
};

//
// Server builder handler.
//
class ServerHandler : public ComponentHandler
{
public:

    ServerHandler(ServerBuilder&);

    virtual void startElement(const XMLCh *const, AttributeList &);
    virtual void endElement(const XMLCh *const);
    virtual void startDocument();

private:

    ServerBuilder& _builder;
};

}

IcePack::ServerHandler::ServerHandler(ServerBuilder& builder) :
    ComponentHandler(builder),
    _builder(builder)
{
}

void
IcePack::ServerHandler::startDocument()
{
    //
    // Create top level directory and configuration directory.
    //
    _builder.createDirectory("");
    _builder.createDirectory("/config");
    _builder.registerServer();
}

void 
IcePack::ServerHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    ComponentHandler::startElement(name, attrs);
    if(!isCurrentTargetDeployable())
    {
	return;
    }

    string str = toString(name);

    if(str == "server")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _builder.overrideBaseDir(basedir);
	}

	string kind = getAttributeValue(attrs, "kind");
	if(kind == "cpp")
	{
	    _builder.setKind(ServerBuilder::ServerKindCppServer);
	    _builder.createConfigFile("/config/config_server");
	}
	else if(kind == "java")
	{
	    _builder.setKind(ServerBuilder::ServerKindJavaServer);
	    _builder.createConfigFile("/config/config_server");
	}
	else if(kind == "cpp-icebox")
	{
	    _builder.setKind(ServerBuilder::ServerKindCppIceBox);
	    _builder.createConfigFile("/config/config_icebox");
	    _builder.createDirectory("/dbs");

	    //
	    // TODO: is the server name a good category?
	    //
	    _builder.addProperty("IceBox.ServiceManager.Identity", _builder.substitute("${name}/ServiceManager"));
	    
	    _builder.registerAdapter("IceBox.ServiceManager", 
				     getAttributeValue(attrs, "endpoints"),
				     _builder.getDefaultAdapterId("IceBox.ServiceManager"));
	}
	else if(kind == "java-icebox")
	{
	    _builder.setKind(ServerBuilder::ServerKindJavaIceBox);
	    _builder.createConfigFile("/config/config_icebox");
	    _builder.createDirectory("/dbs");

	    //
	    // TODO: is the server name a good category?
	    //
	    _builder.addProperty("IceBox.ServiceManager.Identity", _builder.substitute("${name}/ServiceManager"));

	    _builder.registerAdapter("IceBox.ServiceManager", 
				     getAttributeValue(attrs, "endpoints"),
				     _builder.getDefaultAdapterId("IceBox.ServiceManager"));
	}
    }
    else if(str == "service")
    {
	string name = getAttributeValue(attrs, "name");
	string descriptor = getAttributeValue(attrs, "descriptor");
	string targets = getAttributeValueWithDefault(attrs, "targets", "");
	_builder.addService(name, descriptor, targets);
    }
    else if(str == "adapter")
    {
	assert(!_currentAdapterId.empty());
	string name = getAttributeValue(attrs, "name");
	_builder.registerAdapter(name, getAttributeValue(attrs, "endpoints"), _currentAdapterId);
    }
}

void
IcePack::ServerHandler::endElement(const XMLCh *const name)
{
    string str = toString(name);

    if(isCurrentTargetDeployable())
    {
	if(str == "classname")
	{
	    _builder.setClassName(elementValue());
	}
	else if(str == "pwd")
	{
	    _builder.setWorkingDirectory(elementValue());
	}
    }

    ComponentHandler::endElement(name);
}

IcePack::ServerBuilder::ServerBuilder(const NodeInfoPtr& nodeInfo,
				      const map<string, string>& variables,
				      const vector<string>& targets) :
    ComponentBuilder(nodeInfo->getCommunicator(), variables, targets),
    _nodeInfo(nodeInfo)
{
    assert(_variables.find("parent") != _variables.end());
    assert(_variables.find("name") != _variables.end());
    assert(_variables.find("fqn") != _variables.end());
    assert(_variables.find("datadir") != _variables.end());
    assert(_variables.find("binpath") != _variables.end());
    assert(_variables.find("libpath") != _variables.end());

    //
    // Required for the component builder.
    //
    _yellowAdmin = _nodeInfo->getYellowAdmin();

    //
    // Begin to populate the server description.
    //
    _description.name = _variables["name"];
    _description.path = _variables["binpath"];
    _libraryPath = _variables["libpath"];
    _description.node = nodeInfo->getNode()->getName();
    _description.targets = targets;
}

void
IcePack::ServerBuilder::parse(const std::string& descriptor)
{
    ServerHandler handler(*this);

    //
    // Parse the deployment descriptor.
    //
    ComponentBuilder::parse(descriptor, handler);

    //
    // Once everything is parsed, we can perform some final setup
    // before the deployment starts.
    // 
    _description.descriptor = descriptor;

    Ice::PropertiesPtr props = _nodeInfo->getCommunicator()->getProperties();
    _properties->setProperty("Ice.ProgramName", _variables["name"]);

    //
    // TODO: Shall we really generate yellow configuration here?
    //
    _properties->setProperty("Yellow.Query", 
			     _nodeInfo->getCommunicator()->proxyToString(_nodeInfo->getYellowQuery()));

    if(_kind == ServerKindJavaServer || _kind == ServerKindJavaIceBox)
    {
	if(!_libraryPath.empty())
	{
	    _javaOptions.push_back("-classpath");
	    _javaOptions.push_back(_libraryPath);
	}
	_javaOptions.push_back("-ea");
	_javaOptions.push_back(_className);

	for(vector<string>::reverse_iterator p = _javaOptions.rbegin(); p != _javaOptions.rend(); ++p)
	{
	    _description.args.insert(_description.args.begin(), *p);
	}
    }

    _description.args.push_back("--Ice.Config=" + _configFile);
}

void
IcePack::ServerBuilder::execute()
{
    //
    // Creates the server from _description and the server adapters
    // listed in _serverAdapterIds. The adapter proxies are returned
    // in _serverAdapters.
    //
    try
    {
	_server = _nodeInfo->getServerFactory()->createServerAndAdapters(_description,
									 _serverAdapterIds,
									 _serverAdapters);
    }
    catch(const Freeze::DBException& lex)
    {
	ostringstream os;
	os << "couldn't create the server: " << lex.message << ":" << endl;
	os << lex;
	
	ServerDeploymentException ex;
	ex.server = _description.name;
	ex.reason = os.str();
	throw ex;	    
    }
    
    //
    // Deploy the server components. If something goes wrong it's our
    // responsability to destroy the server object. Note: destroying
    // the server object will also destroy the server adapters.
    //
    try
    {
	ComponentBuilder::execute();
    }
    catch(const DeploymentException& ex)
    {
	_server->destroy();
	throw ex;
    }
}

void
IcePack::ServerBuilder::undo()
{ 
    if(!_server)
    {
	ServerRegistryPrx registry = _nodeInfo->getServerRegistry();
	if(registry)
	{
	    try
	    {
		_server = registry->findByName(_description.name);
	    }
	    catch(const ServerNotExistException&)
	    {
		ostringstream os;
		os << "couldn't find server in the registry";

		ServerDeploymentException ex;
		ex.server = _description.name;
		ex.reason = os.str();
		throw ex;
	    }
	    catch(const Ice::LocalException& lex)
	    {
		ostringstream os;
		os << "couldn't contact the server registry:\n" << lex;
		
		ServerDeploymentException ex;
		ex.server = _description.name;
		ex.reason = os.str();
		throw ex;
	    }
	}
    }

    //
    // Set the server activation mode to manual (to avoid it to be
    // restarted on demand) and stop it before removing its
    // components.
    //
    _server->setActivationMode(Manual);
    _server->stop();

    //
    // Remove all the server components.
    //
    ComponentBuilder::undo();
    
    try
    {	
	_server->destroy();
    }
    catch(const Ice::LocalException& lex)
    {
        ostringstream os;
        os << "couldn't destroy the server:\n" << lex;
      
        ServerDeploymentException ex;
        ex.server = _description.name;
        ex.reason = os.str();
        throw ex;
    }
}

void
IcePack::ServerBuilder::setClassName(const string& name)
{
    if(_kind != ServerKindJavaServer)
    {
	throw DeploySAXParseException("classname element only allowed for Java servers", _locator);
    }

    if(name.empty())
    {
	throw DeploySAXParseException("empty classname element value", _locator);
    }

    _className = name;
}

void
IcePack::ServerBuilder::setWorkingDirectory(const string& pwd)
{
    if(pwd.empty())
    {
	throw DeploySAXParseException("no working directory", _locator);
    }

    _description.pwd = pwd;
}

void
IcePack::ServerBuilder::registerServer()
{ 
    _tasks.push_back(new RegisterServerTask(_nodeInfo->getServerRegistry(), _variables["name"], *this));
}

void
IcePack::ServerBuilder::registerAdapter(const string& name, const string& endpoints, const string& adapterId)
{
    AdapterRegistryPrx adapterRegistry = _nodeInfo->getAdapterRegistry();
    if(!adapterRegistry)
    {
	throw DeploySAXParseException("IcePack is not configured to register adapters", _locator);
    }

    if(name.empty())
    {
	throw DeploySAXParseException("empty adapter name", _locator);
    }
    if(endpoints.empty())
    {
	throw DeploySAXParseException("empty adapter endpoints", _locator);
    }
    if(adapterId.empty())
    {
	throw DeploySAXParseException("empty adapter id", _locator);
    }

    //
    // A server adapter object will be created with the server when
    // the server is created (see ServerBuilder::execute()
    // method). The RegisterServerAdapter task will get the server
    // adapter proxy through the builder method getServerAdapter().
    //
    _serverAdapterIds.push_back(adapterId);
    _tasks.push_back(new RegisterServerAdapterTask(adapterRegistry, adapterId, *this));

    //
    // Generate adapter configuration properties.
    //
    addProperty(name + ".Endpoints", endpoints);
    addProperty(name + ".AdapterId", adapterId);
}

void
IcePack::ServerBuilder::addService(const string& name, const string& descriptor, const string& additionalTargets)
{
    if(_kind != ServerKindCppIceBox && _kind !=  ServerKindJavaIceBox)
    {
	throw DeploySAXParseException("services are only allowed in IceBox servers", _locator);
    }

    if(name.empty())
    {
	throw DeploySAXParseException("name attribute value is empty", _locator);
    }
    if(descriptor.empty())
    {
	throw DeploySAXParseException("descriptor attribute value is empty", _locator);
    }

    //
    // Setup new variables for the service, overides the name value.
    //
    std::map<std::string, std::string> variables = _variables;
    variables["parent"] = _variables["name"];
    variables["name"] = name;
    variables["fqn"] = _variables["fqn"] + "." + name;

    vector<string> targets = toTargets(additionalTargets);
    copy(_targets.begin(), _targets.end(), back_inserter(targets));

    ServiceBuilder* task = new ServiceBuilder(_nodeInfo, *this, variables, targets);
    try
    {
	task->parse(toLocation(descriptor));
    }
    catch(const ParserDeploymentException& ex)
    {
	throw ParserDeploymentWrapperException(ex);
    }
    
    _tasks.push_back(task);
}

void
IcePack::ServerBuilder::addOption(const string& option)
{
    _description.args.push_back(option);
}

void
IcePack::ServerBuilder::addJavaOption(const string& option)
{
    _javaOptions.push_back(option);
}

void
IcePack::ServerBuilder::setKind(ServerBuilder::ServerKind kind)
{
    switch(kind)
    {
    case ServerKindCppServer:
    {
	if(_description.path.empty())
	{
	    throw DeploySAXParseException("C++ server path is not specified", _locator);
	}
	break;
    }
    case ServerKindJavaServer:
    {
	if(_description.path.empty())
	{
	    _description.path = "java";
	}
	break;
    }
    case ServerKindJavaIceBox:
    {
	if(_description.path.empty())
	{
	    _description.path = "java";
	}
	
	_description.serviceManager = IceBox::ServiceManagerPrx::uncheckedCast(
	    _nodeInfo->getCommunicator()->stringToProxy(
		_variables["name"] + "/ServiceManager @ IceBox.ServiceManager-" + _variables["name"]));

	_className = "IceBox.Server";

	break;
    }
    case ServerKindCppIceBox:
    {
	if(_description.path.empty())
	{
	    _description.path = "icebox";
	}

	_description.serviceManager = IceBox::ServiceManagerPrx::uncheckedCast(
	    _nodeInfo->getCommunicator()->stringToProxy(
		_variables["name"] + "/ServiceManager @ IceBox.ServiceManager-" + _variables["name"]));

	break;
    }
    }
    
    _kind = kind;
}

ServerPrx
IcePack::ServerBuilder::getServer() const
{
    assert(_server);
    return _server;
}

ServerAdapterPrx
IcePack::ServerBuilder::getServerAdapter(const std::string& id) const
{
    map<string, ServerAdapterPrx>::const_iterator p = _serverAdapters.find(id);
    if(p != _serverAdapters.end())
    {
	return p->second;
    }
    else 
    {
	assert(false);
	return 0; // Keep the compiler happy.
    }
}
