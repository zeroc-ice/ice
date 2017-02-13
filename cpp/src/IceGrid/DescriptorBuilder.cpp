// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Communicator.h>
#include <Ice/LoggerUtil.h>
#include <Ice/LocalException.h>
#include <Ice/Initialize.h>
#include <IceGrid/DescriptorBuilder.h>
#include <IceGrid/Util.h>

using namespace std;
using namespace IceGrid;

XmlAttributesHelper::XmlAttributesHelper(const IceXML::Attributes& attrs, 
                                         const Ice::LoggerPtr& logger,
                                         const string& filename,
                                         int line) :
    _attributes(attrs),
    _logger(logger),
    _filename(filename),
    _line(line)
{
}

void
XmlAttributesHelper::checkUnknownAttributes()
{
    vector<string> notUsed;
    for(map<string, string>::const_iterator p = _attributes.begin(); p != _attributes.end(); ++p)
    {
        if(_used.find(p->first) == _used.end())
        {
            notUsed.push_back(p->first);
        }
    }

    if(!notUsed.empty())
    {
        ostringstream os;
        os << "unknown attributes in <" << _filename << "> descriptor, line " << _line << ":\n" << toString(notUsed);
        throw os.str();
    }
}

bool
XmlAttributesHelper::contains(const string& name) const
{
    _used.insert(name);
    return _attributes.find(name) != _attributes.end();
}

string 
XmlAttributesHelper::operator()(const string& name) const
{
    _used.insert(name);
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
    _used.insert(name);
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
    for(map<string, string>::const_iterator p = _attributes.begin(); p != _attributes.end(); ++p)
    {
        _used.insert(p->first);
    }
    return _attributes;
}

bool
XmlAttributesHelper::asBool(const string& name) const
{
    _used.insert(name);
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
        throw "missing attribute '" + name + "'";
        return true; // Keep the compiler happy.
    }
    else if(p->second == "true")
    {
        return true;
    }    
    else if(p->second == "false")
    {
        return false;
    }
    else
    {
        throw "invalid attribute `" + name + "': value is not 'false' or 'true'";
        return true; // Keep the compiler happy.
    }
}

bool
XmlAttributesHelper::asBool(const string& name, bool def) const
{
    _used.insert(name);
    IceXML::Attributes::const_iterator p = _attributes.find(name);
    if(p == _attributes.end())
    {
        return def;
    }
    else if(p->second == "true")
    {
        return true;
    }    
    else if(p->second == "false")
    {
        return false;
    }
    else
    {
        throw "invalid attribute `" + name + "': value is not 'false' or 'true'";
        return true; // Keep the compiler happy.
    }
}

void
DescriptorBuilder::addVariable(const XmlAttributesHelper&)
{
    throw "the <variable> element can't be a child of this element";
}

PropertySetDescriptorBuilder::PropertySetDescriptorBuilder() : 
    _inPropertySetRef(false)
{
}

void
PropertySetDescriptorBuilder::setId(const string& id)
{
    _id = id;
}

void
PropertySetDescriptorBuilder::setService(const string& service)
{
    _service = service;
}

const string&
PropertySetDescriptorBuilder::getService() const
{
    return _service;
}

const string&
PropertySetDescriptorBuilder::getId() const
{
    assert(!_id.empty());
    return _id;
}

const PropertySetDescriptor&
PropertySetDescriptorBuilder::getDescriptor() const
{
    return _descriptor;
}

void
PropertySetDescriptorBuilder::addProperty(const XmlAttributesHelper& attrs)
{
    PropertyDescriptor prop;
    prop.name = attrs("name");
    prop.value = attrs("value", "");
    _descriptor.properties.push_back(prop);
}

void
PropertySetDescriptorBuilder::addPropertySet(const XmlAttributesHelper& attrs)
{
    if(attrs.contains("id") || !attrs.contains("refid"))
    {
        throw "only <properties refid=\"\"> can be a child of a <properties> element";
    }
    if(!_descriptor.properties.empty())
    {
        throw "<properties refid=\"\"> can't be defined after a <property> element";
    }
    _descriptor.references.push_back(attrs("refid"));
    _inPropertySetRef = true;
}

bool
PropertySetDescriptorBuilder::finish()
{
    if(_inPropertySetRef)
    {
        _inPropertySetRef = false;
        return false;
    }
    return true;
}

ApplicationDescriptorBuilder::ApplicationDescriptorBuilder(const Ice::CommunicatorPtr& communicator,
                                                           const XmlAttributesHelper& attrs, 
                                                           const map<string, string>& overrides) :
    _communicator(communicator),
    _overrides(overrides)
{
    _descriptor.name = attrs("name");
    _descriptor.variables = overrides;
}

ApplicationDescriptorBuilder::ApplicationDescriptorBuilder(const Ice::CommunicatorPtr& communicator,
                                                           const ApplicationDescriptor& app,
                                                           const XmlAttributesHelper& attrs, 
                                                           const map<string, string>& overrides) :
    _communicator(communicator),
    _descriptor(app),
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
ApplicationDescriptorBuilder::addReplicaGroup(const XmlAttributesHelper& attrs)
{
    ReplicaGroupDescriptor adapter;
    adapter.id = attrs("id");
    adapter.proxyOptions = attrs("proxy-options", "");
    adapter.filter = attrs("filter", "");
    _descriptor.replicaGroups.push_back(adapter);
}

void
ApplicationDescriptorBuilder::finishReplicaGroup()
{
    if(!_descriptor.replicaGroups.back().loadBalancing)
    {
        _descriptor.replicaGroups.back().loadBalancing = new RandomLoadBalancingPolicy();
        _descriptor.replicaGroups.back().loadBalancing->nReplicas = "0";
    }
}

void
ApplicationDescriptorBuilder::setLoadBalancing(const XmlAttributesHelper& attrs)
{
    LoadBalancingPolicyPtr policy;
    string type = attrs("type");
    if(type == "random")
    {
        policy = new RandomLoadBalancingPolicy();
    }
    else if(type == "ordered")
    {
        policy = new OrderedLoadBalancingPolicy();
    }
    else if(type == "round-robin")
    {
        policy = new RoundRobinLoadBalancingPolicy();
    }
    else if(type == "adaptive")
    {
        AdaptiveLoadBalancingPolicyPtr alb = new AdaptiveLoadBalancingPolicy();
        alb->loadSample = attrs("load-sample", "1");
        policy = alb;
    }
    else
    {
        throw "invalid load balancing policy `" + type + "'";
    }
    policy->nReplicas = attrs("n-replicas", "1");
    _descriptor.replicaGroups.back().loadBalancing = policy;
}

void
ApplicationDescriptorBuilder::setReplicaGroupDescription(const string& description)
{
    _descriptor.replicaGroups.back().description = description;
}

void
ApplicationDescriptorBuilder::addObject(const XmlAttributesHelper& attrs)
{
    ObjectDescriptor object;
    object.type = attrs("type", "");
    object.id = Ice::stringToIdentity(attrs("identity"));
    object.proxyOptions = attrs("proxy-options", "");
    if(attrs.contains("property"))
    {
        throw "property attribute is not allowed in object descriptors from a replica group";   
    }
    _descriptor.replicaGroups.back().objects.push_back(object);
}

void
ApplicationDescriptorBuilder::addVariable(const XmlAttributesHelper& attrs)
{
    if(!isOverride(attrs("name")))
    {
        _descriptor.variables[attrs("name")] = attrs("value", "");
    }
    else
    {
        attrs.contains("value"); // NOTE: prevents warning about "value" not being used.
    }
}

NodeDescriptorBuilder*
ApplicationDescriptorBuilder::createNode(const XmlAttributesHelper& attrs)
{
    return new NodeDescriptorBuilder(*this, _descriptor.nodes[attrs("name")], attrs);
}

TemplateDescriptorBuilder*
ApplicationDescriptorBuilder::createServerTemplate(const XmlAttributesHelper& attrs)
{
    return new TemplateDescriptorBuilder(*this, attrs, false);
}

TemplateDescriptorBuilder*
ApplicationDescriptorBuilder::createServiceTemplate(const XmlAttributesHelper& attrs)
{
    return new TemplateDescriptorBuilder(*this, attrs, true);
}

PropertySetDescriptorBuilder*
ApplicationDescriptorBuilder::createPropertySet(const XmlAttributesHelper& attrs) const
{
    string id = attrs("id");

    PropertySetDescriptorBuilder* builder = new PropertySetDescriptorBuilder();
    builder->setId(id);
    return builder;
}

void
ApplicationDescriptorBuilder::addNode(const string& name, const NodeDescriptor& desc)
{
    _descriptor.nodes[name] = desc;
}

void
ApplicationDescriptorBuilder::addServerTemplate(const string& id, const TemplateDescriptor& templ)
{
    if(!templ.descriptor)
    {
        throw "invalid server template `" + id + "': server definition is missing";
    }
    if(!_descriptor.serverTemplates.insert(make_pair(id, templ)).second)
    {
        throw "duplicate server template `" + id + "'";
    }
}

void
ApplicationDescriptorBuilder::addServiceTemplate(const string& id, const TemplateDescriptor& templ)
{
    if(!templ.descriptor)
    {
        throw "invalid service template `" + id + "': service definition is missing";
    }
    if(!_descriptor.serviceTemplates.insert(make_pair(id, templ)).second)
    {
        throw "duplicate service template `" + id + "'";
    }
}

void
ApplicationDescriptorBuilder::addPropertySet(const string& id, const PropertySetDescriptor& desc)
{
    if(!_descriptor.propertySets.insert(make_pair(id, desc)).second)
    {
        throw "duplicate property set `" + id + "'";
    }
}

void 
ApplicationDescriptorBuilder::addDistribution(const XmlAttributesHelper& attrs)
{
    _descriptor.distrib.icepatch = attrs("icepatch", "${application}.IcePatch2/server");
}

void
ApplicationDescriptorBuilder::addDistributionDirectory(const string& directory)
{
    _descriptor.distrib.directories.push_back(directory);
}

bool
ApplicationDescriptorBuilder::isOverride(const string& name)
{
    return _overrides.find(name) != _overrides.end();
}

ServerInstanceDescriptorBuilder::ServerInstanceDescriptorBuilder(const XmlAttributesHelper& attrs)
{
    _descriptor._cpp_template = attrs("template");
    _descriptor.parameterValues = attrs.asMap();
    _descriptor.parameterValues.erase("template");
}

PropertySetDescriptorBuilder*
ServerInstanceDescriptorBuilder::createPropertySet(const XmlAttributesHelper& attrs) const
{
    string service;
    if(attrs.contains("service"))
    {
        service = attrs("service"); // Can't be empty.
    }

    PropertySetDescriptorBuilder* builder = new PropertySetDescriptorBuilder();
    builder->setService(service);
    return builder;
}

void
ServerInstanceDescriptorBuilder::addPropertySet(const string& service, const PropertySetDescriptor& desc)
{
    //
    // Allow re-opening of unamed property sets.
    //
    PropertySetDescriptor& p = service.empty() ? _descriptor.propertySet : _descriptor.servicePropertySets[service];
    p.references.insert(p.references.end(), desc.references.begin(), desc.references.end());
    p.properties.insert(p.properties.end(), desc.properties.begin(), desc.properties.end());
}

NodeDescriptorBuilder::NodeDescriptorBuilder(ApplicationDescriptorBuilder& app, 
                                             const NodeDescriptor& desc,
                                             const XmlAttributesHelper& attrs) :
    _application(app),
    _descriptor(desc)
{
    _name = attrs("name");
    _descriptor.loadFactor = attrs("load-factor", "");
}

ServerDescriptorBuilder*
NodeDescriptorBuilder::createServer(const XmlAttributesHelper& attrs)
{
    return new ServerDescriptorBuilder(_application.getCommunicator(), attrs);
}

ServerDescriptorBuilder*
NodeDescriptorBuilder::createIceBox(const XmlAttributesHelper& attrs)
{
    return new IceBoxDescriptorBuilder(_application.getCommunicator(), attrs);
}

ServerInstanceDescriptorBuilder*
NodeDescriptorBuilder::createServerInstance(const XmlAttributesHelper& attrs)
{
    return new ServerInstanceDescriptorBuilder(attrs);
}

PropertySetDescriptorBuilder*
NodeDescriptorBuilder::createPropertySet(const XmlAttributesHelper& attrs) const
{
    string id = attrs("id");

    PropertySetDescriptorBuilder* builder = new PropertySetDescriptorBuilder();
    builder->setId(id);
    return builder;
}

void
NodeDescriptorBuilder::addVariable(const XmlAttributesHelper& attrs)
{
    if(!_application.isOverride(attrs("name")))
    {
        _descriptor.variables[attrs("name")] = attrs("value", "");
    }
    else
    {
        attrs.contains("value"); // NOTE: prevents warning about "value" not being used.
    }
}

void
NodeDescriptorBuilder::addServerInstance(const ServerInstanceDescriptor& desc)
{
    _descriptor.serverInstances.push_back(desc);
}

void
NodeDescriptorBuilder::addServer(const ServerDescriptorPtr& server)
{
    _descriptor.servers.push_back(server);
}

void
NodeDescriptorBuilder::addPropertySet(const string& id, const PropertySetDescriptor& desc)
{
    if(!_descriptor.propertySets.insert(make_pair(id, desc)).second)
    {
        throw "duplicate property set `" + id + "'";
    }
}

void
NodeDescriptorBuilder::setDescription(const string& description)
{
    _descriptor.description = description;
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
    if(find(_descriptor.parameters.begin(), _descriptor.parameters.end(), attrs("name")) !=
       _descriptor.parameters.end())
    {
        throw "duplicate parameter `" + attrs("name") + "'"; 
    }

    _descriptor.parameters.push_back(attrs("name"));
    if(attrs.contains("default"))
    {
        _descriptor.parameterDefaults.insert(make_pair(attrs("name"), attrs("default", "")));
    }
}

void
TemplateDescriptorBuilder::setDescriptor(const CommunicatorDescriptorPtr& desc)
{
    _descriptor.descriptor = desc;
}

ServerDescriptorBuilder*
TemplateDescriptorBuilder::createServer(const XmlAttributesHelper& attrs)
{
    if(_serviceTemplate)
    {
        throw "<server> element can't be a child of <service-template>";
    }
    return new ServerDescriptorBuilder(_application.getCommunicator(), attrs);
}

ServerDescriptorBuilder*
TemplateDescriptorBuilder::createIceBox(const XmlAttributesHelper& attrs)
{
    if(_serviceTemplate)
    {
        throw "<icebox> element can't be a child of <service-template>";
    }
    return new IceBoxDescriptorBuilder(_application.getCommunicator(), attrs);
}

ServiceDescriptorBuilder*
TemplateDescriptorBuilder::createService(const XmlAttributesHelper& attrs)
{
    if(!_serviceTemplate)
    {
        throw "<service> element can't be a child of <server-template>";
    }
    return new ServiceDescriptorBuilder(_application.getCommunicator(), attrs);
}

CommunicatorDescriptorBuilder::CommunicatorDescriptorBuilder(const Ice::CommunicatorPtr& communicator) :
    _communicator(communicator)
{
}

void
CommunicatorDescriptorBuilder::init(const CommunicatorDescriptorPtr& desc, const XmlAttributesHelper&)
{
    _descriptor = desc;
}

void
CommunicatorDescriptorBuilder::finish()
{
    //
    // Add the hidden properties at the begining of the communicator
    // properties. These properties are not added directly to the
    // property set because it's not allowed to define properties
    // before references to property sets.
    //
    _descriptor->propertySet.properties.insert(_descriptor->propertySet.properties.begin(),
                                               _hiddenProperties.begin(), _hiddenProperties.end());
}

void
CommunicatorDescriptorBuilder::setDescription(const string& desc)
{
    _descriptor->description = desc;
}

void
CommunicatorDescriptorBuilder::addProperty(const XmlAttributesHelper& attrs)
{
    addProperty(_descriptor->propertySet.properties, attrs("name"), attrs("value", ""));
}

PropertySetDescriptorBuilder*
CommunicatorDescriptorBuilder::createPropertySet() const
{
    return new PropertySetDescriptorBuilder();
}

void
CommunicatorDescriptorBuilder::addPropertySet(const PropertySetDescriptor& desc)
{
    //
    // Allow re-opening of unamed property sets.
    //
    PropertySetDescriptor& p = _descriptor->propertySet;
    p.references.insert(p.references.end(), desc.references.begin(), desc.references.end());
    p.properties.insert(p.properties.end(), desc.properties.begin(), desc.properties.end());
}

void
CommunicatorDescriptorBuilder::addAdapter(const XmlAttributesHelper& attrs)
{
    AdapterDescriptor desc;
    desc.name = attrs("name");
    if(attrs.contains("id"))
    {
        desc.id = attrs("id");
    }
    else
    {
        string fqn = "${server}";
        if(ServiceDescriptorPtr::dynamicCast(_descriptor))
        {
            fqn += ".${service}";
        }
        desc.id = fqn + "." + desc.name;
    }
    desc.replicaGroupId = attrs("replica-group", "");
    desc.priority = attrs("priority", "");
    desc.registerProcess = attrs.asBool("register-process", false);
    if(desc.id == "")
    {
        throw "empty `id' for adapter `" + desc.name + "'";
    }
    desc.serverLifetime = attrs.asBool("server-lifetime", true);
    _descriptor->adapters.push_back(desc);

    addProperty(_hiddenProperties, desc.name + ".Endpoints", attrs("endpoints", "default"));
    if(attrs.contains("proxy-options"))
    {
        addProperty(_hiddenProperties, desc.name + ".ProxyOptions", attrs("proxy-options", ""));
    }
}

void
CommunicatorDescriptorBuilder::setAdapterDescription(const string& value)
{
    _descriptor->adapters.back().description = value;
}

void
CommunicatorDescriptorBuilder::addObject(const XmlAttributesHelper& attrs)
{
    ObjectDescriptor object;
    object.type = attrs("type", "");
    object.id = Ice::stringToIdentity(attrs("identity"));
    object.proxyOptions = attrs("proxy-options", "");
    if(attrs.contains("property"))
    {
        addProperty(_hiddenProperties, attrs("property"), attrs("identity"));
    }
    _descriptor->adapters.back().objects.push_back(object);
}

void
CommunicatorDescriptorBuilder::addAllocatable(const XmlAttributesHelper& attrs)
{
    ObjectDescriptor object;
    object.type = attrs("type", "");
    object.id = Ice::stringToIdentity(attrs("identity"));
    object.proxyOptions = attrs("proxy-options", "");
    if(attrs.contains("property"))
    {
        addProperty(_hiddenProperties, attrs("property"), attrs("identity"));
    }
    _descriptor->adapters.back().allocatables.push_back(object);
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
    if(!_descriptor->dbEnvs.back().dbHome.empty())
    {
        throw "can't add property to the database environment:\n"
              "properties are only allowed if the database\n"
              "environment home directory is managed by the node";
    }

    PropertyDescriptor prop;
    prop.name = attrs("name");
    prop.value = attrs("value", "");
    _descriptor->dbEnvs.back().properties.push_back(prop);
}

void
CommunicatorDescriptorBuilder::setDbEnvDescription(const string& value)
{
    _descriptor->dbEnvs.back().description = value;
}

void
CommunicatorDescriptorBuilder::addLog(const XmlAttributesHelper& attrs)
{
    if(attrs.contains("property"))
    {
        addProperty(_hiddenProperties, attrs("property"), attrs("path"));
    }
    _descriptor->logs.push_back(attrs("path"));
}

void
CommunicatorDescriptorBuilder::addProperty(PropertyDescriptorSeq& properties, const string& name, const string& value)
{
    PropertyDescriptor prop;
    prop.name = name;
    prop.value = value;
    properties.push_back(prop);
}

ServiceInstanceDescriptorBuilder::ServiceInstanceDescriptorBuilder(const XmlAttributesHelper& attrs)
{
    _descriptor._cpp_template = attrs("template");
    _descriptor.parameterValues = attrs.asMap();
    _descriptor.parameterValues.erase("template");
}

PropertySetDescriptorBuilder*
ServiceInstanceDescriptorBuilder::createPropertySet() const
{
    return new PropertySetDescriptorBuilder();
}

void
ServiceInstanceDescriptorBuilder::addPropertySet(const PropertySetDescriptor& desc)
{
    //
    // Allow re-opening of unamed property sets.
    //
    PropertySetDescriptor& p = _descriptor.propertySet;
    p.references.insert(p.references.end(), desc.references.begin(), desc.references.end());
    p.properties.insert(p.properties.end(), desc.properties.begin(), desc.properties.end());
}

ServerDescriptorBuilder::ServerDescriptorBuilder(const Ice::CommunicatorPtr& communicator,
                                                 const XmlAttributesHelper& attrs) :
    CommunicatorDescriptorBuilder(communicator)
{
    init(new ServerDescriptor(), attrs);
}

ServerDescriptorBuilder::ServerDescriptorBuilder(const Ice::CommunicatorPtr& communicator) :
    CommunicatorDescriptorBuilder(communicator)
{
}

void
ServerDescriptorBuilder::init(const ServerDescriptorPtr& desc, const XmlAttributesHelper& attrs)
{
    CommunicatorDescriptorBuilder::init(desc, attrs);
    _descriptor = desc;
    _descriptor->id = attrs("id");
    _descriptor->exe = attrs("exe");
    _descriptor->activationTimeout = attrs("activation-timeout", "");
    _descriptor->deactivationTimeout = attrs("deactivation-timeout", "");
    _descriptor->pwd = attrs("pwd", "");
    _descriptor->activation = attrs("activation", "manual");
    _descriptor->applicationDistrib = attrs.asBool("application-distrib", true);
    _descriptor->allocatable = attrs.asBool("allocatable", false);
    _descriptor->user = attrs("user", "");
    _descriptor->iceVersion = attrs("ice-version", "");
}

ServiceDescriptorBuilder*
ServerDescriptorBuilder::createService(const XmlAttributesHelper& /*attrs*/)
{
    throw "<service> element can only be a child of an <icebox> element";
    return 0;
}

ServiceInstanceDescriptorBuilder*
ServerDescriptorBuilder::createServiceInstance(const XmlAttributesHelper& /*attrs*/)
{
    throw "<service-instance> element can only be a child of an <icebox> element";
    return 0;
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
ServerDescriptorBuilder::addService(const ServiceDescriptorPtr& /*desc*/)
{
    assert(false);
}

void
ServerDescriptorBuilder::addServiceInstance(const ServiceInstanceDescriptor& /*desc*/)
{
    assert(false);
}

void
ServerDescriptorBuilder::addDistribution(const XmlAttributesHelper& attrs)
{
    _descriptor->distrib.icepatch = attrs("icepatch", "${application}.IcePatch2/server");
}

void
ServerDescriptorBuilder::addDistributionDirectory(const string& directory)
{
    _descriptor->distrib.directories.push_back(directory);
}

IceBoxDescriptorBuilder::IceBoxDescriptorBuilder(const Ice::CommunicatorPtr& communicator,
                                                 const XmlAttributesHelper& attrs) :
    ServerDescriptorBuilder(communicator)
{
    init(new IceBoxDescriptor(), attrs);
}

void
IceBoxDescriptorBuilder::init(const IceBoxDescriptorPtr& desc, const XmlAttributesHelper& attrs)
{
    ServerDescriptorBuilder::init(desc, attrs);
    _descriptor = desc;
}

ServiceDescriptorBuilder*
IceBoxDescriptorBuilder::createService(const XmlAttributesHelper& attrs)
{
    return new ServiceDescriptorBuilder(_communicator, attrs);
}

ServiceInstanceDescriptorBuilder*
IceBoxDescriptorBuilder::createServiceInstance(const XmlAttributesHelper& attrs)
{
    return new ServiceInstanceDescriptorBuilder(attrs);
}

void
IceBoxDescriptorBuilder::addAdapter(const XmlAttributesHelper& /*attrs*/)
{
    throw "<adapter> element can't be a child of an <icebox> element";
}

void
IceBoxDescriptorBuilder::addDbEnv(const XmlAttributesHelper& /*attrs*/)
{
    throw "<dbenv> element can't be a child of an <icebox> element";
}

void
IceBoxDescriptorBuilder::addServiceInstance(const ServiceInstanceDescriptor& desc)
{
    _descriptor->services.push_back(desc);
}

void
IceBoxDescriptorBuilder::addService(const ServiceDescriptorPtr& desc)
{
    ServiceInstanceDescriptor instance;
    assert(desc);
    instance.descriptor = desc;
    _descriptor->services.push_back(instance);
}

ServiceDescriptorBuilder::ServiceDescriptorBuilder(const Ice::CommunicatorPtr& communicator,
                                                   const XmlAttributesHelper& attrs) :
    CommunicatorDescriptorBuilder(communicator)
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
