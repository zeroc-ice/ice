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
#include <IcePack/ServiceDeployer.h>
#include <IcePack/ServerDeployer.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class ServiceDeployHandler : public ComponentDeployHandler
{
public:

    ServiceDeployHandler(ServiceDeployer&);

    virtual void startElement(const XMLCh *const name, AttributeList &attrs); 
    virtual void endElement(const XMLCh *const name);
    virtual void startDocument();

private:

    ServiceDeployer& _deployer;
};

}

IcePack::ServiceDeployHandler::ServiceDeployHandler(ServiceDeployer& deployer) :
    ComponentDeployHandler(deployer),
    _deployer(deployer)
{
}

void
IcePack::ServiceDeployHandler::startDocument()
{
}

void 
IcePack::ServiceDeployHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    ComponentDeployHandler::startElement(name, attrs);

    string str = toString(name);

    if(str == "service")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _deployer.overrideBaseDir(basedir);
	}

	string kind = getAttributeValue(attrs, "kind");
	if(kind == "standard")
	{
	    _deployer.setKind(ServiceDeployer::ServiceKindStandard);
	}
	else if(kind == "freeze")
	{
	    _deployer.setKind(ServiceDeployer::ServiceKindFreeze);
	    _deployer.setDBEnv(getAttributeValueWithDefault(attrs, "dbenv", ""));
	}

	_deployer.createConfigFile("/config/config_" + _deployer.substitute("${name}"));
	_deployer.setEntryPoint(getAttributeValue(attrs, "entry"));
    }
    else if(str == "adapter")
    {
	_deployer.getServerDeployer().addAdapter(getAttributeValue(attrs, "name"), 
						 getAttributeValueWithDefault(attrs, "endpoints", ""));
    }
}

void
IcePack::ServiceDeployHandler::endElement(const XMLCh *const name)
{
    string str = toString(name);

    ComponentDeployHandler::endElement(name);
}

IcePack::ServiceDeployer::ServiceDeployer(const Ice::CommunicatorPtr& communicator,
					  ServerDeployer& serverDeployer,
					  const map<string, string>& variables) :
    ComponentDeployer(communicator),
    _serverDeployer(serverDeployer)
{
    _variables = variables;
}

void
IcePack::ServiceDeployer::parse(const string& descriptor)
{
    ServiceDeployHandler handler(*this);
    
    ComponentDeployer::parse(descriptor, handler);
}

ServerDeployer&
IcePack::ServiceDeployer::getServerDeployer() const
{
    return _serverDeployer;
}

void
IcePack::ServiceDeployer::setKind(ServiceKind kind)
{
    _kind = kind;
}

void
IcePack::ServiceDeployer::setEntryPoint(const string& entry)
{
    assert(!_configFile.empty());
    _serverDeployer.addProperty("IceBox.Service." + _variables["name"], entry + " --Ice.Config=" + _configFile);
}

void
IcePack::ServiceDeployer::setDBEnv(const string& dir)
{
    if(_kind != ServiceKindFreeze)
    {
	throw DeploySAXParseException("Database environment is only allowed for Freeze services", _locator);
    }

    string path;

    if(dir.empty())
    {
	//
	// Provides database environment directory only if the
	// database environment attribute is not specified. If it's
	// specified, it's most likely because we share database
	// environments and then it's the responsabilility of the user
	// to manage the database environment directory.
	//
	createDirectory("/dbs/" + _variables["name"], true);
	path = _variables["datadir"] + "/dbs/" + _variables["name"];
    }
    else
    {
	path = dir[0] == '/' ? dir : _variables["basedir"] + "/" + dir;
    }
    _serverDeployer.addProperty("IceBox.DBEnvName." + _variables["name"], path);
}

