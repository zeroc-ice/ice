// **********************************************************************
//
// Copyright (c) 2003-2004 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IceUtil/UUID.h>
#include <Ice/Ice.h>
#include <IcePack/ServerBuilder.h>
#include <IcePack/ServiceBuilder.h>
#include <IcePack/AdapterFactory.h>
#include <IcePack/ServerFactory.h>

#include <iterator>

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

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);

private:

    ServerBuilder& _builder;
};

}

IcePack::ServerHandler::ServerHandler(ServerBuilder& builder) :
    ComponentHandler(builder),
    _builder(builder)
{
    //
    // Create top level directory and configuration directory.
    //
    _builder.createDirectory("");
    _builder.createDirectory("/config");
    _builder.registerServer();
}

void 
IcePack::ServerHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    ComponentHandler::startElement(name, attrs, line, column);
    if(!isCurrentTargetDeployable())
    {
	return;
    }

    if(name == "server")
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
				     getAttributeValue(attrs, "endpoints"), true,
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
				     getAttributeValue(attrs, "endpoints"), true,
				     _builder.getDefaultAdapterId("IceBox.ServiceManager"));
	}
    }
    else if(name == "service")
    {
	string serviceName = getAttributeValue(attrs, "name");
	string descriptor = getAttributeValue(attrs, "descriptor");
	string targets = getAttributeValueWithDefault(attrs, "targets", "");
	_builder.addService(serviceName, descriptor, targets);
    }
    else if(name == "adapter")
    {
	assert(!_currentAdapterId.empty());
	string adapterName = getAttributeValue(attrs, "name");
        bool registerProcess = getAttributeValueWithDefault(attrs, "register", "false") == "true";
	_builder.registerAdapter(adapterName, getAttributeValue(attrs, "endpoints"), registerProcess,
                                 _currentAdapterId);
    }
}

void
IcePack::ServerHandler::endElement(const string& name, int line, int column)
{
    if(isCurrentTargetDeployable())
    {
	if(name == "classname")
	{
	    _builder.setClassName(elementValue());
	}
	else if(name == "pwd")
	{
	    _builder.setWorkingDirectory(elementValue());
	}
	else if(name == "option")
	{
	    _builder.addOption(elementValue());
	}
	else if(name == "vm-option")
	{
	    _builder.addJavaOption(elementValue());
	}
	else if(name == "env")
	{
	    _builder.addEnvVar(elementValue());
	}
    }

    ComponentHandler::endElement(name, line, column);
}

IcePack::ServerBuilder::ServerBuilder(const NodeInfoPtr& nodeInfo,
				      const map<string, string>& variables,
				      const vector<string>& targets) :
    ComponentBuilder(nodeInfo->getCommunicator(), variables, targets),
    _nodeInfo(nodeInfo)
{
    assert(_variables.back().find("parent") != _variables.back().end());
    assert(_variables.back().find("name") != _variables.back().end());
    assert(_variables.back().find("fqn") != _variables.back().end());
    assert(_variables.back().find("datadir") != _variables.back().end());
    assert(_variables.back().find("binpath") != _variables.back().end());
    assert(_variables.back().find("libpath") != _variables.back().end());

    //
    // Required for the component builder.
    //
    _objectRegistry = _nodeInfo->getObjectRegistry();

    //
    // Begin to populate the server description.
    //
    _description.name = getVariable("name");
    _description.path = getVariable("binpath");
    _libraryPath = getVariable("libpath");
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
    _properties->setProperty("Ice.ProgramName", getVariable("name"));

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

    if(_kind == ServerKindCppIceBox || _kind == ServerKindJavaIceBox)
    {
	ostringstream os;
	copy(_serviceNames.begin(), _serviceNames.end(), ostream_iterator<string>(os," "));
	addProperty("IceBox.LoadOrder", os.str());
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
    catch(const Freeze::DatabaseException& lex)
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
	throw IceXML::ParserException(__FILE__, __LINE__, "classname element only allowed for Java servers");
    }

    if(name.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "empty classname element value");
    }

    _className = name;
}

void
IcePack::ServerBuilder::setWorkingDirectory(const string& pwd)
{
    if(pwd.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "no working directory");
    }

    _description.pwd = pwd;
}

void
IcePack::ServerBuilder::registerServer()
{ 
    _tasks.push_back(new RegisterServerTask(_nodeInfo->getServerRegistry(), getVariable("name"), *this));
}

void
IcePack::ServerBuilder::registerAdapter(const string& name, const string& endpoints, bool registerProcess,
                                        const string& adapterId)
{
    AdapterRegistryPrx adapterRegistry = _nodeInfo->getAdapterRegistry();
    if(!adapterRegistry)
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "IcePack is not configured to register adapters");
    }

    if(name.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "empty adapter name");
    }
    if(endpoints.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "empty adapter endpoints");
    }
    if(adapterId.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "empty adapter id");
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
    if(registerProcess)
    {
        addProperty(name + ".RegisterProcess", "1");
    }
}

void
IcePack::ServerBuilder::addService(const string& name, const string& descriptor, const string& additionalTargets)
{
    if(_kind != ServerKindCppIceBox && _kind !=  ServerKindJavaIceBox)
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "services are only allowed in IceBox servers");
    }

    if(name.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "name attribute value is empty");
    }
    if(descriptor.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "descriptor attribute value is empty");
    }

    //
    // Setup new variables for the service, overides the name value.
    //
    std::map<std::string, std::string> variables;
    vector< map< string, string> >::reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	variables.insert(p->begin(), p->end());
	++p;
    }

    variables["parent"] = getVariable("name");
    variables["name"] = name;
    variables["fqn"] = getVariable("fqn") + "." + name;

    vector<string> targets = toTargets(additionalTargets);
    copy(_targets.begin(), _targets.end(), back_inserter(targets));

    ServiceBuilder* task = new ServiceBuilder(_nodeInfo, *this, variables, targets);
    TaskPtr t = task;
    task->parse(toLocation(descriptor));
    _tasks.push_back(t);

    _serviceNames.push_back(name);
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
IcePack::ServerBuilder::addEnvVar(const string& env)
{
    _description.envs.push_back(env);
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
	    throw IceXML::ParserException(__FILE__, __LINE__, "C++ server path is not specified");
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
		getVariable("name") + "/ServiceManager @ IceBox.ServiceManager-" + getVariable("name")));

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
		getVariable("name") + "/ServiceManager @ IceBox.ServiceManager-" + getVariable("name")));

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
