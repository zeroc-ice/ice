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
	string kind = getAttributeValue(attrs, "kind");
	if(kind == "standard")
	{
	    _deployer.setKind(ServiceDeployer::Standard);
	}
	else if(kind == "freeze")
	{
	    _deployer.setKind(ServiceDeployer::Freeze);
	    _deployer.setDBEnv(getAttributeValueWithDefault(attrs, "dbenv", "${name}"));
	}

	_deployer.setEntryPoint(getAttributeValueWithDefault(attrs, "library", "${name}"),
				getAttributeValueWithDefault(attrs, "entry", "create"));

	_deployer.createConfigFile("/config/config_${name}");
    }
    else if(str == "adapter")
    {
	_deployer.getServerDeployer().addAdapter(getAttributeValue(attrs, "name"));
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
IcePack::ServiceDeployer::setEntryPoint(const string& library, const string& entry)
{
    assert(!_configFile.empty());
    _serverDeployer.addProperty("IceBox.Service.${name}", library + ":" + entry + " --Ice.Config=" + _configFile);
}

void
IcePack::ServiceDeployer::setDBEnv(const string& dir)
{
    assert(!dir.empty());

    if(_kind != Freeze)
    {
	cerr << "Dabase environment is only allowed for Freeze services." << endl;
	_error++;
	return;
    }

    createDirectory("/dbs" + (dir[0] == '/' ? dir : "/" + dir));
    addProperty("IceBox.DBEnvName.${name}", "${datadir}/dbs" + (dir[0] == '/' ? dir : "/" + dir));
}

