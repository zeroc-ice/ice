// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IceGrid/DescriptorHelper.h>

#include <iterator>

using namespace std;
using namespace IceGrid;

namespace IceGrid
{

struct Substitute : unary_function<string&, void>
{
    Substitute(const DescriptorVariablesPtr& variables, vector<string>& missing) : 
	_variables(variables), _missing(missing)
    {
    } 

    void operator()(string& v)
    {
	v.assign(_variables->substituteWithMissing(v, _missing));
    }

    const DescriptorVariablesPtr& _variables;
    vector<string>& _missing;
};

}

DescriptorVariables::DescriptorVariables() :
    _ignoreMissing(false)
{
}

DescriptorVariables::DescriptorVariables(const map<string, string>& variables) :
    _ignoreMissing(false)
{
    reset(variables);
}

string 
DescriptorVariables::substitute(const string& v) const
{
    vector<string> missing;
    return substituteImpl(v, _ignoreMissing, missing);
}

string
DescriptorVariables::substituteWithMissing(const string& v, vector<string>& missing) const
{
    return substituteImpl(v, true, missing);
}

void
DescriptorVariables::dumpVariables() const
{
    vector<map<string, string> >::const_reverse_iterator p = _variables.rbegin();
    while(p != _variables.rend())
    {
	for(map<string, string>::const_iterator q = p->begin(); q != p->end(); ++q)
	{
	    cout << q->first << " = " << q->second << endl;
	}
	++p;
    }
}

string
DescriptorVariables::getVariable(const string& name) const
{
    static const string empty;
    vector<map<string, string> >::const_reverse_iterator p = _variables.rbegin();
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

bool
DescriptorVariables::hasVariable(const string& name) const
{
    vector<map<string, string> >::const_reverse_iterator p = _variables.rbegin();
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

void
DescriptorVariables::remove(const string& name)
{
    _variables.back().erase(name);
}

void
DescriptorVariables::reset(const map<string, string>& vars)
{
    _variables.clear();
    _variables.push_back(vars);
}

void
DescriptorVariables::push(const map<string, string>& vars)
{
    _variables.push_back(vars);
}

void
DescriptorVariables::push()
{
    _variables.push_back(map<string, string>());
}

void
DescriptorVariables::pop()
{
    _variables.pop_back();
}

void
DescriptorVariables::ignoreMissing(bool ignoreMissing)
{
    _ignoreMissing = ignoreMissing;
}

string&
DescriptorVariables::operator[](const string& name)
{
    return _variables.back()[name];
}

string
DescriptorVariables::substituteImpl(const string& v, bool ignoreMissing, vector<string>& missing) const
{
    string value(v);
    string::size_type beg = 0;
    string::size_type end = 0;

    while((beg = value.find("${", beg)) != string::npos)
    {
	if(beg > 0 && value[beg - 1] == '$')
	{
	    string::size_type escape = beg - 1;
	    while(escape > 0 && value[escape - 1] == '$')
	    {
		--escape;
	    }

	    value.replace(escape, beg - escape, (beg - escape) / 2, '$');
	    if((beg - escape) % 2)
	    {
		++beg;
		continue;
	    }
	    else
	    {
		beg -= (beg - escape) / 2;
	    }
	}

	end = value.find("}", beg);
	
	if(end == string::npos)
	{
	    throw "malformed variable name in the '" + value + "' value";
	}
	
	string name = value.substr(beg + 2, end - beg - 2);
	if(!hasVariable(name))
	{
	    if(!ignoreMissing)
	    {
		throw "unknown variable `" + name + "'";
	    }
	    else
	    {
		missing.push_back(name);
		++beg;
		continue;
	    }
	}
	else
	{
	    string val = getVariable(name);
	    if(val != value.substr(beg, end - beg + 1))
	    {
		value.replace(beg, end - beg + 1, val);
	    }
	    else
	    {
		++beg;
		continue;
	    }
	}
    }

    return value;
}

DescriptorTemplates::DescriptorTemplates(const ApplicationDescriptorPtr& descriptor)
{
    if(descriptor)
    {
	_serverTemplates = descriptor->templates;
    }
}

ServerDescriptorPtr
DescriptorTemplates::instantiateServer(const DescriptorHelper& helper, const string& name, 
				       const map<string, string>& attrs)
{
    map<string, ServerDescriptorPtr>::const_iterator p = _serverTemplates.find(name);
    if(p == _serverTemplates.end())
    {
	throw "unknown template `" + name + "'";
    }

    vector<string> missing;
    Substitute substitute(helper.getVariables(), missing);
    map<string, string> attributes = attrs;
    for(map<string, string>::iterator p = attributes.begin(); p != attributes.end(); ++p)
    {
	substitute(p->second);
    }

    helper.getVariables()->push(attributes);
    ServerDescriptorPtr descriptor = ServerDescriptorHelper(helper, p->second).instantiate(missing);
    helper.getVariables()->pop();

    if(!missing.empty())
    {
	ostringstream os;
	os << "server template instance undefined variables: ";
	copy(missing.begin(), missing.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }

    return descriptor;
}

ServiceDescriptorPtr
DescriptorTemplates::instantiateService(const DescriptorHelper& helper, const string& name, 
					const map<string, string>& attrs)
{
    map<string, ServiceDescriptorPtr>::const_iterator p = _serviceTemplates.find(name);
    if(p == _serviceTemplates.end())
    {
	throw "unknown template `" + name + "'";
    }
    
    vector<string> missing;
    Substitute substitute(helper.getVariables(), missing);
    map<string, string> attributes = attrs;
    for(map<string, string>::iterator p = attributes.begin(); p != attributes.end(); ++p)
    {
	substitute(p->second);
    }

    helper.getVariables()->push(attributes);
    ServiceDescriptorPtr descriptor = ServiceDescriptorHelper(helper, p->second).instantiate(missing);
    helper.getVariables()->pop();

    if(!missing.empty())
    {
	ostringstream os;
	os << "service template instance undefined variables: ";
	copy(missing.begin(), missing.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }

    return descriptor;
}

void
DescriptorTemplates::addServerTemplate(const string& id, const ServerDescriptorPtr& descriptor)
{
    _serverTemplates.insert(make_pair(id, descriptor));
}

void
DescriptorTemplates::addServiceTemplate(const string& id, const ServiceDescriptorPtr& descriptor)
{
    _serviceTemplates.insert(make_pair(id, descriptor));
}

XmlAttributesHelper::XmlAttributesHelper(const DescriptorVariablesPtr& variables, const IceXML::Attributes& attrs) :
	_variables(variables), _attributes(attrs)
{
}

bool
XmlAttributesHelper::contains(const string& name)
{
    return _attributes.find(name) != _attributes.end();
}

string 
XmlAttributesHelper::operator()(const string& name)
{
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
	throw "missing attribute '" + name + "'";
    }
    string v = _variables->substitute(p->second);
    if(v.empty())
    {
	throw "attribute '" + name + "' is empty";
    }
    return v;
}

string
XmlAttributesHelper::operator()(const string& name, const string& def)
{
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
	return _variables->substitute(def);
    }
    else
    {
	return _variables->substitute(p->second);
    }
}

DescriptorHelper::DescriptorHelper(const Ice::CommunicatorPtr& communicator, 
				   const DescriptorVariablesPtr& vars,
				   const DescriptorTemplatesPtr& templates) :
    _communicator(communicator),
    _variables(vars),
    _templates(templates)
{
}

DescriptorHelper::DescriptorHelper(const DescriptorHelper& desc) :
    _communicator(desc._communicator),
    _variables(desc._variables),
    _templates(desc._templates)
{
}

DescriptorHelper::~DescriptorHelper()
{
}

const DescriptorVariablesPtr&
DescriptorHelper::getVariables() const
{
    return _variables;
}

ApplicationDescriptorHelper::ApplicationDescriptorHelper(const Ice::CommunicatorPtr& communicator,
							 const ApplicationDescriptorPtr& descriptor) :
    DescriptorHelper(communicator, new DescriptorVariables(), new DescriptorTemplates(descriptor)),
    _descriptor(descriptor)
{
}

ApplicationDescriptorHelper::ApplicationDescriptorHelper(const Ice::CommunicatorPtr& communicator,
							 const DescriptorVariablesPtr& variables,
							 const IceXML::Attributes& attrs) :
    DescriptorHelper(communicator, variables, new DescriptorTemplates()),
    _descriptor(new ApplicationDescriptor())
{
    XmlAttributesHelper attributes(_variables, attrs);
    _descriptor->name = attributes("name");
}

const ApplicationDescriptorPtr&
ApplicationDescriptorHelper::getDescriptor() const
{
    return _descriptor;
}

void
ApplicationDescriptorHelper::setComment(const string& comment)
{
    _descriptor->comment = comment;
}

ServerDescriptorHelper*
ApplicationDescriptorHelper::addServer(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    //
    // Check if the server element is an instantiation of a template
    // (i.e.: the `template' attribute is specified) or a definition.
    //
    auto_ptr<ServerDescriptorHelper> server;
    if(!attributes.contains("template"))
    {
	server.reset(new ServerDescriptorHelper(*this, attrs));
    }
    else
    {
	ServerDescriptorPtr desc = _templates->instantiateServer(*this, attributes("template"), attrs);
	server.reset(new ServerDescriptorHelper(*this, desc));
    }

    //
    // Add the server to the application.
    //
    _descriptor->servers.push_back(server->getDescriptor());
    return server.release();
}

ServerDescriptorHelper*
ApplicationDescriptorHelper::addServerTemplate(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    //
    // Add the template to the application.
    //
    auto_ptr<ServerDescriptorHelper> server(new ServerDescriptorHelper(*this, attrs));
    _descriptor->templates.insert(make_pair(attributes("id"), server->getDescriptor()));
    _templates->addServerTemplate(attributes("id"), server->getDescriptor());
    return server.release();
}

ServiceDescriptorHelper*
ApplicationDescriptorHelper::addServiceTemplate(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    //
    // Add the template to the application.
    //
    auto_ptr<ServiceDescriptorHelper> service(new ServiceDescriptorHelper(*this, attrs));
    _templates->addServiceTemplate(attributes("id"), service->getDescriptor());
    return service.release();
}

ComponentDescriptorHelper::ComponentDescriptorHelper(const DescriptorHelper& helper) : DescriptorHelper(helper)
{
}

ComponentDescriptorHelper::ComponentDescriptorHelper(const Ice::CommunicatorPtr& communicator, 
						     const DescriptorVariablesPtr& vars,
						     const DescriptorTemplatesPtr& templates) :
    DescriptorHelper(communicator, vars, templates)
{
}

void
ComponentDescriptorHelper::init(const ComponentDescriptorPtr& descriptor, const IceXML::Attributes& attrs)
{
    _descriptor = descriptor;

    if(attrs.empty())
    {
	return;
    }

    XmlAttributesHelper attributes(_variables, attrs);
    _descriptor->name = attributes("name");
}

bool
ComponentDescriptorHelper::operator==(const ComponentDescriptorHelper& helper) const
{
    if(_descriptor->ice_id() != helper._descriptor->ice_id())
    {
	return false;
    }

    if(_descriptor->name != helper._descriptor->name)
    {
	return false;
    }

    if(_descriptor->comment != helper._descriptor->comment)
    {
	return false;
    }

    if(set<AdapterDescriptor>(_descriptor->adapters.begin(), _descriptor->adapters.end())  != 
       set<AdapterDescriptor>(helper._descriptor->adapters.begin(), helper._descriptor->adapters.end()))
    {
	return false;
    }

    if(set<PropertyDescriptor>(_descriptor->properties.begin(), _descriptor->properties.end()) != 
       set<PropertyDescriptor>(helper._descriptor->properties.begin(), helper._descriptor->properties.end()))
    {
	return false;
    }

    if(set<DbEnvDescriptor>(_descriptor->dbEnvs.begin(), _descriptor->dbEnvs.end()) != 
       set<DbEnvDescriptor>(helper._descriptor->dbEnvs.begin(), helper._descriptor->dbEnvs.end()))
    {
	return false;
    }

    return true;
}

void
ComponentDescriptorHelper::setComment(const string& comment)
{
    _descriptor->comment = comment;
}

void
ComponentDescriptorHelper::addProperty(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    PropertyDescriptor prop;
    prop.name = attributes("name");
    prop.value = attributes("value", "");
    _descriptor->properties.push_back(prop);
}

void
ComponentDescriptorHelper::addAdapter(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    
    AdapterDescriptor desc;
    desc.name = attributes("name");
    desc.id = attributes("id", "");
    if(desc.id.empty())
    {
	string service = _variables->getVariable("service");
	const string fqn = _variables->getVariable("server") + (service.empty() ? "" : ".") + service;
	desc.id = fqn + "." + desc.name;
    }
    desc.endpoints = attributes("endpoints");
    desc.registerProcess = attributes("register", "false") == "true";
    _descriptor->adapters.push_back(desc);
}

void
ComponentDescriptorHelper::addObject(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    ObjectDescriptor object;
    object.type = attributes("type", "");
    object.proxy = _communicator->stringToProxy(attributes("identity") + "@" + _descriptor->adapters.back().id);
    object.adapterId = _descriptor->adapters.back().id;
    _descriptor->adapters.back().objects.push_back(object);
}

void
ComponentDescriptorHelper::addDbEnv(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    DbEnvDescriptor desc;
    desc.name = attributes("name");

    DbEnvDescriptorSeq::iterator p;
    for(p = _descriptor->dbEnvs.begin(); p != _descriptor->dbEnvs.end(); ++p)
    {
	//
	// We are re-opening the dbenv element to define more properties.
	//
	if(p->name == desc.name)
	{	
	    break;
	}
    }

    if(p != _descriptor->dbEnvs.end())
    {
	//
	// Remove the previously defined dbenv, we'll add it back again when 
	// the dbenv element end tag is reached.
	//
	desc = *p;
	_descriptor->dbEnvs.erase(p);
    }	

    if(desc.dbHome.empty())
    {
	desc.dbHome = attributes("home", "");
    }
    
    _descriptor->dbEnvs.push_back(desc);
}

void
ComponentDescriptorHelper::addDbEnvProperty(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);

    PropertyDescriptor prop;
    prop.name = attributes("name");
    prop.value = attributes("value", "");
    _descriptor->dbEnvs.back().properties.push_back(prop);
}

void
ComponentDescriptorHelper::instantiateImpl(const ComponentDescriptorPtr& desc, vector<string>& missing) const
{
    Substitute substitute(_variables, missing);
    substitute(desc->name);
    substitute(desc->comment);
    for(AdapterDescriptorSeq::iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
    {
	substitute(p->name);
	substitute(p->id);
	substitute(p->endpoints);
	for(ObjectDescriptorSeq::iterator q = p->objects.begin(); q != p->objects.end(); ++q)
	{
	    substitute(q->type);
	    substitute(q->adapterId);
	    string proxy = _communicator->proxyToString(q->proxy);
	    substitute(proxy);
	    q->proxy = _communicator->stringToProxy(proxy);
	}
    }
    for(PropertyDescriptorSeq::iterator p = desc->properties.begin(); p != desc->properties.end(); ++p)
    {
	substitute(p->name);
	substitute(p->value);	
    }
    for(DbEnvDescriptorSeq::iterator p = desc->dbEnvs.begin(); p != desc->dbEnvs.end(); ++p)
    {
	substitute(p->name);
	substitute(p->dbHome);	
	for(PropertyDescriptorSeq::iterator q = p->properties.begin(); q != p->properties.end(); ++q)
	{
	    substitute(q->name);
	    substitute(q->value);	
	}
    }
}

ServerDescriptorHelper::ServerDescriptorHelper(const DescriptorHelper& helper, const ServerDescriptorPtr& descriptor) :
    ComponentDescriptorHelper(helper),
    _descriptor(descriptor)
{
    ComponentDescriptorHelper::init(_descriptor);
}

ServerDescriptorHelper::ServerDescriptorHelper(const DescriptorHelper& helper, const IceXML::Attributes& attrs) :
    ComponentDescriptorHelper(helper)
{
    initFromXml(attrs);
}

ServerDescriptorHelper::ServerDescriptorHelper(const Ice::CommunicatorPtr& communicator,
					       const DescriptorVariablesPtr& variables,
					       const IceXML::Attributes& attrs) :
    ComponentDescriptorHelper(communicator, variables, new DescriptorTemplates())
{
    initFromXml(attrs);
}


void
ServerDescriptorHelper::initFromXml(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    string kind = attributes("kind");
    if(kind == "cpp" || kind == "cs")
    {
	_descriptor = new ServerDescriptor();
	_descriptor->exe = attributes("exe");
    }
    else if(kind == "java")
    {
	JavaServerDescriptorPtr descriptor = new JavaServerDescriptor();
	_descriptor = descriptor;
	_descriptor->exe = attributes("exe", "java");
	descriptor->className = attributes("classname");
    }
    else if(kind == "cpp-icebox")
    {
	_descriptor = new CppIceBoxDescriptor();
	_descriptor->exe = attributes("exe", "icebox");
    }
    else if(kind == "java-icebox")
    {
	JavaIceBoxDescriptorPtr descriptor = new JavaIceBoxDescriptor();
	_descriptor = descriptor;
	_descriptor->exe = attributes("exe", "java");
	descriptor->className = attributes("classname", "IceBox.Server");
    }

    ComponentDescriptorHelper::init(_descriptor, attrs);

    _descriptor->application = _variables->getVariable("application");
    _descriptor->node = _variables->getVariable("node");
    _descriptor->pwd = attributes("pwd", "");
    _descriptor->activation = attributes("activation", "manual");
    
    if(kind == "cpp-icebox" || kind == "java-icebox")
    {
	CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(_descriptor);
	if(cppIceBox)
	{
	    cppIceBox->endpoints = attributes("endpoints");
	}
	JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(_descriptor);
	if(javaIceBox)
	{
	    javaIceBox->endpoints = attributes("endpoints");
	}
	
	PropertyDescriptor prop;
	prop.name = "IceBox.ServiceManager.Identity";
	prop.value = _descriptor->name + "/ServiceManager";
	_descriptor->properties.push_back(prop);
	
	AdapterDescriptor adapter;
	adapter.name = "IceBox.ServiceManager";
	adapter.endpoints = attributes("endpoints");
	adapter.id = _descriptor->name + "." + adapter.name;
	adapter.registerProcess = true;
	_descriptor->adapters.push_back(adapter);
    }
}

bool
ServerDescriptorHelper::operator==(const ServerDescriptorHelper& helper) const
{
    if(!ComponentDescriptorHelper::operator==(helper))
    {
	return false;
    }

    if(_descriptor->exe != helper._descriptor->exe)
    {
	return false;
    }

    if(_descriptor->pwd != helper._descriptor->pwd)
    {
	return false;
    }

    if(_descriptor->node != helper._descriptor->node)
    {
	return false;
    }
    
    if(_descriptor->application != helper._descriptor->application)
    {
	return false;
    }

    if(set<string>(_descriptor->options.begin(), _descriptor->options.end()) != 
       set<string>(helper._descriptor->options.begin(), helper._descriptor->options.end()))
    {
	return false;
    }

    if(set<string>(_descriptor->envs.begin(), _descriptor->envs.end()) != 
       set<string>(helper._descriptor->envs.begin(), helper._descriptor->envs.end()))
    {
	return false;
    }
    
    ServiceDescriptorSeq slhs;
    ServiceDescriptorSeq srhs;

    if(JavaServerDescriptorPtr::dynamicCast(_descriptor))
    {
	JavaServerDescriptorPtr jlhs = JavaServerDescriptorPtr::dynamicCast(_descriptor);
	JavaServerDescriptorPtr jrhs = JavaServerDescriptorPtr::dynamicCast(helper._descriptor);
	
	if(jlhs->className != jrhs->className)
	{
	    return false;
	}

	if(set<string>(jlhs->jvmOptions.begin(), jlhs->jvmOptions.end()) != 
	   set<string>(jrhs->jvmOptions.begin(), jrhs->jvmOptions.end()))
	{
	    return false;
	}
	
	if(JavaIceBoxDescriptorPtr::dynamicCast(_descriptor))
	{
	    JavaIceBoxDescriptorPtr ilhs = JavaIceBoxDescriptorPtr::dynamicCast(_descriptor);
	    JavaIceBoxDescriptorPtr irhs = JavaIceBoxDescriptorPtr::dynamicCast(helper._descriptor);

	    if(ilhs->endpoints != irhs->endpoints)
	    {
		return false;
	    }

	    if(ilhs->services.size() != irhs->services.size())
	    {
		return false;
	    }

	    slhs = ilhs->services;
	    srhs = irhs->services;
	}
    }
    else if(CppIceBoxDescriptorPtr::dynamicCast(_descriptor))
    {
	CppIceBoxDescriptorPtr ilhs = CppIceBoxDescriptorPtr::dynamicCast(_descriptor);
	CppIceBoxDescriptorPtr irhs = CppIceBoxDescriptorPtr::dynamicCast(helper._descriptor);

	if(ilhs->endpoints != irhs->endpoints)
	{
	    return false;
	}

	if(ilhs->services.size() != irhs->services.size())
	{
	    return false;
	}
	
	slhs = ilhs->services;
	srhs = irhs->services;
    }

    if(!slhs.empty())
    {
	for(ServiceDescriptorSeq::const_iterator p = slhs.begin(); p != slhs.end(); ++p)
	{
	    bool found = false;
	    for(ServiceDescriptorSeq::const_iterator q = srhs.begin(); q != srhs.end(); ++q)
	    {
		if((*p)->name == (*q)->name)
		{
		    if(ServiceDescriptorHelper(*this, *p) == ServiceDescriptorHelper(*this, *q))
		    {
			return false;
		    }
		    found = true;
		    break;
		}
	    }
	    if(!found)
	    {
		return false;
	    }
	}
    }

    return true;
}

const ServerDescriptorPtr&
ServerDescriptorHelper::getDescriptor() const
{
    return _descriptor;
}

ServiceDescriptorHelper*
ServerDescriptorHelper::addService(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    
    if(!CppIceBoxDescriptorPtr::dynamicCast(_descriptor) && !JavaIceBoxDescriptorPtr::dynamicCast(_descriptor))
    {
	throw "element <service> can only be a child of an IceBox <server> element";
    }

    //
    // Check if this is a service instantiation (i.e.: the template
    // attribute is specified) or definition.
    //
    auto_ptr<ServiceDescriptorHelper> service;
    if(!attributes.contains("template"))
    {
	service.reset(new ServiceDescriptorHelper(*this, attrs));
    }
    else
    {
	ServiceDescriptorPtr desc = _templates->instantiateService(*this, attributes("template"), attrs);
	service.reset(new ServiceDescriptorHelper(*this, desc));
    }

    CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(_descriptor);
    if(cppIceBox)
    {
	cppIceBox->services.push_back(service->getDescriptor());
    }
    JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(_descriptor);
    if(javaIceBox)
    {
	javaIceBox->services.push_back(service->getDescriptor());
    }
    return service.release();
}

void
ServerDescriptorHelper::addOption(const string& option)
{
    _descriptor->options.push_back(option);
}

void
ServerDescriptorHelper::addEnv(const string& env)
{
    _descriptor->envs.push_back(env);
}

void
ServerDescriptorHelper::addJvmOption(const string& option)
{
    JavaServerDescriptorPtr descriptor = JavaServerDescriptorPtr::dynamicCast(_descriptor);
    if(!descriptor)
    {
	throw "element <jvm-option> can only be the child of a Java <server> element";
    }
    descriptor->jvmOptions.push_back(option);
}

ServerDescriptorPtr
ServerDescriptorHelper::instantiate(vector<string>& missing) const
{
    ServerDescriptorPtr desc = ServerDescriptorPtr::dynamicCast(_descriptor->ice_clone());
    instantiateImpl(desc, missing);
    return desc;    
}

void
ServerDescriptorHelper::instantiateImpl(const ServerDescriptorPtr& desc, vector<string>& missing) const
{
    ComponentDescriptorHelper::instantiateImpl(desc, missing);

    Substitute substitute(_variables, missing);
    substitute(desc->exe);
    substitute(desc->pwd);
    for_each(desc->options.begin(), desc->options.end(), substitute);
    for_each(desc->envs.begin(), desc->envs.end(), substitute);
    if(JavaServerDescriptorPtr::dynamicCast(desc))
    {
	JavaServerDescriptorPtr javaDesc = JavaServerDescriptorPtr::dynamicCast(desc);
 	substitute(javaDesc->className);
 	for_each(javaDesc->jvmOptions.begin(), javaDesc->jvmOptions.end(), substitute);
    }
}

ServiceDescriptorHelper::ServiceDescriptorHelper(const DescriptorHelper& helper, const ServiceDescriptorPtr& desc) :
    ComponentDescriptorHelper(helper),
    _descriptor(desc)
{
    init(_descriptor);
}

ServiceDescriptorHelper::ServiceDescriptorHelper(const DescriptorHelper& helper, const IceXML::Attributes& attrs) :
    ComponentDescriptorHelper(helper),
    _descriptor(new ServiceDescriptor())
{
    XmlAttributesHelper attributes(_variables, attrs);
    init(_descriptor, attrs);

    _descriptor->entry = attributes("entry");
}

bool
ServiceDescriptorHelper::operator==(const ServiceDescriptorHelper& helper) const
{
    if(!ComponentDescriptorHelper::operator==(helper))
    {
	return false;
    }

    if(_descriptor->entry != helper._descriptor->entry)
    {
	return false;
    }

    return true;
}

ServiceDescriptorPtr
ServiceDescriptorHelper::instantiate(vector<string>& missing) const
{
    ServiceDescriptorPtr desc = ServiceDescriptorPtr::dynamicCast(_descriptor->ice_clone());
    instantiateImpl(desc, missing);
    return desc;    
}

const ServiceDescriptorPtr&
ServiceDescriptorHelper::getDescriptor() const
{
    return _descriptor;
}

void
ServiceDescriptorHelper::instantiateImpl(const ServiceDescriptorPtr& desc, vector<string>& missing) const
{
    ComponentDescriptorHelper::instantiateImpl(desc, missing);

    Substitute substitute(_variables, missing);
    substitute(desc->entry);
}
