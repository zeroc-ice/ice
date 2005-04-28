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
#include <IceGrid/DescriptorHelper.h>
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
    
    DescriptorHandler(const string&);

    void setCommunicator(const Ice::CommunicatorPtr&);
    void setVariables(const std::map<std::string, std::string>&);
    void setTargets(const std::vector<std::string>&);

    virtual void startElement(const string&, const IceXML::Attributes&, int, int);
    virtual void endElement(const string&, int, int);
    virtual void characters(const string&, int, int);
    virtual void error(const string&, int, int);
    
    const ApplicationDescriptorPtr& getApplicationDescriptor() const;
    const ServerDescriptorPtr& getServerDescriptor() const;

private:
    
    std::string getAttributeValue(const IceXML::Attributes&, const std::string&) const;
    std::string getAttributeValueWithDefault(const IceXML::Attributes&, const std::string&, const std::string&) const;
    bool isCurrentTargetDeployable() const;
    std::string elementValue() const;
    std::vector<std::string> getTargets(const std::string&) const;
    void error(const string&) const;
    bool isTargetDeployable(const std::string&) const;

    Ice::CommunicatorPtr _communicator;
    string _filename;
    std::vector<std::string> _targets;    
    DescriptorVariablesPtr _variables;
    std::stack<std::string> _elements;
    std::map<std::string, ServerDescriptorPtr> _serverTemplates;
    std::map<std::string, ServiceDescriptorPtr> _serviceTemplates;
    std::map<std::string, ComponentDescriptorPtr> _templates;
    int _targetCounter;
    bool _isCurrentTargetDeployable;
    int _line;
    int _column;

    auto_ptr<ApplicationDescriptorHelper> _currentApplication;
    auto_ptr<ServerDescriptorHelper> _currentServer;
    std::string _currentServerTemplate;
    auto_ptr<ServiceDescriptorHelper> _currentService;
    std::string _currentServiceTemplate;
    ComponentDescriptorHelper* _currentComponent;

    bool _isTopLevel;
    bool _inProperties;
    bool _inAdapters;
    bool _inAdapter;
    bool _inDbEnv;
};

}

DescriptorHandler::DescriptorHandler(const string& filename) : 
    _filename(filename),
    _variables(new DescriptorVariables()),
    _isCurrentTargetDeployable(true),
    _isTopLevel(true),
    _inProperties(false),
    _inAdapters(false),
    _inAdapter(false),
    _inDbEnv(false)
{
    _variables->push();
}

void
DescriptorHandler::setCommunicator(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
}

void
DescriptorHandler::setVariables(const std::map<std::string, std::string>& variables)
{
    _variables->reset(variables);
}

void
DescriptorHandler::setTargets(const vector<string>& targets)
{
    _targets = targets;
}

void 
DescriptorHandler::startElement(const string& name, const IceXML::Attributes& attrs, int line, int column)
{
    _line = line;
    _column = column;
    XmlAttributesHelper attributes(_variables, attrs);

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
		_isCurrentTargetDeployable = isTargetDeployable(getAttributeValue(attrs, "name"));
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
	    return;
	}
	else if(name == "include")
	{
	    _variables->push();
	    
	    string file;
	    string targets;
	    
	    for(IceXML::Attributes::const_iterator p = attrs.begin(); p != attrs.end(); ++p)
	    {
		if(p->first == "descriptor")
		{
		    file = _variables->substitute(p->second);
		}
		else if(p->first == "targets")
		{
		    targets = _variables->substitute(p->second);
		}
		else
		{
		    string v = _variables->substitute(p->second);
		    (*_variables)[p->first] = v;
		}
	    }
	    
	    if(file.empty())
	    {
		error("attribute `descriptor' is mandatory in element <include>");
	    }
	    
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
	    
	    _variables->pop();
	    _filename = oldFileName;
	    _targets = oldTargets;
	}
	else if(name == "application")
	{
	    if(_currentApplication.get())
	    {
		error("only one <application> element is allowed");
	    }

	    _currentApplication.reset(new ApplicationDescriptorHelper(_communicator, _variables, attrs));
	    (*_variables)["application"] = _currentApplication->getDescriptor()->name;
	}
	else if(name == "node")
	{
	    (*_variables)["node"] = attributes("name");
	}
	else if(name == "server")
	{
	    if(_currentServer.get())
	    {
		error("element <server> inside a server definition");
	    }
	    if(!_variables->hasVariable("node"))
	    {
		error("the <server> element can only be a child of a <node> element");
	    }

	    _variables->push();

	    if(!_currentApplication.get())
	    {
		_currentServer.reset(new ServerDescriptorHelper(_communicator, _variables, attrs));
	    }
	    else
	    {
		_currentServer.reset(_currentApplication->addServer(attrs));
	    }
	    _currentComponent = _currentServer.get();
	    (*_variables)["server"] = _currentServer->getDescriptor()->name;
	}
	else if(name == "server-template")
	{
	    if(_currentServer.get())
	    {
		error("element <server-template> inside a server definition");
	    }

	    _variables->push();
	    _variables->ignoreMissing(true);
	    _variables->escape(false);
	    (*_variables)["node"] = "${node}";

	    _currentServer.reset(_currentApplication->addServerTemplate(attrs));
	    _currentComponent = _currentServer.get();
	    (*_variables)["server"] = _currentServer->getDescriptor()->name;
	}
	else if(name == "service")
	{
	    if(_currentService.get())
	    {
		error("element <service> inside a service definition");
	    }

	    _variables->push();
	    
	    _currentService.reset(_currentServer->addService(attrs));
	    _currentComponent = _currentService.get();
	    (*_variables)["service"] = _currentService->getDescriptor()->name;
	}
	else if(name == "service-template")
	{
	    if(_currentService.get())
	    {
		error("element <service-template> inside a service definition");
	    }

	    _variables->push();
	    _variables->ignoreMissing(true);
	    _variables->escape(false);

	    (*_variables)["node"] = "${node}";
	    (*_variables)["server"] = "${server}";

	    _currentService.reset(_currentApplication->addServiceTemplate(attrs));
	    _currentComponent = _currentService.get();
	    (*_variables)["service"] = _currentService->getDescriptor()->name;
	}
	else if(name == "variable")
	{
	    (*_variables)[attributes("name")] = attributes("value", "");
	}
	else if(name == "properties")
	{
	    if(!_currentComponent)
	    {
		error("the <properties> element can only be a child of a <server> or <service> element");
	    }
	    _inProperties = true;
	}
	else if(name == "property")
	{
	    if(!_inProperties)
	    {
		error("the <property> element can only be a child of a <properties> element");
	    }	    
	    _currentComponent->addProperty(attrs);
	}
	else if(name == "adapters")
	{
	    if(!_currentComponent)
	    {
		error("the <adapters> element can only be a child of a <server> or <service> element");
	    }
	    _inAdapters = true;
	}
	else if(name == "adapter")
	{
	    if(!_inAdapters)
	    {
		error("the <adapter> element can only be a child of an <adapters> element");
	    }
	    _currentComponent->addAdapter(attrs);
	}
	else if(name == "object")
	{
	    if(_inAdapter)
	    {
		error("the <object> element can only be a child of an <adapter> element");
	    }
	    _currentComponent->addObject(attrs);
	}
	else if(name == "dbenv")
	{
	    if(!_currentComponent)
	    {
		error("the <dbenv> element can only be a child of a <server> or <service> element");
	    }
	    _currentComponent->addDbEnv(attrs);
	    _inDbEnv = true;
	}
	else if(name == "dbproperty")
	{
	    if(!_inDbEnv)
	    {
		error("the <dbproperty> element can only be a child of a <dbenv> element");
	    }
	    _currentComponent->addDbEnvProperty(attrs);
	}
    }
    catch(const string& reason)
    {
	error(reason);
    }
	
    _elements.push("");
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
    else if(name == "application")
    {
	_variables->remove("application");
    }
    else if(name == "node")
    {
	_variables->remove("node");
    }
    else if(name == "server" || name == "server-template")
    {
	if(_currentApplication.get())
	{
	    _currentServer.reset(0);
	}
	_currentComponent = 0;
	if(name == "server-template")
	{
	    _variables->ignoreMissing(false);
	    _variables->escape(true);
	}
	_variables->pop();
    }
    else if(name == "service" || name == "service-template")
    {
	_currentService.reset(0);
	_currentComponent = _currentServer.get();
	if(name == "service-template")
	{
	    _variables->ignoreMissing(false);
	    _variables->escape(true);
	}
	_variables->pop();
    }
    else if(name == "comment")
    {
	if(_currentComponent)
	{
	    _currentComponent->setComment(elementValue());
	}
	else if(_currentApplication.get())
	{
	    _currentApplication->setComment(elementValue());
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
    else if(name == "jvm-option")
    {
	if(!_currentServer.get())
	{
	    error("element <env> can only be the child of a <server> element");
	}
	_currentServer->addJvmOption(elementValue());
    }
    else if(name == "properties")
    {
	_inProperties = false;
    }
    else if(name == "adapters")
    {
	_inAdapters = false;
    }
    else if(name == "adapter")
    {
	_inAdapter = false;
    } 
    else if(name == "dbenv")
    {
	_inDbEnv = false;
    }

    _elements.pop();
}

void 
DescriptorHandler::characters(const string& chars, int, int)
{
    _elements.top().assign(chars);
}

void 
DescriptorHandler::error(const string& msg, int line, int column)
{
    ostringstream os;
    os << "error in <" << _filename << "> descriptor, line " << line << ", column " << column << ":\n" << msg;
    throw IceXML::ParserException(__FILE__, __LINE__, os.str());
}

const ApplicationDescriptorPtr&
DescriptorHandler::getApplicationDescriptor() const
{
    if(!_currentApplication.get())
    {
	error("no application descriptor defined in this file");
    }
    return _currentApplication->getDescriptor();
}

const ServerDescriptorPtr&
DescriptorHandler::getServerDescriptor() const
{
    if(!_currentServer.get())
    {
	error("no server descriptor defined in this file");
    }
    return _currentServer->getDescriptor();
}

string
DescriptorHandler::getAttributeValue(const IceXML::Attributes& attrs, const string& name) const
{
    IceXML::Attributes::const_iterator p = attrs.find(name);
    if(p == attrs.end())
    {
	error("missing attribute '" + name + "'");
    }
    string v = _variables->substitute(p->second);
    if(v.empty())
    {
	error("attribute '" + name + "' is empty");
    }
    return v;
}

string
DescriptorHandler::getAttributeValueWithDefault(const IceXML::Attributes& attrs, 
						    const string& name, 
						    const string& def) const
{
    IceXML::Attributes::const_iterator p = attrs.find(name);
    if(p == attrs.end())
    {
        return _variables->substitute(def);
    }
    else
    {
        return _variables->substitute(p->second);
    }
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
    return _variables->substitute(_elements.top());
}

bool
DescriptorHandler::isTargetDeployable(const string& target) const
{
    string application = _variables->getVariable("application");
    string node = _variables->getVariable("node");
    string server = _variables->getVariable("server");
    string service = _variables->getVariable("service");

    //
    // Compute the current fully qualified name of the component.
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
    // Go through the list of supplied targets and see if we can match one with the current component + target.
    //
    for(vector<string>::const_iterator p = _targets.begin(); p != _targets.end(); ++p)
    {
	if((*p) == target)
	{
	    //
	    // A supplied target without any component prefix is matching the target.
	    //
	    return true;
	}
	else
	{
	    string componentTarget;
	    string::size_type end = 0;
	    while(end != string::npos)
	    {
		//
		// Add the first component name from the component fully qualified name to the 
		// target and see if matches.
		//
		end = fqn.find('.', end);
		if(end == string::npos)
		{
		    componentTarget = fqn + "." + target;
		}
		else
		{
		    componentTarget = fqn.substr(0, end) + "." + target;
		    ++end;
		}

		if((*p) == componentTarget)
		{
		    return true;
		}
	    }
	}
    }

    return false;
}

ApplicationDescriptorPtr
DescriptorParser::parseApplicationDescriptor(const string& descriptor, 
					     const Ice::StringSeq& targets, 
					     const map<string, string>& variables,
					     const Ice::CommunicatorPtr& communicator)
{
    DescriptorHandler handler(descriptor);
    handler.setCommunicator(communicator);
    handler.setTargets(targets);
    handler.setVariables(variables);
    IceXML::Parser::parse(descriptor, handler);
    return handler.getApplicationDescriptor();
}

ServerDescriptorPtr
DescriptorParser::parseServerDescriptor(const string& descriptor, 
					const Ice::StringSeq& targets, 
					const map<string, string>& variables,
					const Ice::CommunicatorPtr& communicator)
{
    DescriptorHandler handler(descriptor);
    handler.setCommunicator(communicator);
    handler.setTargets(targets);
    handler.setVariables(variables);
    IceXML::Parser::parse(descriptor, handler);
    return handler.getServerDescriptor();
}
