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
#include <IcePack/ServiceBuilder.h>
#include <IcePack/ServerBuilder.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class ServiceHandler : public ComponentHandler
{
public:

    ServiceHandler(ServiceBuilder&);

    virtual void startElement(const XMLCh *const name, AttributeList &attrs); 

private:

    ServiceBuilder& _builder;
};

}

IcePack::ServiceHandler::ServiceHandler(ServiceBuilder& builder) :
    ComponentHandler(builder),
    _builder(builder)
{
}

void 
IcePack::ServiceHandler::startElement(const XMLCh *const name, AttributeList &attrs)
{
    ComponentHandler::startElement(name, attrs);

    if(!isCurrentTargetDeployable())
    {
	return;
    }

    string str = toString(name);

    if(str == "service")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _builder.overrideBaseDir(basedir);
	}

	string kind = getAttributeValue(attrs, "kind");
	if(kind == "standard")
	{
	    _builder.setKind(ServiceBuilder::ServiceKindStandard);
	}
	else if(kind == "freeze")
	{
	    _builder.setKind(ServiceBuilder::ServiceKindFreeze);
	    _builder.setDBEnv(getAttributeValueWithDefault(attrs, "dbenv", ""));
	}
	
	_builder.createConfigFile(_builder.substitute("/config/config_${name}"));
	_builder.setEntryPoint(getAttributeValue(attrs, "entry"));
    }
    else if(str == "adapter")
    {
	string name = getAttributeValue(attrs, "name");
	string id = getAttributeValueWithDefault(attrs, "id", "");

	//
	// If the adapter id is not specified or empty, generate one
	// from the server, service and adapter name: <adapter
	// name>-<server name>-<service name>
	//
	if(id.empty())
	{
	    id = name + "-" + _builder.getServerBuilder().substitute("${name}") + _builder.substitute("${name}");
	}
	_builder.getServerBuilder().registerAdapter(name, getAttributeValue(attrs, "endpoints"), id);
    }
}

IcePack::ServiceBuilder::ServiceBuilder(const NodeInfoPtr& nodeInfo,
					ServerBuilder& serverBuilder,
					const map<string, string>& variables,
					const string& componentPath,
					const vector<string>& targets) :
    ComponentBuilder(nodeInfo->getCommunicator(), componentPath, targets),
    _nodeInfo(nodeInfo),
    _serverBuilder(serverBuilder)
{
    _yellowAdmin = nodeInfo->getYellowAdmin();

    _variables = variables;
}

void
IcePack::ServiceBuilder::parse(const string& descriptor)
{
    ServiceHandler handler(*this);
    
    ComponentBuilder::parse(descriptor, handler);

    //
    // Once everything is parsed, we can perform some final setup
    // before the deployment starts.
    // 
    _properties->setProperty("Yellow.Query", 
			     _nodeInfo->getCommunicator()->proxyToString(_nodeInfo->getYellowQuery()));
}

ServerBuilder&
IcePack::ServiceBuilder::getServerBuilder() const
{
    return _serverBuilder;
}

void
IcePack::ServiceBuilder::setKind(ServiceKind kind)
{
    _kind = kind;
}

void
IcePack::ServiceBuilder::setEntryPoint(const string& entry)
{
    assert(!_configFile.empty());
    _serverBuilder.addProperty("IceBox.Service." + _variables["name"], entry + " --Ice.Config=" + _configFile);
}

void
IcePack::ServiceBuilder::setDBEnv(const string& dir)
{
    if(_kind != ServiceKindFreeze)
    {
	throw DeploySAXParseException("database environment is only allowed for Freeze services", _locator);
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
	path = toLocation(dir);
    }
    _serverBuilder.addProperty("IceBox.DBEnvName." + _variables["name"], path);
}

