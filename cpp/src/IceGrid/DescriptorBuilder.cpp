// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/IdentityUtil.h>
#include <IceGrid/DescriptorBuilder.h>

using namespace std;
using namespace IceGrid;

XmlAttributesHelper::XmlAttributesHelper(const IceXML::Attributes& attrs) : _attributes(attrs)
{
}

bool
XmlAttributesHelper::contains(const string& name) const
{
    return _attributes.find(name) != _attributes.end();
}

string 
XmlAttributesHelper::operator()(const string& name) const
{
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
	throw "missing attribute '" + name + "'";
    }
    string v = p->second;
    if(v.empty())
    {
	throw "attribute '" + name + "' is empty";
    }
    return v;
}

string
XmlAttributesHelper::operator()(const string& name, const string& def) const
{
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
	return def;
    }
    else
    {
	return p->second;
    }
}

map<string, string>
XmlAttributesHelper::asMap() const
{
    return _attributes;
}

void
DescriptorBuilder::addVariable(const XmlAttributesHelper&)
{
    throw "the <variable> element can't be a child of this element";
}

ApplicationDescriptorBuilder::ApplicationDescriptorBuilder(const XmlAttributesHelper& attrs, 
							   const map<string, string>& overrides) :
    _overrides(overrides)
{
    _descriptor.name = attrs("name");
    _descriptor.variables = overrides;
}

const ApplicationDescriptor&
ApplicationDescriptorBuilder::getDescriptor() const
{
    return _descriptor;
}

void
ApplicationDescriptorBuilder::setDescription(const string& desc)
{
    _descriptor.description = desc;
}

void
ApplicationDescriptorBuilder::addReplicatedAdapter(const XmlAttributesHelper& attrs)
{
    ReplicatedAdapterDescriptor adapter;
    adapter.id = attrs("id");
    string policy = attrs("load-balancing", "random");
    if(policy == "random")
    {
	adapter.loadBalancing = new RandomLoadBalancingPolicy();
    }
    else if(policy == "round-robin")
    {
	adapter.loadBalancing = new RoundRobinLoadBalancingPolicy();
    }
    else
    {
	throw "invalid load balancing policy `" + policy + "'";
    }
    _descriptor.replicatedAdapters.push_back(adapter);
}

void
ApplicationDescriptorBuilder::addObject(const XmlAttributesHelper& attrs)
{
    ObjectDescriptor object;
    object.type = attrs("type", "");
    object.id = Ice::stringToIdentity(attrs("identity"));
    _descriptor.replicatedAdapters.back().objects.push_back(object);
}

void
ApplicationDescriptorBuilder::addVariable(const XmlAttributesHelper& attrs)
{
    if(!isOverride(attrs("name")))
    {
	_descriptor.variables[attrs("name")] = attrs("value", "");
    }
}

auto_ptr<NodeDescriptorBuilder>
ApplicationDescriptorBuilder::createNode(const XmlAttributesHelper& attrs)
{
    return auto_ptr<NodeDescriptorBuilder>(new NodeDescriptorBuilder(*this, attrs));
}

auto_ptr<TemplateDescriptorBuilder>
ApplicationDescriptorBuilder::createServerTemplate(const XmlAttributesHelper& attrs)
{
    return auto_ptr<TemplateDescriptorBuilder>(new TemplateDescriptorBuilder(*this, attrs, false));
}

auto_ptr<TemplateDescriptorBuilder>
ApplicationDescriptorBuilder::createServiceTemplate(const XmlAttributesHelper& attrs)
{
    return auto_ptr<TemplateDescriptorBuilder>(new TemplateDescriptorBuilder(*this, attrs, true));
}

void
ApplicationDescriptorBuilder::addNode(const string& name, const NodeDescriptor& desc)
{
    NodeDescriptorDict::iterator p = _descriptor.nodes.find(name);
    if(p != _descriptor.nodes.end())
    {
	NodeDescriptor& n = p->second;

	map<string, string> variables(desc.variables.begin(), desc.variables.end());
	n.variables.swap(variables);
	n.variables.insert(variables.begin(), variables.end());

	n.serverInstances.insert(n.serverInstances.end(), desc.serverInstances.begin(), desc.serverInstances.end());
	n.servers.insert(n.servers.end(), desc.servers.begin(), desc.servers.end());
    }
    else
    {
	_descriptor.nodes.insert(make_pair(name, desc));
    }
}

void
ApplicationDescriptorBuilder::addServerTemplate(const string& id, const TemplateDescriptor& templ)
{
    if(!_descriptor.serverTemplates.insert(make_pair(id, templ)).second)
    {
	throw "duplicate server template `" + id + "'";
    }
}

void
ApplicationDescriptorBuilder::addServiceTemplate(const string& id, const TemplateDescriptor& templ)
{
    if(!_descriptor.serviceTemplates.insert(make_pair(id, templ)).second)
    {
	throw "duplicate service template `" + id + "'";
    }
}

bool
ApplicationDescriptorBuilder::isOverride(const string& name)
{
    return _overrides.find(name) != _overrides.end();
}

NodeDescriptorBuilder::NodeDescriptorBuilder(ApplicationDescriptorBuilder& app, const XmlAttributesHelper& attrs) :
    _application(app)
{
    _name = attrs("name");
}

auto_ptr<ServerDescriptorBuilder>
NodeDescriptorBuilder::createServer(const XmlAttributesHelper& attrs)
{
    return auto_ptr<ServerDescriptorBuilder>(new ServerDescriptorBuilder(attrs));
}

auto_ptr<IceBoxDescriptorBuilder>
NodeDescriptorBuilder::createIceBox(const XmlAttributesHelper& attrs)
{
    return auto_ptr<IceBoxDescriptorBuilder>(new IceBoxDescriptorBuilder(attrs));
}

void
NodeDescriptorBuilder::addVariable(const XmlAttributesHelper& attrs)
{
    if(!_application.isOverride(attrs("name")))
    {
	_descriptor.variables[attrs("name")] = attrs("value", "");
    }
}

void
NodeDescriptorBuilder::addServerInstance(const XmlAttributesHelper& attrs)
{
    ServerInstanceDescriptor instance;
    instance._cpp_template = attrs("template");
    instance.parameterValues = attrs.asMap();
    instance.parameterValues.erase("template");
    _descriptor.serverInstances.push_back(instance);
}

void
NodeDescriptorBuilder::addServer(const ServerDescriptorPtr& server)
{
    _descriptor.servers.push_back(server);
}

TemplateDescriptorBuilder::TemplateDescriptorBuilder(ApplicationDescriptorBuilder& application, 
						     const XmlAttributesHelper& attrs, 
						     bool serviceTemplate) :
    _application(application),
    _serviceTemplate(serviceTemplate),
    _id(attrs("id"))
{
}

void
TemplateDescriptorBuilder::addParameter(const XmlAttributesHelper& attrs)
{
    _descriptor.parameters.push_back(attrs("name"));
}

void
TemplateDescriptorBuilder::setDescriptor(const CommunicatorDescriptorPtr& desc)
{
    _descriptor.descriptor = desc;
}

auto_ptr<ServerDescriptorBuilder>
TemplateDescriptorBuilder::createServer(const XmlAttributesHelper& attrs)
{
    if(_serviceTemplate)
    {
	throw "<server> element can't be a child of <service-template>";
    }
    return auto_ptr<ServerDescriptorBuilder>(new ServerDescriptorBuilder(attrs));
}

auto_ptr<IceBoxDescriptorBuilder>
TemplateDescriptorBuilder::createIceBox(const XmlAttributesHelper& attrs)
{
    if(_serviceTemplate)
    {
	throw "<icebox> element can't be a child of <service-template>";
    }
    return auto_ptr<IceBoxDescriptorBuilder>(new IceBoxDescriptorBuilder(attrs));
}

auto_ptr<ServiceDescriptorBuilder>
TemplateDescriptorBuilder::createService(const XmlAttributesHelper& attrs)
{
    if(!_serviceTemplate)
    {
	throw "<service> element can't be a child of <server-template>";
    }
    return auto_ptr<ServiceDescriptorBuilder>(new ServiceDescriptorBuilder(attrs));
}

void
CommunicatorDescriptorBuilder::init(const CommunicatorDescriptorPtr& desc, const XmlAttributesHelper&)
{
    _descriptor = desc;
}

void
CommunicatorDescriptorBuilder::setDescription(const string& desc)
{
    _descriptor->description = desc;
}

void
CommunicatorDescriptorBuilder::addProperty(const XmlAttributesHelper& attrs)
{
    PropertyDescriptor prop;
    prop.name = attrs("name");
    prop.value = attrs("value", "");
    _descriptor->properties.push_back(prop);
}

void
CommunicatorDescriptorBuilder::addAdapter(const XmlAttributesHelper& attrs)
{
    AdapterDescriptor desc;
    desc.name = attrs("name");
    desc.id = attrs("id", "");
    if(desc.id.empty())
    {
	string fqn = "${server}";
	if(ServiceDescriptorPtr::dynamicCast(_descriptor))
	{
	    fqn += ".${service}";
	}
	desc.id = fqn + "." + desc.name;
    }
    desc.registerProcess = attrs("register", "false") == "true";
    desc.waitForActivation = attrs("wait-for-activation", "true") == "true";
    _descriptor->adapters.push_back(desc);

    if(attrs.contains("endpoints"))
    {
	PropertyDescriptor prop;
	prop.name = desc.name + ".Endpoints";
	prop.value = attrs("endpoints");
	_descriptor->properties.push_back(prop);
    }
}

void
CommunicatorDescriptorBuilder::addObject(const XmlAttributesHelper& attrs)
{
    ObjectDescriptor object;
    object.type = attrs("type", "");
    object.id = Ice::stringToIdentity(attrs("identity"));
    _descriptor->adapters.back().objects.push_back(object);
}

void
CommunicatorDescriptorBuilder::addDbEnv(const XmlAttributesHelper& attrs)
{
    DbEnvDescriptor desc;
    desc.name = attrs("name");


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
	desc.dbHome = attrs("home", "");
    }
    
    _descriptor->dbEnvs.push_back(desc);
}

void
CommunicatorDescriptorBuilder::addDbEnvProperty(const XmlAttributesHelper& attrs)
{
    PropertyDescriptor prop;
    prop.name = attrs("name");
    prop.value = attrs("value", "");
    _descriptor->dbEnvs.back().properties.push_back(prop);
}

ServerDescriptorBuilder::ServerDescriptorBuilder(const XmlAttributesHelper& attrs)
{
    init(new ServerDescriptor(), attrs);
}

ServerDescriptorBuilder::ServerDescriptorBuilder()
{
}

void
ServerDescriptorBuilder::init(const ServerDescriptorPtr& desc, const XmlAttributesHelper& attrs)
{
    CommunicatorDescriptorBuilder::init(desc, attrs);
    _descriptor = desc;
    _descriptor->id = attrs("id");
    _descriptor->exe = attrs("exe");
    _descriptor->activationTimeout = attrs("activation-timeout", "0");
    _descriptor->deactivationTimeout = attrs("deactivation-timeout", "0");
    _descriptor->pwd = attrs("pwd", "");
    _descriptor->activation = attrs("activation", "manual");
}

auto_ptr<ServiceDescriptorBuilder>
ServerDescriptorBuilder::createService(const XmlAttributesHelper& attrs)
{
    throw "<service> element can only be a child of an <icebox> element";
}

void
ServerDescriptorBuilder::addOption(const string& v)
{
    _descriptor->options.push_back(v);
}

void
ServerDescriptorBuilder::addEnv(const string& v)
{
    _descriptor->envs.push_back(v);
}

void
ServerDescriptorBuilder::addService(const ServiceDescriptorPtr& desc)
{
    assert(false);
}

void
ServerDescriptorBuilder::addServiceInstance(const XmlAttributesHelper& desc)
{
    throw "<service-instance> element can only be a child of an <icebox> element";
}

IceBoxDescriptorBuilder::IceBoxDescriptorBuilder(const XmlAttributesHelper& attrs)
{
    init(new IceBoxDescriptor(), attrs);
}

void
IceBoxDescriptorBuilder::init(const IceBoxDescriptorPtr& desc, const XmlAttributesHelper& attrs)
{
    ServerDescriptorBuilder::init(desc, attrs);
    _descriptor = desc;

    PropertyDescriptor prop;
    prop.name = "IceBox.ServiceManager.Identity";
    prop.value = _descriptor->id + "/ServiceManager";
    _descriptor->properties.push_back(prop);
    
    AdapterDescriptor adapter;
    adapter.name = "IceBox.ServiceManager";
    adapter.id = _descriptor->id + "." + adapter.name;
    adapter.registerProcess = true;
    adapter.waitForActivation = true;
    _descriptor->adapters.push_back(adapter);

    prop.name = "IceBox.ServiceManager.Endpoints";
    prop.value = attrs("endpoints");
    _descriptor->properties.push_back(prop);
}

auto_ptr<ServiceDescriptorBuilder>
IceBoxDescriptorBuilder::createService(const XmlAttributesHelper& attrs)
{
    return auto_ptr<ServiceDescriptorBuilder>(new ServiceDescriptorBuilder(attrs));
}

void
IceBoxDescriptorBuilder::addAdapter(const XmlAttributesHelper& attrs)
{
    throw "<adapter> element can't be a child of an <icebox> element";
}

void
IceBoxDescriptorBuilder::addDbEnv(const XmlAttributesHelper& attrs)
{
    throw "<dbenv> element can't be a child of an <icebox> element";
}

void
IceBoxDescriptorBuilder::addServiceInstance(const XmlAttributesHelper& attrs)
{
    ServiceInstanceDescriptor instance;
    instance._cpp_template = attrs("template");
    instance.parameterValues = attrs.asMap();
    instance.parameterValues.erase("template");
    _descriptor->services.push_back(instance);
}

void
IceBoxDescriptorBuilder::addService(const ServiceDescriptorPtr& desc)
{
    ServiceInstanceDescriptor instance;
    assert(desc);
    instance.descriptor = desc;
    _descriptor->services.push_back(instance);
}

ServiceDescriptorBuilder::ServiceDescriptorBuilder(const XmlAttributesHelper& attrs)
{
    init(new ServiceDescriptor(), attrs);
}

void
ServiceDescriptorBuilder::init(const ServiceDescriptorPtr& desc, const XmlAttributesHelper& attrs)
{
    CommunicatorDescriptorBuilder::init(desc, attrs);
    _descriptor = desc;
    _descriptor->name = attrs("name");
    _descriptor->entry = attrs("entry");
}
