// **********************************************************************
//
// Copyright (c) 2003
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

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);

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
IcePack::ServiceHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    ComponentHandler::startElement(name, attrs, line, column);

    if(!isCurrentTargetDeployable())
    {
	return;
    }

    if(name == "service")
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
    else if(name == "adapter")
    {
	assert(!_currentAdapterId.empty());
	string adapterName = getAttributeValue(attrs, "name");
	_builder.getServerBuilder().registerAdapter(adapterName, getAttributeValue(attrs, "endpoints"),
                                                    _currentAdapterId);
    }
}

IcePack::ServiceBuilder::ServiceBuilder(const NodeInfoPtr& nodeInfo,
					ServerBuilder& serverBuilder,
					const map<string, string>& variables,
					const vector<string>& targets) :
    ComponentBuilder(nodeInfo->getCommunicator(), variables, targets),
    _nodeInfo(nodeInfo),
    _serverBuilder(serverBuilder)
{
    assert(_variables.back().find("parent") != _variables.back().end());
    assert(_variables.back().find("name") != _variables.back().end());
    assert(_variables.back().find("fqn") != _variables.back().end());
    assert(_variables.back().find("datadir") != _variables.back().end());

    //
    // Required for the component builder.
    //
    _objectRegistry = _nodeInfo->getObjectRegistry();
}

void
IcePack::ServiceBuilder::parse(const string& descriptor)
{
    ServiceHandler handler(*this);

    ComponentBuilder::parse(descriptor, handler);
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
    _serverBuilder.addProperty("IceBox.Service." + getVariable("name"), entry + " --Ice.Config=" + _configFile);
}

void
IcePack::ServiceBuilder::setDBEnv(const string& dir)
{
    if(_kind != ServiceKindFreeze)
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "database environment is only allowed for Freeze services");
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
	createDirectory("/dbs/" + getVariable("name"), true);
	path = getVariable("datadir") + "/dbs/" + getVariable("name");
    }
    else
    {
	path = toLocation(dir);
    }
    _serverBuilder.addProperty("IceBox.DBEnvName." + getVariable("name"), getVariable("name"));
    addProperty("Freeze.DbEnv." + getVariable("name") + ".DbHome", path);
    
}

//
// Compute an adapter id for a given adapter name.
//
string
IcePack::ServiceBuilder::getDefaultAdapterId(const string& name)
{
    //
    // Concatenate the server and service name to the adapter name.
    //
    return name + "-" + getVariable("parent") + "." + getVariable("name");
}
