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
#include <IceGrid/Util.h>
#include <IceGrid/AdapterCache.h>
#include <IceGrid/ServerCache.h>

#include <iterator>

using namespace std;
using namespace IceUtil;
using namespace IceGrid;

namespace IceGrid
{

struct GetReplicatedAdapterId : unary_function<ReplicatedAdapterDescriptor&, const string&>
{
    const string&
    operator()(const ReplicatedAdapterDescriptor& desc)
    {
	return desc.id;
    }
};

struct TemplateDescriptorEqual : std::binary_function<TemplateDescriptor&, TemplateDescriptor&, bool>
{
    bool
    operator()(const TemplateDescriptor& lhs, const TemplateDescriptor& rhs)
    {
	if(lhs.parameters != rhs.parameters)
	{
	    return false;
	}

	{
	    IceBoxDescriptorPtr slhs = IceBoxDescriptorPtr::dynamicCast(lhs.descriptor);
	    IceBoxDescriptorPtr srhs = IceBoxDescriptorPtr::dynamicCast(rhs.descriptor);
	    if(slhs && srhs)
	    {
		return IceBoxHelper(slhs) == IceBoxHelper(srhs);
	    }
	}
	{
	    ServerDescriptorPtr slhs = ServerDescriptorPtr::dynamicCast(lhs.descriptor);
	    ServerDescriptorPtr srhs = ServerDescriptorPtr::dynamicCast(rhs.descriptor);
	    if(slhs && srhs)
	    {
		return ServerHelper(slhs) == ServerHelper(srhs);
	    }
	}
	{
	    ServiceDescriptorPtr slhs = ServiceDescriptorPtr::dynamicCast(lhs.descriptor);
	    ServiceDescriptorPtr srhs = ServiceDescriptorPtr::dynamicCast(rhs.descriptor);
	    if(slhs && srhs)
	    {
		return ServiceHelper(slhs) == ServiceHelper(srhs);
	    }   
	}
	return false;
    }
};

template <typename GetKeyFunc, typename Seq> Seq
getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func)
{
    return getSeqUpdatedElts(lseq, rseq, func, equal_to<typename Seq::value_type>());
}

template <typename GetKeyFunc, typename EqFunc, typename Seq> Seq
getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq)
{
    Seq result;
    for(typename Seq::const_iterator p = rseq.begin(); p != rseq.end(); ++p)
    {
	typename Seq::const_iterator q = lseq.begin();
	for(; q != lseq.end(); ++q)
	{
	    if(func(*p) == func(*q))
	    {
		break;
	    }
	}
	if(q == lseq.end() || !eq(*p, *q))
	{
	    result.push_back(*p);
	}
    }
    return result;
}

template <typename GetKeyFunc, typename Seq> Ice::StringSeq 
getSeqRemovedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func)
{
    Ice::StringSeq removed;
    for(typename Seq::const_iterator p = lseq.begin(); p != lseq.end(); ++p)
    {
	typename Seq::const_iterator q;
	for(q = rseq.begin(); q != rseq.end(); ++q)
	{
	    if(func(*p) == func(*q))
	    {
		break;
	    }
	}
	if(q == rseq.end())
	{
	    removed.push_back(func(*p));
	}
    }
    return removed;
}

template <typename GetKeyFunc, typename Seq> Seq
updateSeqElts(const Seq& seq, const Seq& update, const Ice::StringSeq& remove, GetKeyFunc func)
{
    Seq result = update;
    set<string> removed(remove.begin(), remove.end());
    for(typename Seq::const_iterator p = seq.begin(); p != seq.end(); ++p)
    {
	if(removed.find(func(*p)) == removed.end())
	{
	    typename Seq::const_iterator q = update.begin();
	    for(; q != update.end(); ++q)
	    {
		if(func(*p) == func(*q))
		{
		    break;
		}
	    }
	    if(q == update.end())
	    {
		result.push_back(*p);
	    }
	}
    }
    return result;
}

template<typename Dict> Dict
getDictUpdatedElts(const Dict& ldict, const Dict& rdict)
{
    return getDictUpdatedElts(ldict, rdict, equal_to<typename Dict::mapped_type>());
}

template<typename Dict, typename EqFunc> Dict
getDictUpdatedElts(const Dict& ldict, const Dict& rdict, EqFunc eq)
{
    Dict result;
    for(typename Dict::const_iterator p = rdict.begin(); p != rdict.end(); ++p)
    {
	typename Dict::const_iterator q = ldict.find(p->first);
	if(q == ldict.end() || !eq(p->second, q->second))
	{
	    result.insert(*p);
	}
    }
    return result;
}

template <typename Dict> Ice::StringSeq
getDictRemovedElts(const Dict& ldict, const Dict& rdict)
{
    Ice::StringSeq removed;
    for(typename Dict::const_iterator p = ldict.begin(); p != ldict.end(); ++p)
    {
	if(rdict.find(p->first) == rdict.end())
	{
	    removed.push_back(p->first);
	}
    }
    return removed;
}

template <typename Dict> Dict
updateDictElts(const Dict& dict, const Dict& update, const Ice::StringSeq& remove)
{
    Dict result = dict;
    for(Ice::StringSeq::const_iterator p = remove.begin(); p != remove.end(); ++p)
    {
	result.erase(*p);
    }
    for(typename Dict::const_iterator q = update.begin(); q != update.end(); ++q)
    {
	result[q->first] = q->second;
    }
    return result;
}

}

Resolver::Resolver(const ApplicationHelper& app, const string& name, const map<string, string>& variables) : 
    _application(app),
    _context("application `" + name + "'"),
    _variables(variables)
{
    //
    // Add allowed reserved variables (reserved variables can't be
    // overrided, in this implementation an empty reserved variable is
    // considered to be undefined (see getVariable))
    //
    _reserved["application"] = name;
    _reserved["node"] = "";
    _reserved["server"] = "";
    _reserved["service"] = "";

    //
    // Make sure the variables don't override reserved variables.
    //
    for(map<string, string>::const_iterator p = _variables.begin(); p != _variables.end(); ++p)
    {
	if(_reserved.find(p->first) != _reserved.end())
	{
	    exception("invalid variable `" + p->first + "': reserved variable name");
	}
    }
}

Resolver::Resolver(const Resolver& resolve, const map<string, string>& values, bool params) :
    _application(resolve._application),
    _context(resolve._context),
    _variables(resolve._variables),
    _reserved(resolve._reserved)
{
    if(params)
    {
	_parameters = values;
	for(map<string, string>::const_iterator p = _parameters.begin(); p != _parameters.end(); ++p)
	{
	    if(_reserved.find(p->first) != _reserved.end())
	    {
		exception("invalid parameter `" + p->first + ": reserved variable name");
	    }
	}
    }
    else
    {
	for(map<string, string>::const_iterator p = values.begin(); p != values.end(); ++p)
	{
	    //
	    // Make sure the variables don't override reserved variables.
	    //
	    if(_reserved.find(p->first) != _reserved.end())
	    {
		exception("invalid variable `" + p->first + ": reserved variable name");
	    }
	    _variables[p->first] = p->second;
	}
    }
}

string 
Resolver::operator()(const string& value, const string& name, const bool allowEmpty) const
{
    try
    {
	string val = substitute(value, true);
	if(!allowEmpty && val.empty())
	{
	    throw "empty value";
	}
	return val;
    }
    catch(const string& reason)
    {
	exception("invalid value `" + value + "' for " + name + ": " + reason);
    }
    catch(const char* reason)
    {
	exception("invalid value `" + value + "' for " + name + ": " + reason);
    }
    return ""; // To prevent compiler warning.
}

string
Resolver::asInt(const string& value, const string& name) const
{
    string v = operator()(value, name);
    if(!v.empty())
    {
	int val;
	if(!(istringstream(v) >> val))
	{
	    DeploymentException ex;
	    ex.reason = "invalid value `" + value + "' for `" + name + "' in " + _context + ": not an integer";
	    throw ex;
	}
    }
    return v;
}

void
Resolver::setReserved(const string& name, const string& value)
{
    assert(_reserved.find(name) != _reserved.end());
    _reserved[name] = value;
}    

void
Resolver::setContext(const string& context)
{
    try
    {
	_context = substitute(context, true);
    }
    catch(const string& reason)
    {
	exception(reason);
    }
    catch(const char* reason)
    {
	exception(reason);
    }
}

void
Resolver::exception(const string& reason) const
{
    DeploymentException ex;
    ex.reason = "invalid " + _context + ": " + reason;
    throw ex;
}

TemplateDescriptor
Resolver::getServerTemplate(const string& tmpl) const
{
    return _application.getServerTemplate(tmpl);
}

TemplateDescriptor
Resolver::getServiceTemplate(const string& tmpl) const
{
    return _application.getServiceTemplate(tmpl);
}

string
Resolver::substitute(const string& v, bool first) const
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
	    throw "malformed variable name";
	}

	//
	// Get the name of the variable and get its value. If the name
	// refered to a parameter we don't do any recursive
	// substitution: the parameter value is computed at the point
	// of definition.
	//
	string name = value.substr(beg + 2, end - beg - 2);
	bool param;
	string val = getVariable(name, first, param);
	if(!param)
	{
	    val = substitute(val, false); // Recursive resolution
	}
	value.replace(beg, end - beg + 1, val);
	beg += val.length();
    }
    return value;
}

string
Resolver::getVariable(const string& name, bool checkParams, bool& param) const
{
    //
    // We first check the reserved variables, then the parameters if
    // necessary and finally the variables.
    //

    param = false;
    map<string, string>::const_iterator p = _reserved.find(name);
    if(p != _reserved.end())
    {
	if(p->second.empty())
	{
	    throw "undefined variable `" + name + "'";
	}
	return p->second;
    }
    if(checkParams)
    {
	p = _parameters.find(name);
	if(p != _parameters.end())
	{
	    param = true;
	    return p->second;
	}
    }
    p = _variables.find(name);
    if(p != _variables.end())
    {
	return p->second;
    }    

    throw "undefined variable `" + name + "'";
}

CommunicatorHelper::CommunicatorHelper(const CommunicatorDescriptorPtr& desc) : 
    _desc(desc)
{
}

bool 
CommunicatorHelper::operator==(const CommunicatorHelper& helper) const
{
    if(_desc->ice_id() != helper._desc->ice_id())
    {
	return false;
    }

    if(_desc->description != helper._desc->description)
    {
	return false;
    }

    if(set<AdapterDescriptor>(_desc->adapters.begin(), _desc->adapters.end())  != 
       set<AdapterDescriptor>(helper._desc->adapters.begin(), helper._desc->adapters.end()))
    {
	return false;
    }

    if(set<PropertyDescriptor>(_desc->properties.begin(), _desc->properties.end()) != 
       set<PropertyDescriptor>(helper._desc->properties.begin(), helper._desc->properties.end()))
    {
	return false;
    }

    if(set<DbEnvDescriptor>(_desc->dbEnvs.begin(), _desc->dbEnvs.end()) != 
       set<DbEnvDescriptor>(helper._desc->dbEnvs.begin(), helper._desc->dbEnvs.end()))
    {
	return false;
    }

    return true;
}

bool
CommunicatorHelper::operator!=(const CommunicatorHelper& helper) const
{
    return !operator==(helper);
}

void 
CommunicatorHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
    {	
	adapterIds.insert(p->id);
	for(ObjectDescriptorSeq::const_iterator q = p->objects.begin(); q != p->objects.end(); ++q)
	{
	    objectIds.insert(q->id);
	}
    }
}

void
CommunicatorHelper::instantiateImpl(const CommunicatorDescriptorPtr& instance, const Resolver& resolve) const
{
    instance->description = resolve(_desc->description, "description");
    for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
    {
	AdapterDescriptor adapter;
	adapter.name = resolve(p->name, "object adapter name", false);
	adapter.id = resolve(p->id, "object adapter id");
	adapter.registerProcess = p->registerProcess;
	adapter.waitForActivation = p->waitForActivation;
	for(ObjectDescriptorSeq::const_iterator q = p->objects.begin(); q != p->objects.end(); ++q)
	{
	    ObjectDescriptor obj;
	    obj.type = resolve(q->type, "object type");
	    obj.id.name = resolve(q->id.name, "object identity name", false);
	    obj.id.category = resolve(q->id.category, "object identity category");
	    adapter.objects.push_back(obj);
	}
	instance->adapters.push_back(adapter);
    }
    for(PropertyDescriptorSeq::const_iterator p = _desc->properties.begin(); p != _desc->properties.end(); ++p)
    {
	PropertyDescriptor prop;
	prop.name = resolve(p->name, "property name");
	prop.value = resolve(p->value, "property value");
	instance->properties.push_back(prop);
    }
    for(DbEnvDescriptorSeq::const_iterator p = _desc->dbEnvs.begin(); p != _desc->dbEnvs.end(); ++p)
    {
	DbEnvDescriptor dbEnv;
	dbEnv.name = resolve(p->name, "database environment name", false);
	dbEnv.dbHome = resolve(p->dbHome, "database environment home directory");
	for(PropertyDescriptorSeq::const_iterator q = p->properties.begin(); q != p->properties.end(); ++q)
	{
	    PropertyDescriptor prop;
	    prop.name = resolve(q->name, "database environment property name", false);
	    prop.value = resolve(q->value, "database environment property value");
	    dbEnv.properties.push_back(prop);
	}
	instance->dbEnvs.push_back(dbEnv);
    }
}

void
CommunicatorHelper::print(Output& out) const
{
    if(!_desc->description.empty())
    {
	out << nl << "description";
	out << sb;
	out << nl << _desc->description;
	out << eb;
    }
    if(!_desc->properties.empty())
    {
	printProperties(out, _desc->properties);
    }
    {
	for(DbEnvDescriptorSeq::const_iterator p = _desc->dbEnvs.begin(); p != _desc->dbEnvs.end(); ++p)
	{
	    printDbEnv(out, *p);
	}
    }
    {
	for(AdapterDescriptorSeq::const_iterator p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
	{
	    printObjectAdapter(out, *p);
	}
    }

}

void 
CommunicatorHelper::printDbEnv(Output& out, const DbEnvDescriptor& dbEnv) const
{
    out << nl << "database environment '" << dbEnv.name << "'";
    if(!dbEnv.dbHome.empty() || !dbEnv.properties.empty())
    {
	out << sb;
	if(!dbEnv.dbHome.empty())
	{
	    out << nl << "home = '" << dbEnv.dbHome << "'";
	}
	if(!dbEnv.properties.empty())
	{
	    out << nl << "properties";
	    out << sb;
	    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
	    {
		out << nl << p->name << " = '" << p->value << "'";
	    }
	    out << eb;
	}
	out << eb;
    }
}

void
CommunicatorHelper::printObjectAdapter(Output& out, const AdapterDescriptor& adapter) const
{
    out << nl << "adapter '" << adapter.name << "'";
    out << sb;
    out << nl << "id = '" << adapter.id << "'";
// TODO    out << nl << "endpoints = '" << adapter.endpoints << "'";
    out << nl << "register process = '" << (adapter.registerProcess ? "true" : "false") << "'";
    out << nl << "wait for activation = '" << (adapter.waitForActivation ? "true" : "false") << "'";
    for(ObjectDescriptorSeq::const_iterator p = adapter.objects.begin(); p != adapter.objects.end(); ++p)
    {
	out << nl << "object";
	if(!p->type.empty())
	{
	    out << sb;
	    out << nl << "identity = '" << Ice::identityToString(p->id) << "' ";
	    out << nl << "type = '" << p->type << "'";
	    out << eb;
	}
    }
    out << eb;
}

void
CommunicatorHelper::printProperties(Output& out, const PropertyDescriptorSeq& properties) const
{
    out << nl << "properties";
    out << sb;
    for(PropertyDescriptorSeq::const_iterator p = properties.begin(); p != properties.end(); ++p)
    {
	out << nl << p->name << " = '" << p->value << "'";
    }
    out << eb;
}

ServiceHelper::ServiceHelper(const ServiceDescriptorPtr& descriptor) :
    CommunicatorHelper(descriptor),
    _desc(descriptor)
{
    //
    // TODO: Add validation
    //
}

bool 
ServiceHelper::operator==(const ServiceHelper& helper) const
{
    if(!CommunicatorHelper::operator==(helper))
    {
	return false;
    }

    if(_desc->name != helper._desc->name)
    {
	return false;
    }

    if(_desc->entry != helper._desc->entry)
    {
	return false;
    }

    return true;
}

bool
ServiceHelper::operator!=(const ServiceHelper& helper) const
{
    return !operator==(helper);
}

ServiceDescriptorPtr
ServiceHelper::getDescriptor() const
{
    return _desc;
}

ServiceDescriptorPtr
ServiceHelper::instantiate(const Resolver& resolver) const
{
    ServiceDescriptorPtr service = new ServiceDescriptor();
    instantiateImpl(service, resolver);
    return service;
}

void
ServiceHelper::instantiateImpl(const ServiceDescriptorPtr& instance, const Resolver& resolve) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);
    instance->name = resolve(_desc->name, "name", false);
    instance->entry = resolve(_desc->entry, "entry", false);
}

void
ServiceHelper::print(Output& out) const
{
    out << "service `" + _desc->name + "'";
    out << sb;
    out << nl << "entry = '" << _desc->entry << "'";
    CommunicatorHelper::print(out);
    out << eb;
}

ServerHelper::ServerHelper(const ServerDescriptorPtr& descriptor) :
    CommunicatorHelper(descriptor),
    _desc(descriptor)
{
}

bool 
ServerHelper::operator==(const ServerHelper& helper) const
{
    if(!CommunicatorHelper::operator==(helper))
    {
	return false;
    }

    if(_desc->id != helper._desc->id)
    {
	return false;
    }

    if(_desc->exe != helper._desc->exe)
    {
	return false;
    }

    if(_desc->pwd != helper._desc->pwd)
    {
	return false;
    }

    if(_desc->activation != helper._desc->activation)
    {
	return false;
    }

    if(_desc->activationTimeout != helper._desc->activationTimeout)
    {
	return false;
    }

    if(_desc->deactivationTimeout != helper._desc->deactivationTimeout)
    {
	return false;
    }

    if(set<string>(_desc->options.begin(), _desc->options.end()) != 
       set<string>(helper._desc->options.begin(), helper._desc->options.end()))
    {
	return false;
    }

    if(set<string>(_desc->envs.begin(), _desc->envs.end()) != 
       set<string>(helper._desc->envs.begin(), helper._desc->envs.end()))
    {
	return false;
    }

    return true;
}

bool
ServerHelper::operator!=(const ServerHelper& helper) const
{
    return !operator==(helper);
}

void
ServerHelper::instantiateImpl(const ServerDescriptorPtr& instance, const Resolver& resolve) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);

    instance->id = resolve(_desc->id, "id", false);
    instance->exe = resolve(_desc->exe, "executable", false);
    instance->pwd = resolve(_desc->pwd, "working directory path");
    instance->activation = resolve(_desc->activation, "activation");
    if(!instance->activation.empty() && instance->activation != "manual" && instance->activation != "on-demand")
    {
	resolve.exception("unknown activation `" + instance->activation + "'");
    }
    instance->activationTimeout = resolve.asInt(_desc->activationTimeout, "activation timeout");
    instance->deactivationTimeout = resolve.asInt(_desc->deactivationTimeout, "deactivation timeout");
    for(Ice::StringSeq::const_iterator p = _desc->options.begin(); p != _desc->options.end(); ++p)
    {
	instance->options.push_back(resolve(*p, "option"));
    }
    for(Ice::StringSeq::const_iterator p = _desc->envs.begin(); p != _desc->envs.end(); ++p)
    {
	instance->envs.push_back(resolve(*p, "environment variable"));
    }
}

ServerDescriptorPtr
ServerHelper::getDescriptor() const
{
    return _desc;
}

ServerDescriptorPtr
ServerHelper::instantiate(const Resolver& resolver) const
{
    ServerDescriptorPtr server = new ServerDescriptor();
    instantiateImpl(server, resolver);
    return server;
}

void
ServerHelper::print(Output& out, const string& application, const string& node) const
{
    out << "server `" + _desc->id + "'";
    out << sb;
    printImpl(out, application, node);
    out << eb;
}

void
ServerHelper::printImpl(Output& out, const string& application, const string& node) const
{
    if(!application.empty())
    {
	out << nl << "application = '" << application << "'";
    }
    if(!node.empty())
    {
	out << nl << "node = '" << node << "'";
    }
    out << nl << "exe = '" << _desc->exe << "'";
    if(!_desc->pwd.empty())
    {
	out << nl << "pwd = '" << _desc->pwd << "'";
    }
    out << nl << "activation = '" << _desc->activation << "'";
    if(!_desc->activationTimeout.empty() && _desc->activationTimeout != "0")
    {
	out << nl << "activationTimeout = " << _desc->activationTimeout;
    }
    if(!_desc->deactivationTimeout.empty() && _desc->deactivationTimeout != "0")
    {
	out << nl << "deactivationTimeout = " << _desc->deactivationTimeout;
    }
    if(!_desc->options.empty())
    {
	out << nl << "options = '" << toString(_desc->options) << "'";
    }
    if(!_desc->envs.empty())
    {
	out << nl << "envs = '" << toString(_desc->envs) << "'";
    }
    CommunicatorHelper::print(out);
}

IceBoxHelper::IceBoxHelper(const IceBoxDescriptorPtr& descriptor, const Resolver& resolve) :
    ServerHelper(descriptor),
    _desc(descriptor)
{
    //
    // TODO: Add validation (e.g.: ensure that service names are unique.)
    //

    //
    // This IceBoxHelper constructor is called for IceBox server
    // instances. Here, we populate the server helper sequence and
    // also update the IceBox descriptor service instances. The
    // service instances of the descriptor contain instances of the
    // services: the ServiceInstanceDescriptor::descriptor attribute
    // is set with the instance descriptor of the service even for
    // template instances.
    //

    for(ServiceInstanceDescriptorSeq::iterator p = _desc->services.begin(); p != _desc->services.end(); ++p)
    {
	_services.push_back(ServiceInstanceHelper(*p, resolve));
	*p = _services.back().getInstance();
    }
}

IceBoxHelper::IceBoxHelper(const IceBoxDescriptorPtr& descriptor) :
    ServerHelper(descriptor),
    _desc(descriptor)
{
    //
    // TODO: Add validation
    //

    for(ServiceInstanceDescriptorSeq::const_iterator p = _desc->services.begin(); p != _desc->services.end(); ++p)
    {
	_services.push_back(ServiceInstanceHelper(*p));
    }
}

bool 
IceBoxHelper::operator==(const IceBoxHelper& helper) const
{
    if(!ServerHelper::operator==(helper))
    {
	return false;
    }
	
    if(_services != helper._services)
    {
	return false;
    }
    
    return true;
}

bool
IceBoxHelper::operator!=(const IceBoxHelper& helper) const
{
    return !operator==(helper);
}

ServerDescriptorPtr
IceBoxHelper::instantiate(const Resolver& resolver) const
{
    IceBoxDescriptorPtr iceBox = new IceBoxDescriptor();
    instantiateImpl(iceBox, resolver);
    return iceBox;
}

void
IceBoxHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    CommunicatorHelper::getIds(adapterIds, objectIds);
    for(vector<ServiceInstanceHelper>::const_iterator p = _services.begin(); p != _services.end(); ++p)
    {
	p->getIds(adapterIds, objectIds);
    }
}

void
IceBoxHelper::instantiateImpl(const IceBoxDescriptorPtr& instance, const Resolver& resolver) const
{
    ServerHelper::instantiateImpl(instance, resolver);
    for(vector<ServiceInstanceHelper>::const_iterator p = _services.begin(); p != _services.end(); ++p)
    {
	instance->services.push_back(p->getDescriptor());
    }
}

void
IceBoxHelper::print(Output& out, const string& application, const string& node) const
{
    out << "icebox `" + _desc->id + "'";
    out << sb;
// TODO    out << nl << "service manager endpoints = '" << iceBox->endpoints << "'";
    printImpl(out, application, node);
    for(vector<ServiceInstanceHelper>::const_iterator p = _services.begin(); p != _services.end(); ++p)
    {
	out << nl;
	p->print(out);
    }
    out << eb;
}

map<string, string>
InstanceHelper::instantiateParams(const Resolver& resolve, const string& tmpl, const map<string, string>& parameters,
				  const vector<string>& requiredParameters)
{
    map<string, string> params;

    set<string> unknown;
    for(map<string, string>::const_iterator p = parameters.begin(); p != parameters.end(); ++p)
    {
	if(find(requiredParameters.begin(), requiredParameters.end(), p->first) == requiredParameters.end())
	{
	    unknown.insert(p->first);
	}
	params.insert(make_pair(p->first, resolve(p->second, "parameter `" + p->first + "'")));
    }
    if(!unknown.empty())
    {
	ostringstream os;
	os << "template `" + tmpl + "' instance unknown parameters: ";
	copy(unknown.begin(), unknown.end(), ostream_iterator<string>(os, " "));
	throw DeploymentException(os.str());
    }
    
    set<string> missingParams;
    for(vector<string>::const_iterator q = requiredParameters.begin(); q != requiredParameters.end(); ++q)
    {
	if(params.find(*q) == params.end())
	{
	    missingParams.insert(*q);
	}
    }
    if(!missingParams.empty())
    {
	ostringstream os;
	os << "template `" + tmpl + "' instance undefined parameters: ";
	copy(missingParams.begin(), missingParams.end(), ostream_iterator<string>(os, " "));
	throw DeploymentException(os.str());
    }

    return params;
}

ServiceInstanceHelper::ServiceInstanceHelper(const ServiceInstanceDescriptor& desc, const Resolver& resolve) :
    _template(desc._cpp_template), 
    _parameters(desc.parameterValues)
{
    //
    // TODO: Add validation
    //

    ServiceDescriptorPtr def = desc.descriptor;
    map<string, string> params;
    if(!def)
    {
	if(_template.empty())
	{
	    throw DeploymentException("invalid service instance: no template defined");
	}
	TemplateDescriptor tmpl = resolve.getServiceTemplate(_template);
	def = ServiceDescriptorPtr::dynamicCast(tmpl.descriptor);
	params = instantiateParams(resolve, _template, _parameters, tmpl.parameters);
    }
    assert(def);
    _definition = ServiceHelper(def);

    Resolver svcResolve(resolve, params, true);
    svcResolve.setReserved("service", svcResolve(def->name, "service name", false));
    svcResolve.setContext("service `${service}' from server `${server}'");
    _instance = ServiceHelper(_definition.instantiate(svcResolve));
}

ServiceInstanceHelper::ServiceInstanceHelper(const ServiceInstanceDescriptor& desc) :
    _template(desc._cpp_template), 
    _parameters(desc.parameterValues)
{
    if(_template.empty())
    {
	//
	// If the service instance is not a template instance, its
	// descriptor must be set and contain the definition of the
	// service.
	//
	if(!desc.descriptor)
	{
	    throw DeploymentException("invalid service instance: no template defined");
	}
	_definition = ServiceHelper(desc.descriptor);
    }
    else
    {
	//
	// If the service instance is a template instance and its
	// descriptor is set, the descriptor contains the
	// instantiation of the service definition.
	//
	if(desc.descriptor)
	{
	    _instance = ServiceHelper(desc.descriptor);
	}
    }
}

bool
ServiceInstanceHelper::operator==(const ServiceInstanceHelper& helper) const
{
    if(_template.empty())
    {
	return _definition == helper._definition;
    }
    else
    {
	return _template == helper._template && _parameters == helper._parameters;
    }
}

bool
ServiceInstanceHelper::operator!=(const ServiceInstanceHelper& helper) const
{
    return !operator==(helper);
}

ServiceInstanceDescriptor
ServiceInstanceHelper::getDescriptor() const
{
    ServiceInstanceDescriptor desc;
    desc._cpp_template = _template;
    desc.parameterValues = _parameters;
    desc.descriptor = _definition.getDescriptor();
    return desc;
}

ServiceInstanceDescriptor
ServiceInstanceHelper::getInstance() const
{
    assert(_instance.getDescriptor());
    ServiceInstanceDescriptor desc;
    desc._cpp_template = _template;
    desc.parameterValues = _parameters;
    desc.descriptor = _instance.getDescriptor();
    return desc;
}

void
ServiceInstanceHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_instance.getDescriptor());
    _instance.getIds(adapterIds, objectIds);
}

void
ServiceInstanceHelper::print(Output& out) const
{
    if(_instance.getDescriptor())
    {
	_instance.print(out);
    }
    else if(_template.empty())
    {
	_definition.print(out);
    }
    else
    {
	out << nl << "service instance";
	out << sb;
	out << nl << "template = " << _template;
	out << nl << "parameters";
	out << sb;
	for(StringStringDict::const_iterator p = _parameters.begin(); p != _parameters.end(); ++p)
	{
	    out << nl << p->first << " = '" << p->second << "'";
	}
	out << eb;
	out << eb;
    }
}

ServerInstanceHelper::ServerInstanceHelper(const ServerInstanceDescriptor& desc, const Resolver& resolve) :
    _template(desc._cpp_template), _parameters(desc.parameterValues)
{
    //
    // TODO: Add validation
    //

    init(0, resolve);
}

ServerInstanceHelper::ServerInstanceHelper(const ServerDescriptorPtr& definition, const Resolver& resolve)
{
    //
    // TODO: Add validation
    //

    init(definition, resolve);
}

void
ServerInstanceHelper::init(const ServerDescriptorPtr& definition, const Resolver& resolve)
{
    //
    // Get the server definition if it's not set.
    //
    map<string, string> params;
    ServerDescriptorPtr def = definition;
    if(!def)
    {
	if(_template.empty())
	{
	    throw DeploymentException("invalid server instance: template is not defined");
	}
	
	TemplateDescriptor tmpl = resolve.getServerTemplate(_template);
	def = ServerDescriptorPtr::dynamicCast(tmpl.descriptor);
	params = instantiateParams(resolve, _template, _parameters, tmpl.parameters);
    }
    assert(def);
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(def);
    if(iceBox)
    {
	_definition = new IceBoxHelper(iceBox);
    }
    else
    {
	_definition = new ServerHelper(def);
    }

    Resolver svrResolve(resolve, params, true);
    svrResolve.setReserved("server", svrResolve(def->id, "server id", false));
    svrResolve.setContext("server `${server}'");
    if(iceBox)
    {
	_instance = new IceBoxHelper(IceBoxDescriptorPtr::dynamicCast(_definition->instantiate(svrResolve)),
				     svrResolve);
    }
    else
    {
	_instance = new ServerHelper(_definition->instantiate(svrResolve));
    }
}

bool
ServerInstanceHelper::operator==(const ServerInstanceHelper& helper) const
{
    if(_template.empty())
    {
	return *_definition == *helper._definition;
    }
    else
    {
	return _template == helper._template && _parameters == helper._parameters;
    }
}

bool
ServerInstanceHelper::operator!=(const ServerInstanceHelper& helper) const
{
    return !operator==(helper);
}

string
ServerInstanceHelper::getId() const
{
    assert(_instance);
    return _instance->getDescriptor()->id;
}

ServerInstanceDescriptor
ServerInstanceHelper::getDescriptor() const
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = _template;
    desc.parameterValues = _parameters;
    return desc;
}

ServerDescriptorPtr
ServerInstanceHelper::getDefinition() const
{
    return _definition->getDescriptor();
}

ServerDescriptorPtr 
ServerInstanceHelper::getInstance() const
{
    assert(_instance);
    return _instance->getDescriptor();
}

void
ServerInstanceHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_instance);
    _instance->getIds(adapterIds, objectIds);
}

NodeHelper::NodeHelper(const string& name, const NodeDescriptor& descriptor, const Resolver& appResolve) : 
    _name(name),
    _desc(descriptor)
{
    if(_name.empty())
    {
	appResolve.exception("invalid node: empty name");
    }

    Resolver resolve(appResolve, _desc.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");
    for(ServerInstanceDescriptorSeq::const_iterator p = _desc.serverInstances.begin(); 
	p != _desc.serverInstances.end(); ++p)
    {
	ServerInstanceHelper helper(*p, resolve);
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }
    for(ServerDescriptorSeq::const_iterator q = _desc.servers.begin(); q != _desc.servers.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve);
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    validate();
}

bool
NodeHelper::operator==(const NodeHelper& helper) const
{
    if(_desc.variables != helper._desc.variables)
    {
	return false;
    }

    if(_serverInstances != helper._serverInstances)
    {
	return false;
    }

    if(_servers != helper._servers)
    {
	return false;
    }

    return true;
}

bool
NodeHelper::operator!=(const NodeHelper& helper) const
{
    return !operator==(helper);
}

NodeUpdateDescriptor
NodeHelper::diff(const NodeHelper& helper) const
{
    assert(_name == helper._name);

    NodeUpdateDescriptor update;

    update.name = _name;

    update.variables = getDictUpdatedElts(helper._desc.variables, _desc.variables);
    update.removeVariables = getDictRemovedElts(helper._desc.variables, _desc.variables);
    
    ServerInstanceHelperDict updated = getDictUpdatedElts(helper._serverInstances,_serverInstances);
    for(ServerInstanceHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	update.serverInstances.push_back(p->second.getDescriptor());
    }
    update.removeServers = getDictRemovedElts(helper._serverInstances, _serverInstances);

    updated = getDictUpdatedElts(helper._servers, _servers);
    for(ServerInstanceHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	update.servers.push_back(p->second.getDefinition());
    }
    Ice::StringSeq removed = getDictRemovedElts(helper._servers, _servers);
    update.removeServers.insert(update.removeServers.end(), removed.begin(), removed.end());
    return update;
}

void
NodeHelper::update(const NodeUpdateDescriptor& update, const Resolver& appResolve)
{
    assert(update.name == _name);

    //
    // Remove the variables, the servers and server instances.
    //
    Ice::StringSeq::const_iterator r;
    for(r = update.removeVariables.begin(); r != update.removeVariables.end(); ++r)
    {
	_desc.variables.erase(*r);
    }
    for(r = update.removeServers.begin(); r != update.removeServers.end(); ++r)
    {
	_serverInstances.erase(*r);
	_servers.erase(*r);
    }

    for(map<string, string>::const_iterator p = update.variables.begin(); p != update.variables.end(); ++p)
    {
	_desc.variables[p->first] = p->second;
    }

    //
    // NOTE: It's important to create the resolver *after* updating the node variables!
    //
    Resolver resolve(appResolve, _desc.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    //
    // Update the server instances, first we instantiate the server
    // instances from the update, remove the old server instances that
    // were updated, and then we re-instantiate the server instances
    // that were not updated. We also ensure that the re-instantiation
    // isn't changing the id: this is not allowed, instead the old
    // server should be removed first.
    //
    ServerInstanceHelperDict serverInstances;
    serverInstances.swap(_serverInstances);
    for(ServerInstanceDescriptorSeq::const_iterator q = update.serverInstances.begin(); 
	q != update.serverInstances.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve);
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	serverInstances.erase(helper.getId());
    }
    for(ServerInstanceHelperDict::const_iterator q = serverInstances.begin(); q != serverInstances.end(); ++q)
    {
	ServerInstanceHelper helper(q->second.getDescriptor(), resolve); // Re-instantiate the server.
	if(helper.getId() != q->first)
	{
	    throw DeploymentException("invalid update in node `" + _name + "':\n" +
				      "server instance id `" + q->first + "' changed to `" + helper.getId() + "'");
	}
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    //
    // Update the servers, first we instantiate the servers from the
    // update, remove the old servers that were updated, and then we
    // re-instantiate the servers that were not updated. We also
    // ensure that the re-instantiation isn't changing the id: this is
    // not allowed, instead the old server should be removed first.
    //
    ServerInstanceHelperDict servers;
    servers.swap(_servers);
    for(ServerDescriptorSeq::const_iterator s = update.servers.begin(); s != update.servers.end(); ++s)
    {
	ServerInstanceHelper helper(*s, resolve);
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	servers.erase(helper.getId());
    }    
    for(ServerInstanceHelperDict::const_iterator q = servers.begin(); q != servers.end(); ++q)
    {
	ServerInstanceHelper helper(q->second.getDefinition(), resolve); // Re-instantiate the server.
	if(helper.getId() != q->first)
	{
	    throw DeploymentException("invalid update in node `" + _name + "':\n" +
				      "server instance id `" + q->first + "' changed to `" + helper.getId() + "'");
	}	
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    throw DeploymentException("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    //
    // Update the node descriptor with the new server instances and servers.
    //
    _desc.serverInstances.clear();
    for(ServerInstanceHelperDict::const_iterator t = _serverInstances.begin(); t != _serverInstances.end(); ++t)
    {
	_desc.serverInstances.push_back(t->second.getDescriptor());
    }

    _desc.servers.clear();
    for(ServerInstanceHelperDict::const_iterator u = _servers.begin(); u != _servers.end(); ++u)
    {
	_desc.servers.push_back(u->second.getDefinition());
    }
}

void
NodeHelper::getIds(multiset<string>& serverIds, multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    for(ServerInstanceHelperDict::const_iterator p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	serverIds.insert(p->first);
	p->second.getIds(adapterIds, objectIds);
    }
    for(ServerInstanceHelperDict::const_iterator q = _servers.begin(); q != _servers.end(); ++q)
    {
	serverIds.insert(q->first);
	q->second.getIds(adapterIds, objectIds);
    }    
}

const NodeDescriptor&
NodeHelper::getDescriptor() const
{
    return _desc;
}

void
NodeHelper::getServerInfos(const string& application, map<string, ServerInfo>& servers) const
{
    for(ServerInstanceHelperDict::const_iterator p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	ServerInfo info;
	info.node = _name;
	info.application = application;
	info.descriptor = p->second.getInstance();
	servers.insert(make_pair(p->second.getId(), info));
    }
    for(ServerInstanceHelperDict::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
    {
	ServerInfo info;
	info.node = _name;
	info.application = application;
	info.descriptor = p->second.getInstance();
	servers.insert(make_pair(p->second.getId(), info));
    }
}

void
NodeHelper::print(Output& out) const
{
    out << nl << "node '" << _name << "'";
    out << sb;
    if(!_desc.variables.empty())
    {
	out << nl << "variables";
	out << sb;
	for(StringStringDict::const_iterator q = _desc.variables.begin(); q != _desc.variables.end(); ++q)
	{
	    out << nl << q->first << " = '" << q->second << "'";
	}
	out << eb;
    }

    if(_serverInstances.empty() && _servers.empty())
    {
	return;
    }
    
    out << nl << "servers";
    out << sb;
    for(ServerInstanceHelperDict::const_iterator p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	out << nl << p->first;
    }
    for(ServerInstanceHelperDict::const_iterator p = _servers.begin(); p != _servers.end(); ++p)
    {
	out << nl << p->first;
    }
    out << eb;
    out << eb;
}

void
NodeHelper::printDiff(Output& out, const NodeHelper& helper) const
{
    ServerInstanceHelperDict updated1 = getDictUpdatedElts(helper._serverInstances,_serverInstances);
    Ice::StringSeq removed1 = getDictRemovedElts(helper._serverInstances, _serverInstances);
    ServerInstanceHelperDict updated2 = getDictUpdatedElts(helper._servers,_servers);
    Ice::StringSeq removed2 = getDictRemovedElts(helper._servers, _servers);
    
    ServerInstanceHelperDict updated;
    Ice::StringSeq removed;
    updated.insert(updated1.begin(), updated1.end());
    removed.insert(removed.end(), removed1.begin(), removed1.end());
    updated.insert(updated2.begin(), updated2.end());
    removed.insert(removed.end(), removed2.begin(), removed2.end());

    if(updated.empty() && removed.empty())
    {
	return;
    }

    //
    // TODO: Show updated variables?
    //

    out << nl << "node `" + _name + "' updated";
    out << sb;

    out << nl << "servers";
    out << sb;
    for(ServerInstanceHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	if(helper._serverInstances.find(p->first) == helper._serverInstances.end() &&
	   helper._servers.find(p->first) == helper._servers.end())
	{
	    out << nl << "server `" << p->first << "' added";
	}
    }
    for(ServerInstanceHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	if(helper._serverInstances.find(p->first) != helper._serverInstances.end() ||
	   helper._servers.find(p->first) != helper._servers.end())
	{
	    out << nl << "server `" << p->first << "' updated";
	}
    }
    for(Ice::StringSeq::const_iterator p = removed.begin(); p != removed.end(); ++p)
    {
	out << nl << "server `" << *p << "' removed";
    }    
    out << eb;

    out << eb;
}

void
NodeHelper::validate()
{
    //
    // TODO: Add validation
    //    
}

ApplicationHelper::ApplicationHelper(const ApplicationDescriptor& desc) :
    _desc(desc)
{
    if(_desc.name.empty())
    {
	throw "invalid application: empty name";
    }

    Resolver resolve(*this, _desc.name, desc.variables);
    for(NodeDescriptorDict::const_iterator p = _desc.nodes.begin(); p != _desc.nodes.end(); ++p)
    {
	_nodes.insert(make_pair(p->first, NodeHelper(p->first, p->second, resolve)));
    }
    
    validate();
}

ApplicationUpdateDescriptor
ApplicationHelper::diff(const ApplicationHelper& helper)
{
    ApplicationUpdateDescriptor update;

    update.name = _desc.name;
    if(_desc.description != helper._desc.description)
    {
	update.description = new BoxedDescription(_desc.description);
    }

    update.variables = getDictUpdatedElts(helper._desc.variables, _desc.variables);
    update.removeVariables = getDictRemovedElts(helper._desc.variables, _desc.variables);

    GetReplicatedAdapterId rk;
    update.replicatedAdapters = getSeqUpdatedElts(helper._desc.replicatedAdapters, _desc.replicatedAdapters, rk);
    update.removeReplicatedAdapters =getSeqRemovedElts(helper._desc.replicatedAdapters, _desc.replicatedAdapters, rk);

    TemplateDescriptorEqual tmpleq;
    update.serverTemplates = getDictUpdatedElts(helper._desc.serverTemplates, _desc.serverTemplates, tmpleq);
    update.removeServerTemplates = getDictRemovedElts(helper._desc.serverTemplates, _desc.serverTemplates);
    update.serviceTemplates = getDictUpdatedElts(helper._desc.serviceTemplates, _desc.serviceTemplates, tmpleq);
    update.removeServiceTemplates = getDictRemovedElts(helper._desc.serviceTemplates, _desc.serviceTemplates);

    NodeHelperDict updated = getDictUpdatedElts(helper._nodes, _nodes);
    for(NodeHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	NodeHelperDict::const_iterator q = helper._nodes.find(p->first);
	if(q == helper._nodes.end())
	{
	    NodeUpdateDescriptor nodeUpdate;
	    const NodeDescriptor& node = p->second.getDescriptor();
	    nodeUpdate.name = p->first;
	    nodeUpdate.variables = node.variables;
	    nodeUpdate.servers = node.servers;
	    nodeUpdate.serverInstances = node.serverInstances;
	    update.nodes.push_back(nodeUpdate);
	}
	else
	{
	    update.nodes.push_back(p->second.diff(q->second));
	}
    }
    update.removeNodes = getDictRemovedElts(helper._nodes, _nodes);

    return update;
    
}

void
ApplicationHelper::update(const ApplicationUpdateDescriptor& update)
{
    if(update.description)
    {
	_desc.description = update.description->value;
    }

    _desc.replicatedAdapters = updateSeqElts(_desc.replicatedAdapters, update.replicatedAdapters,
					     update.removeReplicatedAdapters, GetReplicatedAdapterId());
    
    _desc.variables = updateDictElts(_desc.variables, update.variables, update.removeVariables);

    _desc.serverTemplates = updateDictElts(_desc.serverTemplates, update.serverTemplates, 
					   update.removeServerTemplates);

    _desc.serviceTemplates = updateDictElts(_desc.serviceTemplates, update.serviceTemplates, 
					    update.removeServiceTemplates);


    for(Ice::StringSeq::const_iterator p = update.removeNodes.begin(); p != update.removeNodes.end(); ++p)
    {
	_nodes.erase(*p);
    }

    //
    // NOTE: It's important to create the resolver *after* updating the application variables!
    //
    Resolver resolve(*this, _desc.name, _desc.variables);

    //
    // We first update or add the nodes from the update descriptor and
    // then we re-instantiate the nodes which were not updated to make
    // sure that their server instances are up to date.
    //
    NodeHelperDict nodes;
    nodes.swap(_nodes);
    for(NodeUpdateDescriptorSeq::const_iterator p = update.nodes.begin(); p != update.nodes.end(); ++p)
    {
	NodeHelperDict::iterator q = nodes.find(p->name);
	if(q == nodes.end())
	{
	    NodeDescriptor desc;
	    desc.variables = p->variables;
	    desc.servers = p->servers;
	    desc.serverInstances = p->serverInstances;
	    _nodes.insert(make_pair(p->name, NodeHelper(p->name, desc, resolve)));
	}
	else
	{
	    q->second.update(*p, resolve);
	    _nodes.insert(make_pair(p->name, q->second));
	    nodes.erase(q);
	}
    }
    for(NodeHelperDict::const_iterator p = nodes.begin(); p != nodes.end(); ++p)
    {
	NodeHelper helper(p->first, p->second.getDescriptor(), resolve); // Re-instantiate the node.
	_nodes.insert(make_pair(p->first, helper));
    }

    //
    // Update the application desriptor.
    //
    _desc.nodes.clear();
    for(NodeHelperDict::const_iterator t = _nodes.begin(); t != _nodes.end(); ++t)
    {
	_desc.nodes.insert(make_pair(t->first, t->second.getDescriptor()));
    }

    //
    // Validate the new application descriptor.
    //
    validate();
}

void
ApplicationHelper::getIds(set<string>& serverIds, set<string>& adapterIds, set<Ice::Identity>& objectIds) const
{
    multiset<string> sIds;
    multiset<string> aIds;
    multiset<Ice::Identity> oIds;
    
    for(NodeHelperDict::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
    {
	p->second.getIds(sIds, aIds, oIds);
    }

    for(ReplicatedAdapterDescriptorSeq::const_iterator r = _desc.replicatedAdapters.begin();
	r != _desc.replicatedAdapters.end(); ++r)
    {
	aIds.insert(r->id);
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    oIds.insert(o->id);
	}
    }

    copy(sIds.begin(), sIds.end(), inserter(serverIds, serverIds.begin()));
    copy(aIds.begin(), aIds.end(), inserter(adapterIds, adapterIds.begin()));
    copy(oIds.begin(), oIds.end(), inserter(objectIds, objectIds.begin()));
}

const ApplicationDescriptor&
ApplicationHelper::getDescriptor() const
{
    return _desc;
}

TemplateDescriptor 
ApplicationHelper::getServerTemplate(const string& name) const
{
    TemplateDescriptorDict::const_iterator p = _desc.serverTemplates.find(name);
    if(p == _desc.serverTemplates.end())
    {
	throw DeploymentException("unknown server template `" + name + "'");
    }
    return p->second;
}

TemplateDescriptor 
ApplicationHelper::getServiceTemplate(const string& name) const
{
    TemplateDescriptorDict::const_iterator p = _desc.serviceTemplates.find(name);
    if(p == _desc.serviceTemplates.end())
    {
	throw DeploymentException("unknown service template `" + name + "'");
    }
    return p->second;
}

map<string, ServerInfo>
ApplicationHelper::getServerInfos() const
{
    map<string, ServerInfo> servers;
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	n->second.getServerInfos(_desc.name, servers);
    }
    return servers;
}

void
ApplicationHelper::print(Output& out) const
{
    out << "application '" << _desc.name << "'";
    out << sb;
    if(!_desc.description.empty())
    {
	out << nl << "description = " << _desc.description;
    }
    if(!_desc.variables.empty())
    {
	out << nl << "variables";
	out << sb;
	for(StringStringDict::const_iterator p = _desc.variables.begin(); p != _desc.variables.end(); 
	    ++p)
	{
	    out << nl << p->first << " = '" << p->second << "'";
	}
	out << eb;
    }
    if(!_desc.replicatedAdapters.empty())
    {
	out << nl << "replicated adapters";
	out << sb;
	for(ReplicatedAdapterDescriptorSeq::const_iterator p = _desc.replicatedAdapters.begin(); 
	    p != _desc.replicatedAdapters.end(); ++p)
	{
	    out << nl << "id = `" << p->id << "' load balancing = '";
	    if(RandomLoadBalancingPolicyPtr::dynamicCast(p->loadBalancing))
	    {
		out << "random";
	    }
	    else if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(p->loadBalancing))
	    {
		out << "round-robin";
	    }
	    else
	    {
		out << "<unknown load balancing policy>";
	    }
	    out << "'";
	}
	out << eb;
    }
    if(!_desc.serverTemplates.empty())
    {
	out << nl << "server templates";
	out << sb;
	for(TemplateDescriptorDict::const_iterator p = _desc.serverTemplates.begin();
	    p != _desc.serverTemplates.end(); ++p)
	{
	    out << nl << p->first;
	}
	out << eb;
    }
    if(!_desc.serviceTemplates.empty())
    {
	out << nl << "service templates";
	out << sb;
	for(TemplateDescriptorDict::const_iterator p = _desc.serviceTemplates.begin();
	    p != _desc.serviceTemplates.end(); ++p)
	{
	    out << nl << p->first;
	}
	out << eb;
    }
    if(!_nodes.empty())
    {
	for(NodeHelperDict::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
	{
	    p->second.print(out);
	}
    }
    out << eb;
}

void
ApplicationHelper::printDiff(Output& out, const ApplicationHelper& helper) const
{
    out << "application `" << _desc.name << "'";
    out << sb;

    //
    // TODO: Show updated variables?
    //

    {
	TemplateDescriptorEqual eq;
	TemplateDescriptorDict updated = getDictUpdatedElts(helper._desc.serverTemplates, _desc.serverTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._desc.serverTemplates, _desc.serverTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "server templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._desc.serverTemplates.find(p->first) == helper._desc.serverTemplates.end())
		{
		    out << nl << "server template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._desc.serverTemplates.find(p->first) != helper._desc.serverTemplates.end())
		{
		    out << nl << "server template `" << p->first << "' updated";
		}
	    }
	    for(Ice::StringSeq::const_iterator p = removed.begin(); p != removed.end(); ++p)
	    {
		out << nl << "server template `" << *p << "' removed";
	    }
	    out << eb;
	}
    }
    {
	TemplateDescriptorEqual eq;
	TemplateDescriptorDict updated = getDictUpdatedElts(helper._desc.serviceTemplates, _desc.serviceTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._desc.serviceTemplates, _desc.serviceTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "service templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._desc.serviceTemplates.find(p->first) == helper._desc.serviceTemplates.end())
		{
		    out << nl << "service template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._desc.serviceTemplates.find(p->first) != helper._desc.serviceTemplates.end())
		{
		    out << nl << "service template `" << p->first << "' updated";
		}
	    }
	    for(Ice::StringSeq::const_iterator p = removed.begin(); p != removed.end(); ++p)
	    {
		out << nl << "service template `" << *p << "' removed";
	    }
	    out << eb;
	}
    }
    {
	NodeHelperDict updated = getDictUpdatedElts(helper._nodes, _nodes);
	Ice::StringSeq removed = getDictRemovedElts(helper._nodes, _nodes);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "nodes";
	    out << sb;
	    for(NodeHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		NodeHelperDict::const_iterator q = helper._nodes.find(p->first);
		if(q == helper._nodes.end())
		{
		    p->second.print(out);
		}
	    }
	    for(NodeHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		NodeHelperDict::const_iterator q = helper._nodes.find(p->first);
		if(q != helper._nodes.end())
		{
		    p->second.printDiff(out, q->second);
		}
	    }
	    for(Ice::StringSeq::const_iterator p = removed.begin(); p != removed.end(); ++p)
	    {
		out << nl << "node `" << *p << "' removed";
	    }
	    out << eb;
	}
    }
    out << eb;
}

void
ApplicationHelper::validate()
{

    //
    // TODO: Add more validation
    //

    //
    // Ensure the unicity of object ids, adapter ids and server ids.
    //    
    multiset<string> serverIds;
    multiset<string> adapterIds;
    multiset<Ice::Identity> objectIds;
    set<string> replicatedAdapterIds;
    for(ReplicatedAdapterDescriptorSeq::const_iterator r = _desc.replicatedAdapters.begin();
	r != _desc.replicatedAdapters.end(); ++r)
    {
	if(!replicatedAdapterIds.insert(r->id).second)
	{
	    throw DeploymentException("invalid application `" + _desc.name + "': duplicate replicated adapter `" + 
				      r->id + "'");
	}
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    objectIds.insert(o->id);
	}
    }

    for(NodeHelperDict::const_iterator p = _nodes.begin(); p != _nodes.end(); ++p)
    {
	p->second.getIds(serverIds, adapterIds, objectIds);
    }

    for(multiset<string>::const_iterator p = serverIds.begin(); p != serverIds.end(); ++p)
    {
	if(serverIds.count(*p) > 1)
	{
	    throw DeploymentException("invalid application `" + _desc.name + "': duplicate server `" + *p + "'");
	}
    }
    for(multiset<string>::const_iterator p = adapterIds.begin(); p != adapterIds.end(); ++p)
    {
	if(adapterIds.count(*p) > 1 && replicatedAdapterIds.find(*p) == replicatedAdapterIds.end())
	{
	    throw DeploymentException("invalid application `" + _desc.name + "': duplicate adapter `" + *p + "'");
	}
    }
    for(multiset<Ice::Identity>::const_iterator p = objectIds.begin(); p != objectIds.end(); ++p)
    {
	if(objectIds.count(*p) > 1)
	{
	    throw DeploymentException("invalid application `" + _desc.name + "': duplicate object `" + 
				      Ice::identityToString(*p) + "'");
	}
    }
}
