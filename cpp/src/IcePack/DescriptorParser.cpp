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
#include <IcePack/Admin.h>
#include <IcePack/DescriptorParser.h>

#include <stack>
#include <fstream>

using namespace std;
using namespace Ice;
using namespace IcePack;

namespace IcePack
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
    std::string substitute(const std::string&) const;
    std::string elementValue() const;
    std::vector<std::string> getTargets(const std::string&) const;
    void error(const string&) const;

    const std::string& getVariable(const std::string&) const;
    bool hasVariable(const std::string&) const;
    bool isTargetDeployable(const std::string&) const;

    Ice::CommunicatorPtr _communicator;
    string _filename;
    std::vector<std::string> _targets;    
    std::vector< std::map<std::string, std::string> > _variables;
    std::stack<std::string> _elements;
    int _targetCounter;
    bool _isCurrentTargetDeployable;
    int _line;
    int _column;

    ApplicationDescriptorPtr _currentApplication;
    ServerDescriptorPtr _currentServer;
    ServiceDescriptorPtr _currentService;
    ComponentDescriptorPtr _currentComponent;
    AdapterDescriptor _currentAdapter;
    DbEnvDescriptor _currentDbEnv;

    bool _isTopLevel;
    bool _inProperties;
    bool _inAdapters;
};

}

DescriptorHandler::DescriptorHandler(const string& filename) : 
    _filename(filename),
    _isCurrentTargetDeployable(true),
    _isTopLevel(true),
    _inProperties(false),
    _inAdapters(false)
{
    _variables.push_back(map<string, string>());
}

void
DescriptorHandler::setCommunicator(const Ice::CommunicatorPtr& communicator)
{
    _communicator = communicator;
}

void
DescriptorHandler::setVariables(const std::map<std::string, std::string>& variables)
{
    _variables.clear();
    _variables.push_back(variables);
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
    
    if(name == "icepack")
    {
	if(!_isTopLevel)
	{
	    error("element <icepack> is a top level element");
	}
	_isTopLevel = false;
    }
    else if(_isTopLevel)
    {
	error("only the <icepack> element is allowed at the top-level");
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
	_variables.push_back(map<string, string>());

	string file;
	string targets;

	for(IceXML::Attributes::const_iterator p = attrs.begin(); p != attrs.end(); ++p)
	{
	    if(p->first == "descriptor")
	    {
		file = substitute(p->second);
	    }
	    else if(p->first == "targets")
	    {
		targets = substitute(p->second);
	    }
	    else
	    {
		string v = substitute(p->second);
		_variables.back()[p->first] = v;
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

	_variables.pop_back();
	_filename = oldFileName;
	_targets = oldTargets;
    }
    else if(name == "application")
    {
	if(_currentApplication)
	{
	    error("only one <application> element is allowed");
	}
	_currentApplication = new ApplicationDescriptor();
	_currentApplication->name = getAttributeValue(attrs, "name");
	_variables.back()["application"] = _currentApplication->name;
    }
    else if(name == "node")
    {
	_variables.back()["node"] = getAttributeValue(attrs, "name");
    }
    else if(name == "server")
    {
	if(!hasVariable("node"))
	{
	    error("the <server> element can only be a child of a <node> element");
	}

	string kind = getAttributeValue(attrs, "kind");
	if(kind == "cpp" || kind == "cs")
	{
	    _currentServer = new ServerDescriptor();
	    _currentServer->exe = getAttributeValue(attrs, "exe");
	}
	else if(kind == "java")
	{
	    JavaServerDescriptorPtr descriptor = new JavaServerDescriptor();
	    _currentServer = descriptor;
	    _currentServer->exe = getAttributeValueWithDefault(attrs, "exe", "java");
	    descriptor->className = getAttributeValue(attrs, "classname");
	}
	else if(kind == "cpp-icebox")
	{
	    _currentServer = new CppIceBoxDescriptor();
	    _currentServer->exe = getAttributeValueWithDefault(attrs, "exe", "icebox");
	}
	else if(kind == "java-icebox")
	{
	    JavaIceBoxDescriptorPtr descriptor = new JavaIceBoxDescriptor();
	    _currentServer = descriptor;
	    _currentServer->exe = getAttributeValueWithDefault(attrs, "exe", "java");
	    descriptor->className = getAttributeValueWithDefault(attrs, "classname", "IceBox.Server");
	}

	_currentServer->name = getAttributeValue(attrs, "name");
	_currentServer->node = getVariable("node");
	_currentServer->pwd = getAttributeValueWithDefault(attrs, "pwd", "");
	_currentServer->activation = 
	    getAttributeValueWithDefault(attrs, "activation", "manual") == "on-demand" ? OnDemand : Manual;

	if(_currentApplication)
	{
	    _currentServer->application = _currentApplication->name;
	}	
	
	if(kind == "cpp-icebox" || kind == "java-icebox")
	{
	    CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(_currentServer);
	    if(cppIceBox)
	    {
		cppIceBox->endpoints = getAttributeValue(attrs, "endpoints");
	    }
	    JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(_currentServer);
	    if(javaIceBox)
	    {
		javaIceBox->endpoints = getAttributeValue(attrs, "endpoints");
	    }

	    PropertyDescriptor prop;
	    prop.name = "IceBox.ServiceManager.Identity";
	    prop.value = _currentServer->name + "/ServiceManager";
	    _currentServer->properties.push_back(prop);

	    AdapterDescriptor adapter;
	    adapter.name = "IceBox.ServiceManager";
	    adapter.endpoints = getAttributeValue(attrs, "endpoints");
	    adapter.id = _currentServer->name + "." + adapter.name;
	    adapter.registerProcess = true;
	    _currentServer->adapters.push_back(adapter);
	}

	_currentComponent = _currentServer;
	_variables.back()["server"] = _currentServer->name;
    }
    else if(name == "service")
    {
	if(!CppIceBoxDescriptorPtr::dynamicCast(_currentServer) && 
	   !JavaIceBoxDescriptorPtr::dynamicCast(_currentServer))
	{
	    error("element <service> can only be a child of an IceBox <server> element");
	}

	_currentService = new ServiceDescriptor();
	_currentService->name = getAttributeValue(attrs, "name");
	_currentService->entry = getAttributeValue(attrs, "entry");

	_currentComponent = _currentService;
	_variables.back()["service"] = _currentService->name;
    }
    else if(name == "variable")
    {
	_variables.back()[getAttributeValue(attrs, "name")] = getAttributeValueWithDefault(attrs, "value", "");
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

	PropertyDescriptor prop;
	prop.name = getAttributeValue(attrs, "name");
	prop.value = getAttributeValueWithDefault(attrs, "value", "");
	_currentComponent->properties.push_back(prop);
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

	_currentAdapter.name = getAttributeValue(attrs, "name");
	_currentAdapter.id = getAttributeValueWithDefault(attrs, "id", "");
	if(_currentAdapter.id.empty())
	{
	    string service = getVariable("service");
	    const string fqn = getVariable("server") + (service.empty() ? "" : ".") + service;
	    _currentAdapter.id = fqn + "." + _currentAdapter.name;
	}
	_currentAdapter.endpoints = getAttributeValue(attrs, "endpoints");
	_currentAdapter.registerProcess = getAttributeValueWithDefault(attrs, "register", "false") == "true";
    }
    else if(name == "object")
    {
	if(_currentAdapter.name.empty())
	{
	    error("the <object> element can only be a child of an <adapter> element");
	}

	ObjectDescriptor object;
	object.type = getAttributeValueWithDefault(attrs, "type", "");
	object.proxy = _communicator->stringToProxy(getAttributeValue(attrs, "identity") + "@" + _currentAdapter.id);
	object.adapterId = _currentAdapter.id;
	_currentAdapter.objects.push_back(object);
    }
    else if(name == "dbenv")
    {
	if(!_currentComponent)
	{
	    error("the <dbenv> element can only be a child of a <server> or <service> element");
	}

	_currentDbEnv.name = getAttributeValue(attrs, "name");

	DbEnvDescriptorSeq::iterator p;
	for(p = _currentComponent->dbEnvs.begin(); p != _currentComponent->dbEnvs.end(); ++p)
	{
	    //
	    // We are re-opening the dbenv element to define more properties.
	    //
	    if(p->name == _currentDbEnv.name)
	    {	
		break;
	    }
	}

	if(p != _currentComponent->dbEnvs.end())
	{
	    //
	    // Remove the previously defined dbenv, we'll add it back again when 
	    // the dbenv element end tag is reached.
	    //
	    _currentDbEnv = *p;
	    _currentComponent->dbEnvs.erase(p);
	}	

	if(_currentDbEnv.dbHome.empty())
	{
	    _currentDbEnv.dbHome = getAttributeValueWithDefault(attrs, "home", "");
	}
    }
    else if(name == "dbproperty")
    {
	if(_currentDbEnv.name.empty())
	{
	    error("the <dbproperty> element can only be a child of a <dbenv> element");
	}

	PropertyDescriptor prop;
	prop.name = getAttributeValue(attrs, "name");
	prop.value = getAttributeValueWithDefault(attrs, "value", "");
	_currentDbEnv.properties.push_back(prop);
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
	_variables.back()["application"] = "";
    }
    else if(name == "node")
    {
	_variables.back()["node"] = "";
    }
    else if(name == "server")
    {
	if(_currentApplication)
	{
	    _currentApplication->servers.push_back(_currentServer);
	    _currentServer = 0;
	}
	_currentComponent = 0;
	_variables.back()["server"] = "";
    }
    else if(name == "service")
    {
	CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(_currentServer);
	if(cppIceBox)
	{
	    cppIceBox->services.push_back(_currentService);
	}
	JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(_currentServer);
	if(javaIceBox)
	{
	    javaIceBox->services.push_back(_currentService);
	}
	_currentService = 0;
	_currentComponent = _currentServer;
	_variables.back()["service"] = "";
    }
    else if(name == "comment")
    {
	if(_currentComponent)
	{
	    _currentComponent->comment = elementValue();
	}
	else if(_currentApplication)
	{
	    _currentApplication->comment = elementValue();
	}
    }
    else if(name == "option")
    {
	if(!_currentServer)
	{
	    error("element <option> can only be the child of a <server> element");
	}
	_currentServer->options.push_back(elementValue());
    }
    else if(name == "env")
    {
	if(!_currentServer)
	{
	    error("element <env> can only be the child of a <server> element");
	}
	_currentServer->envs.push_back(elementValue());
    }
    else if(name == "jvm-option")
    {
	JavaServerDescriptorPtr descriptor = JavaServerDescriptorPtr::dynamicCast(_currentServer);
	if(!descriptor)
	{
	    error("element <jvm-option> can only be the child of a Java <server> element");
	}
	descriptor->jvmOptions.push_back(elementValue());
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
	_currentComponent->adapters.push_back(_currentAdapter);
	_currentAdapter = AdapterDescriptor();
    }
    else if(name == "dbenv")
    {
	_currentComponent->dbEnvs.push_back(_currentDbEnv);
	_currentDbEnv = DbEnvDescriptor();
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
    if(!_currentApplication)
    {
	error("no application descriptor defined in this file");
    }
    return _currentApplication;
}

const ServerDescriptorPtr&
DescriptorHandler::getServerDescriptor() const
{
    if(!_currentServer)
    {
	error("no server descriptor defined in this file");
    }
    return _currentServer;
}

string
DescriptorHandler::getAttributeValue(const IceXML::Attributes& attrs, const string& name) const
{
    IceXML::Attributes::const_iterator p = attrs.find(name);
    if(p == attrs.end())
    {
	error("missing attribute '" + name + "'");
    }
    string v = substitute(p->second);
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
        return substitute(def);
    }
    else
    {
        return substitute(p->second);
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

const string&
DescriptorHandler::getVariable(const string& name) const
{
    static const string empty;

    vector< map< string, string> >::const_reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	map<string, string>::const_iterator q = p->find(name);
	if(q != p->end())
	{
	    return q->second;
	}
	++p;
    }
    return empty;
}

string
DescriptorHandler::substitute(const string& v) const
{
    string value(v);
    string::size_type beg = 0;
    string::size_type end = 0;

    while((beg = value.find("${", beg)) != string::npos)
    {
	end = value.find("}", beg);
	
	if(end == string::npos)
	{
	    error("malformed variable name in the '" + value + "' value");
	}
	
	string name = value.substr(beg + 2, end - beg - 2);
	if(!hasVariable(name))
	{
	    error("unknown variable `" + name + "'");
	}
	else
	{
	    value.replace(beg, end - beg + 1, getVariable(name));
	}
    }

    return value;
}

string
DescriptorHandler::elementValue() const
{
    return substitute(_elements.top());
}

bool
DescriptorHandler::hasVariable(const string& name) const
{
    vector< map< string, string> >::const_reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	map<string, string>::const_iterator q = p->find(name);
	if(q != p->end())
	{
	    return true;
	}
	++p;
    }
    return false;
}

bool
DescriptorHandler::isTargetDeployable(const string& target) const
{
    string application = getVariable("application");
    string node = getVariable("node");
    string server = getVariable("server");
    string service = getVariable("service");

    //
    // Compute the current fully qualified name of the component.
    //
    string fqn;
    if(!application.empty())
    {
	fqn = application;
	if(!node.empty())
	{
	    fqn += "." + node;
	}
    }
    else
    {
	if(!node.empty())
	{
	    fqn = node;
	}
    }
    if(!server.empty())
    {
	assert(!node.empty());
	fqn += "." + server;
    }
    if(!service.empty())
    {
	assert(!server.empty());
	fqn += "." + service;
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
