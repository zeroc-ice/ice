// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
	      const string& binpath, const string& libpath, const Ice::StringSeq& targets) :
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
    Ice::StringSeq _targets;
};

class ApplicationHandler : public ComponentHandler
{
public:

    ApplicationHandler(ApplicationBuilder&);

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    
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
IcePack::ApplicationHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    ComponentHandler::startElement(name, attrs, line, column);
    if(!isCurrentTargetDeployable())
    {
	return;
    }

    if(name == "application")
    {
	string basedir = getAttributeValueWithDefault(attrs, "basedir", "");
	if(!basedir.empty())
	{
	    _builder.overrideBaseDir(basedir);
	}
    }
    else if(name == "node")
    {
	if(!_currentNode.empty())
	{
            ostringstream ostr;
            ostr << "line " << line << ": node element enclosed in a node element is not allowed";
	    throw IceXML::ParserException(__FILE__, __LINE__, ostr.str());
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
    else if(name == "server")
    {
	if(_currentNode.empty())
	{
            ostringstream ostr;
            ostr << "line " << line << ": server element is not allowed outside the scope of a node element";
	    throw IceXML::ParserException(__FILE__, __LINE__, ostr.str());
	}

	string serverName = getAttributeValue(attrs, "name");
	string descriptor = _builder.toLocation(getAttributeValue(attrs, "descriptor"));
	string binpath = _builder.toLocation(getAttributeValueWithDefault(attrs, "binpath", ""));
	string libpath = getAttributeValueWithDefault(attrs, "libpath", "");
	string targets = getAttributeValueWithDefault(attrs, "targets", "");
	_builder.addServer(serverName, _currentNode, descriptor, binpath, libpath, targets);
    }
}

void
IcePack::ApplicationHandler::endElement(const string& name, int line, int column)
{
    if(isCurrentTargetDeployable())
    {
	if(name == "node")
	{
	    _currentNode = "";
	    _builder.setBaseDir(_savedBaseDir);
	}
    }

    ComponentHandler::endElement(name, line, column);
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
	throw IceXML::ParserException(__FILE__, __LINE__, "name attribute value is empty");
    }
    if(nodeName.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "node attribute value is empty");
    }
    if(descriptor.empty())
    {
	throw IceXML::ParserException(__FILE__, __LINE__, "descriptor attribute value is empty");
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
	throw IceXML::ParserException(__FILE__, __LINE__, "can't find node `" + nodeName + "'");
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
	throw IceXML::ParserException(__FILE__, __LINE__, "can't contact node `" + nodeName + "'");
    }
}

void
IcePack::ApplicationBuilder::setBaseDir(const string& basedir)
{
    setVariable("basedir", basedir);
}
