// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceXML/Parser.h>
#include <IceGrid/Admin.h>
#include <IceGrid/DescriptorParser.h>
#include <IceGrid/DescriptorBuilder.h>
#include <IceGrid/Util.h>

#include <stack>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

class DescriptorHandler : public IceXML::Handler
{
public:
    
    DescriptorHandler(const string&, const Ice::CommunicatorPtr&);

    void setAdmin(const IceGrid::AdminPrx&);
    void setVariables(const map<string, string>&, const vector<string>&);

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    virtual void characters(const string&, int, int);
    virtual void error(const string&, int, int);
    
    const ApplicationDescriptor& getApplicationDescriptor() const;

private:
    
    bool isCurrentTargetDeployable() const;
    string elementValue() const;
    vector<string> getTargets(const string&) const;
    void error(const string&) const;
    bool isTargetDeployable(const string&) const;

    const Ice::CommunicatorPtr _communicator;
    IceGrid::AdminPrx _admin;
    string _filename;
    map<string, string> _overrides; 
    vector<string> _targets;
    string _data;
    int _targetCounter;
    bool _isCurrentTargetDeployable;
    int _line;
    int _column;

    auto_ptr<ApplicationDescriptorBuilder> _currentApplication;
    auto_ptr<NodeDescriptorBuilder> _currentNode;
    auto_ptr<TemplateDescriptorBuilder> _currentTemplate;
    auto_ptr<ServerDescriptorBuilder> _currentServer;
    auto_ptr<ServiceDescriptorBuilder> _currentService;
    CommunicatorDescriptorBuilder* _currentCommunicator;

    bool _isTopLevel;
    bool _inAdapter;
    bool _inDbEnv;
    bool _inDistrib;
};

}

DescriptorHandler::DescriptorHandler(const string& filename, const Ice::CommunicatorPtr& communicator) : 
    _communicator(communicator),
    _filename(filename),
    _isCurrentTargetDeployable(true),
    _currentCommunicator(0),
    _isTopLevel(true),
    _inAdapter(false),
    _inDbEnv(false)
{
}

void
DescriptorHandler::setAdmin(const AdminPrx& admin)
{
    _admin = admin;
}

void
DescriptorHandler::setVariables(const map<string, string>& variables, const vector<string>& targets)
{
    _overrides = variables;
    _targets = targets;
}

void 
DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    _line = line;
    _column = column;
    XmlAttributesHelper attributes(attrs, _communicator->getLogger(), _filename, line);

    try
    {
	if(name == "icegrid")
	{
	    if(!_isTopLevel)
	    {
		error("element <icegrid> is a top level element");
	    }
	    _isTopLevel = false;
	}
	else if(_isTopLevel)
	{
	    error("only the <icegrid> element is allowed at the top-level");
	}
	else if(name == "target")
	{
	    if(!_isCurrentTargetDeployable)
	    {
		++_targetCounter;
	    }
	    else
	    {
		_isCurrentTargetDeployable = isTargetDeployable(attributes("name"));
		_targetCounter = 1;
		return;
	    }
	}
	else if(!isCurrentTargetDeployable())
	{
	    //
	    // We don't bother to parse the elements if the elements are enclosed in a target element 
	    // which won't be deployed.
	    //
	    attributes.asMap();
	    return;
	}
	else if(name == "include")
	{
	    string targets = attributes("targets", "");
	    string file = attributes("descriptor");
	    if(file[0] != '/')
	    {
		string::size_type end = _filename.find_last_of('/');
		if(end != string::npos)
		{
		    file = _filename.substr(0, end) + "/" + file;
		}
	    }
	    
	    string oldFileName = _filename;
	    vector<string> oldTargets = _targets;
	    _isTopLevel = true;
	    _filename = file;
	    _targets = getTargets(targets);
	    
	    IceXML::Parser::parse(file, *this);
	    
	    _filename = oldFileName;
	    _targets = oldTargets;
	}
	else if(name == "application")
	{
	    if(_currentApplication.get())
	    {
		error("only one <application> element is allowed");
	    }

	    if(_admin && attributes("import-default-templates", "false") == "true")
	    {
		ApplicationDescriptor application = _admin->getDefaultApplicationDescriptor();
		_currentApplication.reset(new ApplicationDescriptorBuilder(application, attributes, _overrides));
	    }
	    else
	    {
		_currentApplication.reset(new ApplicationDescriptorBuilder(attributes, _overrides));
	    }
	}
	else if(name == "node")
	{
	    if(!_currentApplication.get())
	    {
		error("the <server> element can only be a child of a <application> element");
	    }
	    _currentNode.reset(_currentApplication->createNode(attributes));
	}
	else if(name == "server-instance")
	{
	    if(!_currentNode.get() || _currentServer.get())
	    {
		error("the <server-instance> element can only be a child of a <node> element");
	    }
	    _currentNode->addServerInstance(attributes);
	}
	else if(name == "server")
	{
	    if(!_currentNode.get() && !_currentTemplate.get() || _currentServer.get())
	    {
		error("the <server> element can only be a child of a <node> or <server-template> element");
	    }
	    if(_currentNode.get())
	    {
		_currentServer.reset(_currentNode->createServer(attributes));
	    }
	    else
	    {
		_currentServer.reset(_currentTemplate->createServer(attributes));
	    }
	    _currentCommunicator = _currentServer.get();
	}
	else if(name == "icebox")
	{
	    if(!_currentNode.get() && !_currentTemplate.get() || _currentServer.get())
	    {
		error("the <icebox> element can only be a child of a <node> or <server-template> element");
	    }
	    if(_currentNode.get())
	    {
		_currentServer.reset(_currentNode->createIceBox(attributes));
	    }
	    else
	    {
		_currentServer.reset(_currentTemplate->createIceBox(attributes));
	    }
	    _currentCommunicator = _currentServer.get();
	}
	else if(name == "server-template")
	{
	    if(_currentTemplate.get() || _currentNode.get())
	    {
		error("element <server-template> can only be a child of an <application> element");
	    }
	    _currentTemplate.reset(_currentApplication->createServerTemplate(attributes));
	}
	else if(name == "service-instance")
	{
	    if(!_currentServer.get())
	    {
		error("the <service-instance> element can only be a child of an <icebox> element");
	    }
	    _currentServer->addServiceInstance(attributes);
	}
	else if(name == "service")
	{
	    if(!_currentServer.get() && !_currentTemplate.get() || _currentService.get())
	    {
		error("the <service> element can only be a child of an <icebox> or <service-template> element");
	    }

	    if(_currentServer.get())
	    {
		_currentService.reset(_currentServer->createService(attributes));
	    }
	    else
	    {
		_currentService.reset(_currentTemplate->createService(attributes));
	    }
	    _currentCommunicator = _currentService.get();
	}
	else if(name == "service-template")
	{
	    if(_currentNode.get() || _currentTemplate.get())
	    {
		error("element <service-template> can only be a child of an <application> element");
	    }

	    _currentTemplate.reset(_currentApplication->createServiceTemplate(attributes));
	}
	else if(name == "replicated-adapter")
	{
	    if(!_currentApplication.get())
	    {
		error("the <replicated-adapter> element can only be a child of a <application> element");
	    }
	    _currentApplication->addReplicatedAdapter(attributes);
	    _inAdapter = true;
	}
	else if(name == "load-balancing")
	{
	    if(!_inAdapter || _currentServer.get())
	    {
		error("the <load-balancing> element can only be a child of a <replicated-adapter> element");
	    }
	    _currentApplication->setLoadBalancing(attributes);
	}
	else if(name == "variable")
	{
	    if(_currentNode.get())
	    {
		_currentNode->addVariable(attributes);
	    }
	    else if(_currentApplication.get())
	    {
		_currentApplication->addVariable(attributes);
	    }
	    else
	    {
		error("the <variable> element can only be a child of an <application> or <node> element");
	    }
	}
	else if(name == "parameter")
	{
	    if(!_currentTemplate.get())
	    {
		error("the <parameter> element can only be a child of a <template> element");
	    }
	    _currentTemplate->addParameter(attributes);
	}
	else if(name == "property")
	{
	    if(!_currentCommunicator)
	    {
		error("the <properties> element can only be a child of an <icebox>, <server> or <service> element");
	    }
	    _currentCommunicator->addProperty(attributes);
	}
	else if(name == "adapter")
	{
	    if(!_currentCommunicator)
	    {
		error("the <adapter> element can only be a child of a <server> or <service> element");
	    }
	    _currentCommunicator->addAdapter(attributes);
	    _inAdapter = true;
	}
	else if(name == "object")
	{
	    if(!_inAdapter)
	    {
		error("the <object> element can only be a child of an <adapter> or <replicated-adapter> element");
	    }
	    if(!_currentCommunicator)
	    {
		_currentApplication->addObject(attributes);
	    }
	    else
	    {
		_currentCommunicator->addObject(attributes);
	    }
	}
	else if(name == "distrib")
	{
	    if(!_currentApplication.get() && (!_currentServer.get() || _currentServer.get() != _currentCommunicator))
	    {
		error("the <distrib> element can only be a child of an <application>, <server> or <icebox> element");
	    }
	    if(!_currentServer.get())
	    {
		_currentApplication->addDistribution(attributes);
	    }
	    else
	    {
		_currentServer->addDistribution(attributes);
	    }
	    _inDistrib = true;
	}
	else if(name == "dbenv")
	{
	    if(!_currentCommunicator)
	    {
		error("the <dbenv> element can only be a child of an <server> or <service> element");
	    }
	    _currentCommunicator->addDbEnv(attributes);
	    _inDbEnv = true;
	}
	else if(name == "dbproperty")
	{
	    if(!_inDbEnv)
	    {
		error("the <dbproperty> element can only be a child of a <dbenv> element");
	    }
	    _currentCommunicator->addDbEnvProperty(attributes);
	}

	attributes.checkUnknownAttributes();
    }
    catch(const string& reason)
    {
	error(reason);
    }
    
    _data = "";
}

void 
DescriptorHandler::endElement(const string& name, int line, int column)
{
    _line = line;
    _column = column;
    
    if(name == "target")
    {
	if(!_isCurrentTargetDeployable && --_targetCounter == 0)
	{
	    _isCurrentTargetDeployable = true;
	    _targetCounter = 0;
	}
	return;
    }
    else if(!isCurrentTargetDeployable())
    {
	//
	// We don't bother to parse the elements if the elements are enclosed in a target element 
	// which won't be deployed.
	//
	return;
    }
    else if(name == "node")
    {
	_currentApplication->addNode(_currentNode->getName(), _currentNode->getDescriptor());
	_currentNode.reset(0);
    }
    else if(name == "server" || name == "icebox")
    {
	assert(_currentServer.get());
	if(_currentTemplate.get())
	{
	    _currentTemplate->setDescriptor(_currentServer->getDescriptor());
	}
	else
	{
	    assert(_currentNode.get());
	    _currentNode->addServer(_currentServer->getDescriptor());
	}
	_currentServer.reset(0);
	_currentCommunicator = 0;
    }
    else if(name == "server-template")
    {
	assert(_currentApplication.get());
	_currentApplication->addServerTemplate(_currentTemplate->getId(), _currentTemplate->getDescriptor());
	_currentTemplate.reset(0);
    }
    else if(name == "service")
    {
	assert(_currentService.get());
	if(_currentServer.get())
	{
	    _currentServer->addService(_currentService->getDescriptor());
	}
	else
	{
	    _currentTemplate->setDescriptor(_currentService->getDescriptor());
	}
	_currentService.reset(0);
	_currentCommunicator = _currentServer.get();
    }
    else if(name == "service-template")
    {
	_currentApplication->addServiceTemplate(_currentTemplate->getId(), _currentTemplate->getDescriptor());
	_currentTemplate.reset(0);
    }
    else if(name == "comment")
    {
	if(_currentCommunicator)
	{
	    _currentCommunicator->setDescription(elementValue());
	}
	else if(_currentApplication.get())
	{
	    _currentApplication->setDescription(elementValue());
	}
	else
	{
	    error("element <comment> can only be the child of an  <application>, <server> or <service> element");
	}
    }
    else if(name == "option")
    {
	if(!_currentServer.get())
	{
	    error("element <option> can only be the child of a <server> element");
	}
	_currentServer->addOption(elementValue());
    }
    else if(name == "env")
    {
	if(!_currentServer.get())
	{
	    error("element <env> can only be the child of a <server> element");
	}
	_currentServer->addEnv(elementValue());
    }
    else if(name == "directory")
    {
	if(!_inDistrib)
	{
		error("the <directory> element can only be a child of a <distrib> element");
	}
	if(!_currentServer.get())
	{
	    _currentApplication->addDistributionDirectory(elementValue());
	}
	else
	{
	    _currentServer->addDistributionDirectory(elementValue());
	}
    }
    else if(name == "adapter")
    {
	_inAdapter = false;
    } 
    else if(name == "replicated-adapter")
    {
	_inAdapter = false;
    } 
    else if(name == "dbenv")
    {
	_inDbEnv = false;
    }
    else if(name == "distrib")
    {
	_inDistrib = false;
    }
}

void 
DescriptorHandler::characters(const string& chars, int, int)
{
    if(isCurrentTargetDeployable())
    {
	_data += chars;
    }
}

void 
DescriptorHandler::error(const string& msg, int line, int column)
{
    ostringstream os;
    os << "error in <" << _filename << "> descriptor, line " << line << ", column " << column << ":\n" << msg;
    throw IceXML::ParserException(__FILE__, __LINE__, os.str());
}

const ApplicationDescriptor&
DescriptorHandler::getApplicationDescriptor() const
{
    if(!_currentApplication.get())
    {
	error("no application descriptor defined in this file");
    }
    return _currentApplication->getDescriptor();
}

bool
DescriptorHandler::isCurrentTargetDeployable() const
{
    return _isCurrentTargetDeployable;
}

vector<string>
DescriptorHandler::getTargets(const string& targets) const
{
    vector<string> result;

    if(!targets.empty())
    {
	const string delim = " \t\n\r";

	string::size_type beg = 0;
	string::size_type end = 0;
	do
	{
	    end = targets.find_first_of(delim, end);
	    if(end == string::npos)
	    {
		end = targets.size();
	    }

	    result.push_back(targets.substr(beg, end - beg));
	    beg = ++end;
	}
	while(end < targets.size());
    }

    copy(_targets.begin(), _targets.end(), back_inserter(result));

    return result;
}

void
DescriptorHandler::error(const string& msg) const
{
    ostringstream os;
    os << "error in <" << _filename << "> descriptor, line " << _line << ", column " << _column << ":\n" << msg;
    throw IceXML::ParserException(__FILE__, __LINE__, os.str());
}

string
DescriptorHandler::elementValue() const
{
    return _data;
}

bool
DescriptorHandler::isTargetDeployable(const string& target) const
{
    string application = _currentApplication.get() ? _currentApplication->getDescriptor().name : "";
    string node = _currentNode.get() ? _currentNode->getName() : "";
    string server = _currentServer.get() ? _currentServer->getDescriptor()->id : "";
    string service = _currentService.get() ? _currentService->getDescriptor()->name : "";

    //
    // Compute the current fully qualified name of the communicator.
    //
    string fqn;
    if(!application.empty())
    {
	fqn += (fqn.empty() ? "" : ".") + application;
    }
    if(!node.empty())
    {
	fqn += (fqn.empty() ? "" : ".") + node;
    }
    if(!server.empty())
    {
	fqn += (fqn.empty() ? "" : ".") + server;
    }
    if(!service.empty())
    {
	fqn += (fqn.empty() ? "" : ".") + service;
    }

    //
    // Go through the list of supplied targets and see if we can match one with the current communicator + target.
    //
    for(vector<string>::const_iterator p = _targets.begin(); p != _targets.end(); ++p)
    {
	if((*p) == target)
	{
	    //
	    // A supplied target without any communicator prefix is matching the target.
	    //
	    return true;
	}
	else
	{
	    string communicatorTarget;
	    string::size_type end = 0;
	    while(end != string::npos)
	    {
		//
		// Add the first communicator name from the communicator fully qualified name to the 
		// target and see if matches.
		//
		end = fqn.find('.', end);
		if(end == string::npos)
		{
		    communicatorTarget = fqn + "." + target;
		}
		else
		{
		    communicatorTarget = fqn.substr(0, end) + "." + target;
		    ++end;
		}

		if((*p) == communicatorTarget)
		{
		    return true;
		}
	    }
	}
    }

    return false;
}

ApplicationDescriptor
DescriptorParser::parseDescriptor(const string& descriptor, 
				  const Ice::StringSeq& targets, 
				  const map<string, string>& variables,
				  const Ice::CommunicatorPtr& communicator,
				  const IceGrid::AdminPrx& admin)
{
    DescriptorHandler handler(descriptor, communicator);
    handler.setAdmin(admin);
    handler.setVariables(variables, targets);
    IceXML::Parser::parse(descriptor, handler);
    return handler.getApplicationDescriptor();
}

ApplicationDescriptor
DescriptorParser::parseDescriptor(const string& descriptor, const Ice::CommunicatorPtr& communicator)
{
    DescriptorHandler handler(descriptor, communicator);
    IceXML::Parser::parse(descriptor, handler);
    return handler.getApplicationDescriptor();
}
