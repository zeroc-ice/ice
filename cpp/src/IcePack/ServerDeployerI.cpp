// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePack/ServerDeployerI.h>
#include <IcePack/ServerFactory.h>
#include <IcePack/ServerBuilder.h>
#include <IcePack/TraceLevels.h>

using namespace std;
using namespace IcePack;

IcePack::ServerDeployerI::ServerDeployerI(const NodeInfoPtr& nodeInfo) :
    _nodeInfo(nodeInfo)
{
}

IcePack::ServerDeployerI::~ServerDeployerI()
{
}

void
IcePack::ServerDeployerI::add(const string& name, const string& descriptor, const string& binPath, 
			      const string& libPath, const Ice::StringSeq& targets, const Ice::Current&)
{
    //
    // Setup required variables.
    //
    map<string, string> variables;
    variables["parent"] = _nodeInfo->getNode()->getName();
    variables["name"] = name;
    variables["fqn"] = _nodeInfo->getNode()->getName() + "." + name;
    variables["binpath"] = binPath;
    variables["libpath"] = libPath;

    string dataDir = _nodeInfo->getCommunicator()->getProperties()->getProperty("IcePack.Node.Data");
    variables["datadir"] = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "servers/" + name;

    ServerBuilder builder(_nodeInfo, variables, targets);

    //
    // Parse the server deployment descriptors.
    //
    builder.parse(descriptor);

    //
    // Execute the builder tasks.
    //
    builder.execute();
}

void
IcePack::ServerDeployerI::remove(const string& name, const Ice::Current&)
{
    ServerRegistryPrx registry = _nodeInfo->getServerRegistry();

    //
    // Component path is used to identify the component. For example:
    // node1.server1.service3
    //
    string componentPath = _nodeInfo->getNode()->getName() + "." + name;

    ServerPrx server;
    try
    {
	server = registry->findByName(name);
    }
    catch(const ServerNotExistException&)
    {
	ServerDeploymentException ex;
	ex.server = name;
	ex.reason = "server doesn't exist";
	ex.component = componentPath;
	throw ex;
    }

    
    ServerDescription desc;
    try
    {
	desc = server->getServerDescription();
    }
    catch(const Ice::ObjectNotExistException&)
    {
	ServerDeploymentException ex;
	ex.server = name;
	ex.reason = "server doesn't exist";
	ex.component = componentPath;
	throw ex;
    }

    //
    // Ensure the server is from this node.
    //
    if(desc.node != _nodeInfo->getNode()->getName())
    {
	ServerDeploymentException ex;
	ex.server = name;
	ex.reason = "server is not managed by this node";
	ex.component = componentPath;
	throw ex;
    }

    map<string, string> variables;
    variables["parent"] = _nodeInfo->getNode()->getName();
    variables["name"] = name;
    variables["fqn"] = _nodeInfo->getNode()->getName() + "." + name;
    variables["binpath"] = desc.path;
    variables["libpath"] = "";

    string dataDir = _nodeInfo->getCommunicator()->getProperties()->getProperty("IcePack.Node.Data");
    variables["datadir"] = dataDir + (dataDir[dataDir.length() - 1] == '/' ? "" : "/") + "servers/" + name;

    ServerBuilder builder(_nodeInfo, variables, desc.targets);

    //
    // Parse the server deployment descriptors.
    //
    builder.parse(desc.descriptor);

    //
    // Execute the builder tasks.
    //
    builder.undo();
}
