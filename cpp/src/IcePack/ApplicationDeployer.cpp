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
#include <IcePack/ApplicationDeployer.h>
#include <IcePack/ServerDeployer.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class AddServer : public Task
{
public:
    
    AddServer(const AdminPtr& admin, const string& name, const string& descriptor, const string& binpath, 
	      const string& libpath, const Targets& targets) :
	_admin(admin),
	_name(name),
	_descriptor(descriptor),
	_binpath(binpath),
	_libpath(libpath),
	_targets(targets)
    {
    }
    
    virtual void
    deploy()
    {
	_admin->addServer(_name, _binpath, _libpath, _descriptor, _targets);
    }

    virtual void
    undeploy()
    {
	try
	{
	    _admin->removeServer(_name);
	}
	catch(const ServerNotExistException&)
	{
	}
    }

private:

    AdminPtr _admin;
    string _name;
    string _descriptor;
    string _binpath;
    string _libpath;
    Targets _targets;
};

class ApplicationDeployHandler : public ComponentDeployHandler
{
public:

    ApplicationDeployHandler(ApplicationDeployer&);

    virtual void startElement(const XMLCh *const name, AttributeList &attrs); 

private:

    ApplicationDeployer& _deployer;
};

}

IcePack::ApplicationDeployHandler::ApplicationDeployHandler(ApplicationDeployer& deployer) :
    ComponentDeployHandler(deployer),
    _deployer(deployer)
{
}

void 
IcePack::ApplicationDeployHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    ComponentDeployHandler::startElement(name, attrs);
    if(!isCurrentTargetDeployable())
    {
	return;
    }

    string str = toString(name);
    if(str == "server")
    {
	string name = getAttributeValue(attrs, "name");
	string descriptor = getAttributeValue(attrs, "descriptor");
	string binpath = getAttributeValueWithDefault(attrs, "binpath", "");
	string libpath = getAttributeValueWithDefault(attrs, "libpath", "");
	_deployer.addServer(name, descriptor, binpath, libpath);
    }
}

IcePack::ApplicationDeployer::ApplicationDeployer(const Ice::CommunicatorPtr& communicator,
						  const AdminPtr& admin,
						  const vector<string>& targets) :
    ComponentDeployer(communicator, "", targets),
    _admin(admin)
{
}

void
IcePack::ApplicationDeployer::parse(const string& descriptor)
{
    ApplicationDeployHandler handler(*this);    

    ComponentDeployer::parse(descriptor, handler);
}

void
IcePack::ApplicationDeployer::addServer(const string& name, 
					const string& descriptor, 
					const string& binpath,
					const string& libpath)
{
    if(name.empty())
    {
	throw DeploySAXParseException("name attribute value is empty", _locator);
    }
    if(descriptor.empty())
    {
	throw DeploySAXParseException("descriptor attribute value is empty", _locator);
    }

    _tasks.push_back(new AddServer(_admin, name, descriptor, binpath, libpath, _targets));
}

