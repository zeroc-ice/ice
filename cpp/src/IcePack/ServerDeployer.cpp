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
	catch(const AdapterExistsException&)
	{
	    throw DeploymentException();
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

    _serverManager->create(_description);
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
	cerr << "Class name element only allowed for Java servers." << endl;
	_error++;
	return;	
    }

    if(name.empty())
    {
	cerr << "Empty path." << endl;
	_error++;
	return;
    }

    _className = name;
}

void
IcePack::ServerDeployer::setWorkingDirectory(const string& pwd)
{
    if(pwd.empty())
    {
	cerr << "Empty working directory." << endl;
	_error++;
	return;
    }

    _description.pwd = pwd;
}

void
IcePack::ServerDeployer::addAdapter(const string& name, const string& endpoints)
{
    if(!_adapterManager)
    {
	cerr << "Adapter manager not set, can't register the adapter '" << name << "'" << endl;
	_error++;
	return;	
    }

    AdapterDescription desc;
    desc.name = name;
    desc.server = ServerPrx::uncheckedCast(
	_communicator->stringToProxy("server/" + _description.name + "@IcePack.Internal"));
    if(desc.name == "")
    {
	cerr << "Empty adapter name." << endl;
	_error++;
	return;
    }

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
	cerr << "Service elements are only allowed for Java or C++ IceBox servers." << endl;
	_error++;
	return;
    }

    if(name.empty() || descriptor.empty())
    {
	cerr << "Name or descriptor attribute value is empty in service element." << endl;
	_error++;
	return;
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
    catch(const DeploymentException&)
    {
	cerr << "Failed to parse the service '" << name << "' descriptor" << endl;
	delete task;
	_error++;
	return;
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
	    cerr << "C++ server path is not specified" << endl;
	    _error++;
	    break;
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
