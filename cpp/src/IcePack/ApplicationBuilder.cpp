// **********************************************************************
//
// Copyright (c) 2001
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

#include <IcePack/ApplicationBuilder.h>

using namespace std;
using namespace IcePack;

namespace IcePack
{

class AddServer : public Task
{
public:
    
    AddServer(const ServerDeployerPrx& deployer, const string& node, const string& name, const string& descriptor,
	      const string& binpath, const string& libpath, const Targets& targets) :
	_deployer(deployer),
	_node(node),
	_name(name),
	_descriptor(descriptor),
	_binpath(binpath),
	_libpath(libpath),
	_targets(targets)
    {
    }
    
    virtual void
    execute()
    {
	try
	{
	    _deployer->add(_name, _descriptor, _binpath, _libpath, _targets);
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the node:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    ex.component = _node + "." + _name;
	    throw ex;
	}
    }

    virtual void
    undo()
    {
	try
	{
	    _deployer->remove(_name);
	}
	catch(const ServerNotExistException& lex)
	{
	    ostringstream os;
	    os << "couldn't remove server from the node:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    ex.component = _node + "." + _name;
	    throw ex;
	}
	catch(const Ice::LocalException& lex)
	{
	    ostringstream os;
	    os << "couldn't contact the node:\n" << lex;

	    ServerDeploymentException ex;
	    ex.server = _name;
	    ex.reason = os.str();
	    ex.component = _node + "." + _name;
	    throw ex;
	}
    }

private:

    ServerDeployerPrx _deployer;
    string _node;
    string _name;
    string _descriptor;
    string _binpath;
    string _libpath;
    Targets _targets;
};

class ApplicationHandler : public ComponentHandler
{
public:

    ApplicationHandler(ApplicationBuilder&);

    virtual void startElement(const XMLCh*, AttributeList&); 
    virtual void endElement(const XMLCh*);
    
private:

    string _currentNode;
    string _savedBaseDir;

    ApplicationBuilder& _builder;
};

}

IcePack::ApplicationHandler::ApplicationHandler(ApplicationBuilder& builder) :
    ComponentHandler(builder),
    _builder(builder)
{
}

void 
IcePack::ApplicationHandler::startElement(const XMLCh* name, AttributeList& attrs)
{
    ComponentHandler::startElement(name, attrs);
    if(!isCurrentTargetDeployable())
    {
	return;
    }

    string str = toString(name);

    if(str == "application")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _builder.overrideBaseDir(basedir);
	}
    }
    else if(str == "node")
    {
	if(!_currentNode.empty())
	{
	    throw DeploySAXParseException("Node element enclosed in an node element is not allowed", _locator);
	}
	_currentNode = getAttributeValue(attrs, "name");

	//
	// Set the current base directory with the one defined in the
	// node element or nothing if it's not defined. Relative path
	// defined inside the scope of the node element, will be
	// relative to this base directory. If this base directory is
	// empty, these path will relative to the working directory of
	// the icepacknode process.
	//
	_savedBaseDir = _builder.substitute("${basedir}");
	_builder.setBaseDir(getAttributeValueWithDefault(attrs, "basedir", ""));
    }
    else if(str == "server")
    {
	if(_currentNode.empty())
	{
	    throw DeploySAXParseException("Server element is not allowed outside the scope of a node element", 
					  _locator);
	}

	string name = getAttributeValue(attrs, "name");
	string descriptor = _builder.toLocation(getAttributeValue(attrs, "descriptor"));
	string binpath = _builder.toLocation(getAttributeValueWithDefault(attrs, "binpath", ""));
	string libpath = getAttributeValueWithDefault(attrs, "libpath", "");
	string targets = getAttributeValueWithDefault(attrs, "targets", "");
	_builder.addServer(name, _currentNode, descriptor, binpath, libpath, targets);
    }
}

void
IcePack::ApplicationHandler::endElement(const XMLCh* name)
{
    string str = toString(name);    

    if(isCurrentTargetDeployable())
    {
	if(str == "node")
	{
	    _currentNode = "";
	    _builder.setBaseDir(_savedBaseDir);
	}
    }

    ComponentHandler::endElement(name);
}

IcePack::ApplicationBuilder::ApplicationBuilder(const Ice::CommunicatorPtr& communicator,
						const NodeRegistryPtr& nodeRegistry,
						const vector<string>& targets) :
    ComponentBuilder(communicator, map<string, string>(), targets),
    _nodeRegistry(nodeRegistry)
{
}

void
IcePack::ApplicationBuilder::parse(const string& descriptor)
{
    ApplicationHandler handler(*this);    

    ComponentBuilder::parse(descriptor, handler);
}

void
IcePack::ApplicationBuilder::addServer(const string& name,
				       const string& nodeName,
				       const string& descriptor, 
				       const string& binpath,
				       const string& libpath,
				       const string& additionalTargets)
{
    if(name.empty())
    {
	throw DeploySAXParseException("name attribute value is empty", _locator);
    }
    if(nodeName.empty())
    {
	throw DeploySAXParseException("node attribute value is empty", _locator);
    }
    if(descriptor.empty())
    {
	throw DeploySAXParseException("descriptor attribute value is empty", _locator);
    }
    
    vector<string> targets = toTargets(additionalTargets);
    copy(_targets.begin(), _targets.end(), back_inserter(targets));

    NodePrx node;
    try
    {
	node = _nodeRegistry->findByName(nodeName);
    }
    catch(const NodeNotExistException&)
    {
	throw DeploySAXParseException("can't find node `" + nodeName + "'", _locator);
    }

    //
    // TODO: the path here are path which are relative to the node
    // current directory. This might be very confusing.
    //
    try
    {
	ServerDeployerPrx deployer = node->getServerDeployer();	    
	_tasks.push_back(new AddServer(deployer, nodeName, name, descriptor, binpath, libpath, targets));
    }
    catch(::Ice::LocalException&)
    {
	throw DeploySAXParseException("can't contact node `" + nodeName + "'", _locator);
    }
}

void
IcePack::ApplicationBuilder::setBaseDir(const string& basedir)
{
    _variables["basedir"] = basedir;
}
