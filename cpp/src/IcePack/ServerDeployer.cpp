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

    AddAdapterTask(const AdapterManagerPrx& manager, const ServerPtr& server, const AdapterDescription& desc) :
	_manager(manager),
	_server(server),
	_desc(desc)
    {
    }

    virtual void 
    deploy()
    {
	try
	{
	    AdapterPrx adapter = _manager->create(_desc);
	    
	    _server->_description.adapters.push_back(_desc.name);
	    _server->_adapters.push_back(adapter);
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

	// 
	// NOTE: We don't need to remove the adapter from the
	// server. The undo occured because a problem during the
	// server deployment so the server will be removed anyway.
	//
    }

private:

    AdapterManagerPrx _manager;
    ServerPtr _server;
    AdapterDescription _desc;
};

//
// Server deployer handler.
//
class ServerDeployHandler : public ComponentDeployHandler
{
public:

    ServerDeployHandler(ServerDeployer&);

    virtual void startElement(const XMLCh *const name, AttributeList &attrs); 
    virtual void endElement(const XMLCh *const name);
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
	string kind = getAttributeValue(attrs, "kind");
	if(kind == "cpp")
	{
	    _deployer.setKind(ServerDeployer::CppServer);
	}
	else if(kind == "java")
	{
	    _deployer.setKind(ServerDeployer::JavaServer);
	}
	else if(kind == "cpp-icebox")
	{
	    _deployer.setKind(ServerDeployer::CppIceBox);
	}
	else if(kind == "java-icebox")
	{
	    _deployer.setKind(ServerDeployer::JavaIceBox);
	}

	_deployer.createConfigFile("/config/config_server");
    }
    else if(str == "service")
    {
	string name = getAttributeValue(attrs, "name");
	string descriptor = getAttributeValue(attrs, "descriptor");
	_deployer.addService(name, descriptor);
    }
    else if(str == "adapter")
    {
	_deployer.addAdapter(getAttributeValue(attrs, "name"));
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
					const ServerPtr& server, 
					const ServerPrx& serverProxy) :
    ComponentDeployer(communicator),
    _server(server),
    _serverProxy(serverProxy)
{
    _variables["name"] = _server->_description.name;
    _variables["datadir"] += "/" + _variables["name"];
}

void
IcePack::ServerDeployer::setAdapterManager(const AdapterManagerPrx& manager)
{
    _adapterManager = manager;
}

void
IcePack::ServerDeployer::parse()
{
    ServerDeployHandler handler(*this);

    ComponentDeployer::parse(_server->_description.descriptor, handler);

    //
    // Once everything is parsed, we can perform some final setup
    // before the deployment starts.
    // 
    Ice::PropertiesPtr props = _communicator->getProperties();
    _properties->setProperty("Ice.ProgramName", _variables["name"]);
    _properties->setProperty("Ice.Default.Locator", props->getProperty("Ice.Default.Locator"));
    _properties->setProperty("Yellow.Query", props->getProperty("IcePack.Yellow.Query"));

    if(_kind == JavaServer)
    {
	if(!_server->_description.libraryPath.empty())
	{
	    _javaOptions.push_back("-classpath");
	    _javaOptions.push_back(_server->_description.libraryPath);
	    _javaOptions.push_back("-ea");
	}
	_javaOptions.push_back(_className);

	for(vector<string>::reverse_iterator p = _javaOptions.rbegin(); p != _javaOptions.rend(); ++p)
	{
	    _server->_description.args.insert(_server->_description.args.begin(), *p);
	}
    }

    _server->_description.args.push_back("--Ice.Config=" + _configFile);
}

void
IcePack::ServerDeployer::setClassName(const string& name)
{
    if(_kind != JavaServer)
    {
	cerr << "Class name element only allowed for Java servers." << endl;
	_error++;
	return;	
    }

    if(name == "")
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
    if(pwd == "")
    {
	cerr << "Empty working directory." << endl;
	_error++;
	return;
    }

    _server->_description.pwd = substitute(pwd);
}

void
IcePack::ServerDeployer::addAdapter(const string& name)
{
    if(!_adapterManager)
    {
	cerr << "Adapter manager not set, can't register the adapter '" << name << "'" << endl;
	_error++;
	return;	
    }

    AdapterDescription desc;
    desc.name = substitute(name);
    desc.server = _serverProxy;
    if(desc.name == "")
    {
	cerr << "Empty adapter name." << endl;
	_error++;
	return;
    }

    _tasks.push_back(new AddAdapterTask(_adapterManager, _server, desc));
}

void
IcePack::ServerDeployer::addService(const string& name, const string& descriptor)
{
    if(_kind != CppIceBox && _kind != JavaIceBox)
    {
	cerr << "Service elements are only allowed for Java or C++ IceBox servers." << endl;
	_error++;
	return;
    }

    std::map<std::string, std::string> variables = _variables;
    variables["name"] = name;

    ServiceDeployer* task = new ServiceDeployer(_communicator, *this, variables);
    try
    {
	task->parse(descriptor);
    }
    catch(const DeploymentException&)
    {
	cerr << "Failed to parse the service '" << name << "' descriptor" << endl;
	delete task;
	_error++;
    }
    
    _tasks.push_back(task);
}

void
IcePack::ServerDeployer::addOption(const string& option)
{
    _server->_description.args.push_back(substitute(option));
}

void
IcePack::ServerDeployer::addJavaOption(const string& option)
{
    _javaOptions.push_back(substitute(option));
}

void
IcePack::ServerDeployer::setKind(ServerDeployer::ServerKind kind)
{
    switch(kind)
    {
    case CppServer:
	if(_server->_description.path.empty())
	{
	    cerr << "C++ server path is not specified" << endl;
	    _error++;
	    break;
	}
	
    case JavaServer:	
	if(_server->_description.path.empty())
	{
	    _server->_description.path = "java";
	}
	break;

    case JavaIceBox:
	if(_server->_description.path.empty())
	{
	    _server->_description.path = "java";
	}
	addProperty("IceBox.Name", "${name}");
	addAdapter("${name}.ServiceManagerAdapter");
	break;

    case CppIceBox:
	if(_server->_description.path.empty())
	{
	    _server->_description.path = "icebox";
	}
	addProperty("IceBox.Name", "${name}");
	addAdapter("${name}.ServiceManagerAdapter");
	break;
    }
    
    _kind = kind;
}
