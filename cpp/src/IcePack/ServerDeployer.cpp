// **********************************************************************
//
// Copyright (c) 2001
// Mutable Realms, Inc.
// Huntsville, AL, USA
//
// All Rights Reserved
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ServerDeployer.h>
#include <IcePack/ServiceDeployer.h>
#include <IcePack/AdapterManager.h>
#include <IcePack/ServerManager.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

//
// Adapter registration task.
//
class AddAdapterTask : public Task
{
public:

    AddAdapterTask(const AdapterManagerPrx& manager, const AdapterDescription& desc) :
	_manager(manager),
	_desc(desc)
    {
    }

    virtual void 
    deploy()
    {
	try
	{
	    AdapterPrx adapter = _manager->create(_desc);    
	}
	catch(const AdapterExistsException& ex)
	{
	    AdapterDeploymentException ex;
	    ex.adapter = _desc.name;
	    ex.reason = "adapter already exist";
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the adpater manager: " << lex << endl;

	    AdapterDeploymentException ex;
	    ex.adapter = _desc.name;
	    ex.reason = os.str();
	    throw ex;
	}
    }

    virtual void 
    undeploy()
    {
	_manager->remove(_desc.name);
    }

private:

    AdapterManagerPrx _manager;
    AdapterDescription _desc;
};

//
// Server deployer handler.
//
class ServerDeployHandler : public ComponentDeployHandler
{
public:

    ServerDeployHandler(ServerDeployer&);

    virtual void startElement(const XMLCh *const, AttributeList &);
    virtual void endElement(const XMLCh *const);
    virtual void startDocument();

private:

    ServerDeployer& _deployer;
};

}

IcePack::ServerDeployHandler::ServerDeployHandler(ServerDeployer& deployer) :
    ComponentDeployHandler(deployer),
    _deployer(deployer)
{
}

void
IcePack::ServerDeployHandler::startDocument()
{
    //
    // Create top level directory and configuration directory.
    //
    _deployer.createDirectory("");
    _deployer.createDirectory("/config");
}

void 
IcePack::ServerDeployHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    ComponentDeployHandler::startElement(name, attrs);

    string str = toString(name);

    if(str == "server")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _deployer.overrideBaseDir(basedir);
	}

	string kind = getAttributeValue(attrs, "kind");
	if(kind == "cpp")
	{
	    _deployer.setKind(ServerDeployer::ServerKindCppServer);
	    _deployer.createConfigFile("/config/config_server");
	}
	else if(kind == "java")
	{
	    _deployer.setKind(ServerDeployer::ServerKindJavaServer);
	    _deployer.createConfigFile("/config/config_server");
	}
	else if(kind == "cpp-icebox")
	{
	    _deployer.setKind(ServerDeployer::ServerKindCppIceBox);
	    _deployer.addProperty("IceBox.ServiceManager.Endpoints", getAttributeValue(attrs, "endpoints"));
	    _deployer.createConfigFile("/config/config_icebox");
	}
	else if(kind == "java-icebox")
	{
	    _deployer.setKind(ServerDeployer::ServerKindJavaIceBox);
	    _deployer.addProperty("IceBox.ServiceManager.Endpoints", getAttributeValue(attrs, "endpoints"));
	    _deployer.createConfigFile("/config/config_icebox");
	}
    }
    else if(str == "service")
    {
	string name = getAttributeValue(attrs, "name");
	string descriptor = getAttributeValue(attrs, "descriptor");
	_deployer.addService(name, descriptor);
    }
    else if(str == "adapter")
    {
	_deployer.addAdapter(getAttributeValue(attrs, "name"), getAttributeValueWithDefault(attrs, "endpoints", ""));
    }
}

void
IcePack::ServerDeployHandler::endElement(const XMLCh *const name)
{
    string str = toString(name);

    if(str == "classname")
    {
	_deployer.setClassName(elementValue());
    }
    else if(str == "pwd")
    {
	_deployer.setWorkingDirectory(elementValue());
    }

    ComponentDeployHandler::endElement(name);
}

IcePack::ServerDeployer::ServerDeployer(const Ice::CommunicatorPtr& communicator,
					const string& name,
					const string& path,
					const string& libraryPath) :
    ComponentDeployer(communicator),
    _libraryPath(libraryPath)
{
    _variables["name"] = name;
    _variables["datadir"] += "/" + _variables["name"];

    _description.name = name;
    _description.path = path;
}

void
IcePack::ServerDeployer::setAdapterManager(const AdapterManagerPrx& manager)
{
    _adapterManager = manager;
}

void
IcePack::ServerDeployer::setServerManager(const ServerManagerPrx& manager)
{
    _serverManager = manager;
}

void
IcePack::ServerDeployer::parse(const std::string& descriptor)
{
    ServerDeployHandler handler(*this);

    //
    // Parse the deployment descriptor.
    //
    ComponentDeployer::parse(descriptor, handler);

    //
    // Once everything is parsed, we can perform some final setup
    // before the deployment starts.
    // 
    _description.descriptor = descriptor;

    Ice::PropertiesPtr props = _communicator->getProperties();
    _properties->setProperty("Ice.ProgramName", _variables["name"]);
    _properties->setProperty("Ice.Default.Locator", props->getProperty("Ice.Default.Locator"));
    _properties->setProperty("Yellow.Query", props->getProperty("IcePack.Yellow.Query"));

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
IcePack::ServerDeployer::deploy()
{
    ComponentDeployer::deploy();

    try
    {
	_serverManager->create(_description);
    }
    catch(const ServerExistsException& ex)
    {
	ServerDeploymentException ex1;
	ex1.server = _variables["name"];
	ex1.reason = "Server already exists";
	throw ex1;
    }
}

void
IcePack::ServerDeployer::undeploy()
{ 
    _serverManager->remove(_description.name);

    ComponentDeployer::undeploy();
}

void
IcePack::ServerDeployer::setClassName(const string& name)
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
IcePack::ServerDeployer::setWorkingDirectory(const string& pwd)
{
    if(pwd.empty())
    {
	throw DeploySAXParseException("no working directory", _locator);
    }

    _description.pwd = pwd;
}

void
IcePack::ServerDeployer::addAdapter(const string& name, const string& endpoints)
{
    if(!_adapterManager)
    {
	throw DeploySAXParseException("IcePack is not configured to deploy adapters", _locator);
    }

    if(name.empty())
    {
	throw DeploySAXParseException("no adapter name", _locator);
    }

    AdapterDescription desc;
    desc.name = name;
    desc.server = ServerPrx::uncheckedCast(
	_communicator->stringToProxy("server/" + _description.name + "@IcePack.Internal"));

    _tasks.push_back(new AddAdapterTask(_adapterManager, desc));

    _description.adapters.push_back(desc.name);
    
    if(!endpoints.empty())
    {
	addProperty("Ice.Adapter." + name + ".Endpoints", endpoints);
    }
}

void
IcePack::ServerDeployer::addService(const string& name, const string& descriptor)
{
    if(_kind != ServerKindCppIceBox && _kind !=  ServerKindJavaIceBox)
    {
	// TODO: ML: Grammar. Should this be "allowed"?
	throw DeploySAXParseException("services are only allows in IceBox servers", _locator);
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
    variables["name"] = name;

    ServiceDeployer* task = new ServiceDeployer(_communicator, *this, variables);
    try
    {
	string xmlFile = descriptor[0] != '/' ? _variables["basedir"] + "/" + descriptor : descriptor;
	task->parse(xmlFile);
    }
    catch(const ParserDeploymentException& ex)
    {
	//
	// Add component and wrap the exception in a
	// ParserDeploymentWrapperException.
	//
	ParserDeploymentException ex1(ex);
	ex1.component = _variables["name"] + ":" + ex.component;
	throw ParserDeploymentWrapperException(ex1);
    }
    
    _tasks.push_back(task);
}

void
IcePack::ServerDeployer::addOption(const string& option)
{
    _description.args.push_back(option);
}

void
IcePack::ServerDeployer::addJavaOption(const string& option)
{
    _javaOptions.push_back(option);
}

void
IcePack::ServerDeployer::setKind(ServerDeployer::ServerKind kind)
{
    switch(kind)
    {
    case ServerKindCppServer:
	if(_description.path.empty())
	{
	    throw DeploySAXParseException("C++ server path is not specified", _locator);
	}
	
    case ServerKindJavaServer:	
	if(_description.path.empty())
	{
	    _description.path = "java";
	}
	break;

    case ServerKindJavaIceBox:
	if(_description.path.empty())
	{
	    _description.path = "java";
	}
	_className = "IceBox.Server";
	createDirectory("/dbs");
	addProperty("IceBox.Name", _variables["name"]);
	addAdapter(_variables["name"] + ".ServiceManagerAdapter","");
	break;

    case ServerKindCppIceBox:
	if(_description.path.empty())
	{
	    _description.path = "icebox";
	}
	createDirectory("/dbs");
	addProperty("IceBox.Name", _variables["name"]);
	addAdapter(_variables["name"] + ".ServiceManagerAdapter","");
	break;
    }
    
    _kind = kind;
}
