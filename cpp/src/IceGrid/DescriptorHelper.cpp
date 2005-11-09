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

#include <iterator>

using namespace std;
using namespace IceUtil;
using namespace IceGrid;

namespace IceGrid
{

struct GetReplicaGroupId : unary_function<ReplicaGroupDescriptor&, const string&>
{
    const string&
    operator()(const ReplicaGroupDescriptor& desc)
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

	if(lhs.parameterDefaults != rhs.parameterDefaults)
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

struct ReplicaGroupEq : std::binary_function<ReplicaGroupDescriptor&, ReplicaGroupDescriptor&, bool>
{
    bool
    operator()(const ReplicaGroupDescriptor& lhs, const ReplicaGroupDescriptor& rhs)
    {
	if(lhs.id != rhs.id)
	{
	    return false;
	}
	if(set<ObjectDescriptor>(lhs.objects.begin(), lhs.objects.end()) != 
	   set<ObjectDescriptor>(rhs.objects.begin(), rhs.objects.end()))
	{
	    return false;
	}
	if(lhs.loadBalancing && rhs.loadBalancing)
	{
	    if(lhs.loadBalancing->ice_id() != rhs.loadBalancing->ice_id())
	    {
		return false;
	    }
	    if(lhs.loadBalancing->nReplicas != rhs.loadBalancing->nReplicas)
	    {
		return false;
	    }
	    AdaptiveLoadBalancingPolicyPtr alhs = AdaptiveLoadBalancingPolicyPtr::dynamicCast(lhs.loadBalancing);
	    AdaptiveLoadBalancingPolicyPtr arhs = AdaptiveLoadBalancingPolicyPtr::dynamicCast(rhs.loadBalancing);
	    if(alhs && arhs && alhs->loadSample != arhs->loadSample)
	    {
		return false;
	    }
	}
	else if(lhs.loadBalancing || rhs.loadBalancing)
	{
	    return false;
	}

	return true;
    }
};

template <typename GetKeyFunc, typename Seq> Seq
getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func)
{
#if defined(_MSC_VER) && (_MSC_VER < 1300)
   return getSeqUpdatedEltsWithEq(lseq, rseq, func, equal_to<Seq::value_type>());
#else
   return getSeqUpdatedEltsWithEq(lseq, rseq, func, equal_to<typename Seq::value_type>());
#endif
}

template <typename GetKeyFunc, typename EqFunc, typename Seq> Seq
getSeqUpdatedEltsWithEq(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq)
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
#if defined(_MSC_VER) && (_MSC_VER < 1300)
    return getDictUpdatedEltsWithEq(ldict, rdict, equal_to<Dict::mapped_type>());
#else
    return getDictUpdatedEltsWithEq(ldict, rdict, equal_to<typename Dict::mapped_type>());
#endif
}

template<typename Dict, typename EqFunc> Dict
getDictUpdatedEltsWithEq(const Dict& ldict, const Dict& rdict, EqFunc eq)
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
    _application(&app),
    _escape(false),
    _context("application `" + name + "'"),
    _variables(variables),
    _reserved(getReserved())
{
    //
    // Make sure the variables don't override reserved variables.
    //
    checkReserved("variable", _variables);
    setReserved("application", name);

    //
    // Some reserved variables which are ignored for now and will be
    // substituted later.
    //
    _ignore.insert("node.os");
    _ignore.insert("node.hostname");
    _ignore.insert("node.release");
    _ignore.insert("node.version");
    _ignore.insert("node.machine");
    _ignore.insert("node.datadir");
}

Resolver::Resolver(const Resolver& resolve, const map<string, string>& values, bool params) :
    _application(resolve._application),
    _escape(resolve._escape),
    _context(resolve._context),
    _variables(params ? resolve._variables : values),
    _parameters(!params ? resolve._parameters : values),
    _reserved(resolve._reserved),
    _ignore(resolve._ignore)
{
    if(params)
    {
	checkReserved("parameter", values);
    }
    else
    {
	_variables.insert(resolve._variables.begin(), resolve._variables.end());
	checkReserved("variable", values);
    }
}

Resolver::Resolver(const string& context, const map<string, string>& values) :
    _application(0),
    _escape(true),
    _context(context),
    _variables(values),
    _reserved(getReserved())
{
    checkReserved("variable", values);
}

string 
Resolver::operator()(const string& value, const string& name, bool allowEmpty, bool useParams) const
{
    try
    {
	string val = substitute(value, useParams);
	if(!allowEmpty && val.empty())
	{
	    throw "empty value";
	}
	return val;
    }
    catch(const string& reason)
    {
	exception("invalid value `" + value + "' for `" + name + "': " + reason);
    }
    catch(const char* reason)
    {
	exception("invalid value `" + value + "' for `" + name + "': " + reason);
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
    throw DeploymentException(_context + ": " + reason);
}

TemplateDescriptor
Resolver::getServerTemplate(const string& tmpl) const
{
    assert(_application);
    return _application->getServerTemplate(tmpl);
}

TemplateDescriptor
Resolver::getServiceTemplate(const string& tmpl) const
{
    assert(_application);
    return _application->getServiceTemplate(tmpl);
}

bool
Resolver::hasReplicaGroup(const string& id) const
{
    if(!_application)
    {
	return true; // If we don't know the application descrpitor we
		     // assume that the replica group exists.
    }
    ReplicaGroupDescriptorSeq::const_iterator p;
    const ApplicationDescriptor& app = _application->getDescriptor();
    for(p = app.replicaGroups.begin(); p != app.replicaGroups.end(); ++p)
    {
	if(p->id == id)
	{
	    return true;
	}
    } 
    return false;
}

string
Resolver::substitute(const string& v, bool useParams) const
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
	    
	    if((beg - escape) % 2)
	    {
		if(_escape)
		{
		    value.replace(escape, beg - escape, (beg - escape) / 2, '$');		    
		}
		++beg;
		continue;
	    }
	    else
	    {
		value.replace(escape, beg - escape, (beg - escape) / 2, '$');
		beg -= (beg - escape) / 2;
	    }
	}

	end = value.find("}", beg);
	if(end == string::npos)
	{
	    throw "malformed variable name";
	}

	//
	// Get the name of the variable and get its value if the
	// variable is not current ignored (in which case we do
	// nothing, the variable will be substituted later). If the
	// name refered to a parameter we don't do any recursive
	// substitution: the parameter value is computed at the point
	// of definition.
	//
	string name = value.substr(beg + 2, end - beg - 2);
	if(_ignore.find(name) != _ignore.end())
	{
	    ++beg;
	    continue;
	}
	bool param;
	string val = getVariable(name, useParams, param);
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

map<string, string>
Resolver::getReserved()
{
    //
    // Allowed reserved variables (reserved variables can't be
    // overrided, in this implementation an empty reserved variable is
    // considered to be undefined (see getVariable))
    //
    map<string, string> reserved;
    reserved["application"] = "";
    reserved["node"] = "";
    reserved["node.os"] = "";
    reserved["node.hostname"] = "";
    reserved["node.release"] = "";
    reserved["node.version"] = "";
    reserved["node.machine"] = "";
    reserved["node.datadir"] = "";
    reserved["application.distrib"] = "${node.datadir}/distrib/${application}";
    reserved["server.distrib"] = "${node.datadir}/servers/${server}/distrib";
    reserved["server"] = "";
    reserved["service"] = "";
    return reserved;
}

void
Resolver::checkReserved(const string& type, const map<string, string>& values) const
{
    for(map<string, string>::const_iterator p = values.begin(); p != values.end(); ++p)
    {
	if(_reserved.find(p->first) != _reserved.end())
	{
	    exception("invalid " + type + " `" + p->first + "': reserved variable name");
	}
    }
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
	if(!p->id.empty())
	{
	    adapterIds.insert(p->id);
	}
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
	adapter.description = resolve(p->description, "object adapter description");
	adapter.id = resolve(p->id, "object adapter id");
	adapter.registerProcess = p->registerProcess;
	adapter.waitForActivation = p->waitForActivation;
	adapter.replicaGroupId = resolve(p->replicaGroupId, "object adapter replica group id");
	if(!adapter.replicaGroupId.empty() && !resolve.hasReplicaGroup(adapter.replicaGroupId))
	{
	    resolve.exception("unknown replica group `" + adapter.replicaGroupId + "'");
	}
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
    for(PropertyDescriptorSeq::const_iterator r = _desc->properties.begin(); r != _desc->properties.end(); ++r)
    {
	PropertyDescriptor prop;
	prop.name = resolve(r->name, "property name");
	prop.value = resolve(r->value, "property value");
	instance->properties.push_back(prop);
    }
    for(DbEnvDescriptorSeq::const_iterator s = _desc->dbEnvs.begin(); s != _desc->dbEnvs.end(); ++s)
    {
	DbEnvDescriptor dbEnv;
	dbEnv.name = resolve(s->name, "database environment name", false);
	dbEnv.description = resolve(s->description, "database environment description");
	dbEnv.dbHome = resolve(s->dbHome, "database environment home directory");
	for(PropertyDescriptorSeq::const_iterator q = s->properties.begin(); q != s->properties.end(); ++q)
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
	    out << nl << "home = `" << dbEnv.dbHome << "'";
	}
	if(!dbEnv.description.empty())
	{
	    out << nl << "description = `" << dbEnv.description << "'";
	}
	if(!dbEnv.properties.empty())
	{
	    out << nl << "properties";
	    out << sb;
	    for(PropertyDescriptorSeq::const_iterator p = dbEnv.properties.begin(); p != dbEnv.properties.end(); ++p)
	    {
		out << nl << p->name << " = `" << p->value << "'";
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
    out << nl << "id = `" << adapter.id << "'";
    out << nl << "replica group id = `" << adapter.replicaGroupId << "'";
    out << nl << "endpoints = `" << getProperty(adapter.name + ".Endpoints") << "'";
    out << nl << "register process = `" << (adapter.registerProcess ? "true" : "false") << "'";
    out << nl << "wait for activation = `" << (adapter.waitForActivation ? "true" : "false") << "'";
    for(ObjectDescriptorSeq::const_iterator p = adapter.objects.begin(); p != adapter.objects.end(); ++p)
    {
	out << nl << "object";
	if(!p->type.empty())
	{
	    out << sb;
	    out << nl << "identity = `" << Ice::identityToString(p->id) << "' ";
	    out << nl << "type = `" << p->type << "'";
	    out << eb;
	}
    }
    if(!adapter.description.empty())
    {
	out << nl << "description = `" << adapter.description << "'";
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
	out << nl << p->name << " = `" << p->value << "'";
    }
    out << eb;
}

string
CommunicatorHelper::getProperty(const string& name) const
{
    return IceGrid::getProperty(_desc->properties, name);
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
    out << nl << "entry = `" << _desc->entry << "'";
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

    if(_desc->distrib != helper._desc->distrib)
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
    instance->applicationDistrib = _desc->applicationDistrib;
    if(!instance->activation.empty() && instance->activation != "manual" && instance->activation != "on-demand")
    {
	resolve.exception("unknown activation `" + instance->activation + "'");
    }
    instance->activationTimeout = resolve.asInt(_desc->activationTimeout, "activation timeout");
    instance->deactivationTimeout = resolve.asInt(_desc->deactivationTimeout, "deactivation timeout");
    Ice::StringSeq::const_iterator p;
    for(p = _desc->options.begin(); p != _desc->options.end(); ++p)
    {
	instance->options.push_back(resolve(*p, "option"));
    }
    for(p = _desc->envs.begin(); p != _desc->envs.end(); ++p)
    {
	instance->envs.push_back(resolve(*p, "environment variable"));
    }
    instance->distrib.icepatch = resolve(_desc->distrib.icepatch, "IcePatch2 server proxy");
    for(p = _desc->distrib.directories.begin(); p != _desc->distrib.directories.end(); ++p)
    {
	instance->distrib.directories.push_back(resolve(*p, "distribution source directory"));
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
	out << nl << "application = `" << application << "'";
    }
    if(!node.empty())
    {
	out << nl << "node = `" << node << "'";
    }
    out << nl << "exe = `" << _desc->exe << "'";
    if(!_desc->pwd.empty())
    {
	out << nl << "pwd = `" << _desc->pwd << "'";
    }
    out << nl << "activation = `" << _desc->activation << "'";
    if(!_desc->activationTimeout.empty() && _desc->activationTimeout != "0")
    {
	out << nl << "activationTimeout = " << _desc->activationTimeout;
    }
    if(!_desc->deactivationTimeout.empty() && _desc->deactivationTimeout != "0")
    {
	out << nl << "deactivationTimeout = " << _desc->deactivationTimeout;
    }
    if(!_desc->applicationDistrib)
    {
	out << nl << "application distribution = false";
    }
    if(!_desc->options.empty())
    {
	out << nl << "options = `" << toString(_desc->options) << "'";
    }
    if(!_desc->envs.empty())
    {
	out << nl << "envs = `" << toString(_desc->envs) << "'";
    }
    if(!_desc->distrib.icepatch.empty())
    {
	out << nl << "distribution";
	out << sb;
	out << nl << "proxy = `" << _desc->distrib.icepatch << "'";
	out << nl << "directories = " << toString(_desc->distrib.directories);
	out << eb;
    }
    CommunicatorHelper::print(out);
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
	instance->services.push_back(p->instantiate(resolver));
    }
}

void
IceBoxHelper::print(Output& out, const string& application, const string& node) const
{
    out << "icebox `" + _desc->id + "'";
    out << sb;
    out << nl << "service manager endpoints = `" << getProperty("IceBox.ServiceManager.Endpoints") << "'";
    printImpl(out, application, node);
    for(vector<ServiceInstanceHelper>::const_iterator p = _services.begin(); p != _services.end(); ++p)
    {
	out << nl;
	p->print(out);
    }
    out << eb;
}

map<string, string>
InstanceHelper::instantiateParams(const Resolver& resolve, 
				  const string& tmpl, 
				  const map<string, string>& parameters,
				  const vector<string>& requiredParameters,
				  const map<string, string>& defaults) const
{
    map<string, string> params;

    set<string> required(requiredParameters.begin(), requiredParameters.end());
    set<string> unknown;
    for(map<string, string>::const_iterator p = parameters.begin(); p != parameters.end(); ++p)
    {
	if(required.find(p->first) == required.end())
	{
	    unknown.insert(p->first);
	}
	params.insert(make_pair(p->first, resolve(p->second, "parameter `" + p->first + "'")));
    }
    if(!unknown.empty())
    {
	ostringstream os;
	os << "unknown parameters when instantiating `" + tmpl + "' template: ";
	copy(unknown.begin(), unknown.end(), ostream_iterator<string>(os, " "));
	resolve.exception(os.str());
    }
    
    set<string> missingParams;
    for(set<string>::const_iterator q = required.begin(); q != required.end(); ++q)
    {
	if(params.find(*q) == params.end())
	{
	    map<string, string>::const_iterator r = defaults.find(*q);
	    if(r == defaults.end())
	    {
		missingParams.insert(*q);
	    }
	    else
	    {
		params.insert(make_pair(r->first, resolve(r->second, "default parameter `" + r->first + "'")));
	    }
	}
    }
    if(!missingParams.empty())
    {
	ostringstream os;
	os << "undefined parameters when instantiating `" + tmpl + "' template: ";
	copy(missingParams.begin(), missingParams.end(), ostream_iterator<string>(os, " "));
	resolve.exception(os.str());
    }

    return params;
}

ServiceInstanceHelper::ServiceInstanceHelper(const ServiceInstanceDescriptor& desc) :
    _template(desc._cpp_template), 
    _parameters(desc.parameterValues)
{
    //
    // If the service instance is not a template instance, its
    // descriptor must be set and contain the definition of the
    // service.
    //
    if(_template.empty() && !desc.descriptor)
    {
	throw DeploymentException("invalid service instance: no template defined");
    }
    if(desc.descriptor)
    {
	_service = ServiceHelper(desc.descriptor);
    }
}

bool
ServiceInstanceHelper::operator==(const ServiceInstanceHelper& helper) const
{
    if(_template.empty())
    {
	return _service == helper._service;
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
ServiceInstanceHelper::instantiate(const Resolver& resolve) const
{ 
    ServiceHelper def = _service;
    map<string, string> params;
    if(!def.getDescriptor())
    {
	assert(!_template.empty());
	TemplateDescriptor tmpl = resolve.getServiceTemplate(_template);
	def = ServiceHelper(ServiceDescriptorPtr::dynamicCast(tmpl.descriptor));
	params = instantiateParams(resolve, _template, _parameters, tmpl.parameters, tmpl.parameterDefaults);
    }

    Resolver svcResolve(resolve, params, !_service.getDescriptor());
    svcResolve.setReserved("service", svcResolve(def.getDescriptor()->name, "service name", false));
    svcResolve.setContext("service `${service}' from server `${server}'");
    
    ServiceInstanceDescriptor desc;
    desc.descriptor = def.instantiate(svcResolve);
    //
    // NOTE: We can't keep the following attributes in the service
    // instance otherwise the instance comparison would be based on
    // the template + parameters which would be wrong (if the template
    // changed the instance also changed.)
    //
    //desc._cpp_template = _template;
    //desc.parameterValues = _parameters;
    return desc;
}

void
ServiceInstanceHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_service.getDescriptor());
    _service.getIds(adapterIds, objectIds);
}

void
ServiceInstanceHelper::print(Output& out) const
{
    if(_service.getDescriptor())
    {
	_service.print(out);
    }
    else
    {
	assert(!_template.empty());
	out << nl << "service instance";
	out << sb;
	out << nl << "template = " << _template;
	out << nl << "parameters";
	out << sb;
	for(StringStringDict::const_iterator p = _parameters.begin(); p != _parameters.end(); ++p)
	{
	    out << nl << p->first << " = `" << p->second << "'";
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
    ServerDescriptorPtr def = definition;
    if(!def)
    {
	if(_template.empty())
	{
	    resolve.exception("invalid server instance: template is not defined");
	}
	
	TemplateDescriptor tmpl = resolve.getServerTemplate(_template);
	def = ServerDescriptorPtr::dynamicCast(tmpl.descriptor);
	_instanceParams = instantiateParams(resolve, _template, _parameters, tmpl.parameters, tmpl.parameterDefaults);
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

    Resolver svrResolve(resolve, _instanceParams, true);
    svrResolve.setReserved("server", svrResolve(def->id, "server id", false));
    svrResolve.setContext("server `${server}'");
    if(iceBox)
    {
	_instance = new IceBoxHelper(IceBoxDescriptorPtr::dynamicCast(_definition->instantiate(svrResolve)));
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
ServerInstanceHelper::getDefinition() const
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = _template;
    desc.parameterValues = _parameters;
    return desc;
}

ServerInstanceDescriptor
ServerInstanceHelper::getInstance() const
{
    ServerInstanceDescriptor desc;
    desc._cpp_template = _template;
    desc.parameterValues = _instanceParams;
    return desc;
}

ServerDescriptorPtr
ServerInstanceHelper::getServerDefinition() const
{
    return _definition->getDescriptor();
}

ServerDescriptorPtr 
ServerInstanceHelper::getServerInstance() const
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
    _definition(descriptor)
{
    if(_name.empty())
    {
	appResolve.exception("invalid node: empty name");
    }

    Resolver resolve(appResolve, _definition.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    ServerInstanceDescriptorSeq::const_iterator p;
    for(p = _definition.serverInstances.begin(); p != _definition.serverInstances.end(); ++p)
    {
	ServerInstanceHelper helper(*p, resolve);
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    ServerDescriptorSeq::const_iterator q;
    for(q = _definition.servers.begin(); q != _definition.servers.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve);
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    _instance = instantiate(resolve);

    validate(appResolve);
}

bool
NodeHelper::operator==(const NodeHelper& helper) const
{
    if(_instance.variables != helper._instance.variables)
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

    if(_instance.loadFactor != helper._instance.loadFactor)
    {
	return false;
    }

    if(_instance.description != helper._instance.description)
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

NodeDescriptor
NodeHelper::instantiate(const Resolver& resolve) const
{
    NodeDescriptor desc;
    desc.variables = _definition.variables;
    desc.loadFactor = resolve(_definition.loadFactor, "load factor");
    desc.description = resolve(_definition.description, "description");
    ServerInstanceHelperDict::const_iterator r;
    for(r = _serverInstances.begin(); r != _serverInstances.end(); ++r)
    {
	desc.serverInstances.push_back(r->second.getInstance());
    }
    for(r = _servers.begin(); r != _servers.end(); ++r)
    {
	desc.servers.push_back(r->second.getServerInstance());
    }
    return desc;
}

NodeUpdateDescriptor
NodeHelper::diff(const NodeHelper& helper) const
{
    assert(_name == helper._name);

    NodeUpdateDescriptor update;

    update.name = _name;
    if(_definition.loadFactor != helper._definition.loadFactor)
    {
	update.loadFactor = new BoxedString(_definition.loadFactor);
    }

    if(_definition.description != helper._definition.description)
    {
	update.description = new BoxedString(_definition.description);
    }

    update.variables = getDictUpdatedElts(helper._definition.variables, _definition.variables);
    update.removeVariables = getDictRemovedElts(helper._definition.variables, _definition.variables);
    
    ServerInstanceHelperDict updated = getDictUpdatedElts(helper._serverInstances, _serverInstances);
    for(ServerInstanceHelperDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
    {
	update.serverInstances.push_back(p->second.getDefinition());
    }
    update.removeServers = getDictRemovedElts(helper._serverInstances, _serverInstances);

    updated = getDictUpdatedElts(helper._servers, _servers);
    for(ServerInstanceHelperDict::const_iterator q = updated.begin(); q != updated.end(); ++q)
    {
	update.servers.push_back(q->second.getServerDefinition());
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
    // Update the variables.
    //
    _definition.variables = updateDictElts(_definition.variables, update.variables, update.removeVariables);

    if(update.loadFactor)
    {
	_definition.loadFactor = update.loadFactor->value;
    }
    
    if(update.description)
    {
	_definition.description = update.description->value;
    }

    //
    // Remove the servers and server instances.
    //
    for(Ice::StringSeq::const_iterator t = update.removeServers.begin(); t != update.removeServers.end(); ++t)
    {
	_serverInstances.erase(*t);
	_servers.erase(*t);
    }

    for(map<string, string>::const_iterator p = update.variables.begin(); p != update.variables.end(); ++p)
    {
	_definition.variables[p->first] = p->second;
    }

    //
    // NOTE: It's important to create the resolver *after* updating
    // the node variables!
    //
    Resolver resolve(appResolve, _definition.variables, false);
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
    ServerInstanceDescriptorSeq::const_iterator q;
    for(q = update.serverInstances.begin(); q != update.serverInstances.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve);
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	serverInstances.erase(helper.getId());
    }
    ServerInstanceHelperDict::const_iterator r;
    for(r = serverInstances.begin(); r != serverInstances.end(); ++r)
    {
	ServerInstanceHelper helper(r->second.getDefinition(), resolve); // Re-instantiate the server.
	if(helper.getId() != r->first)
	{
	    resolve.exception("invalid update in node `" + _name + "':\n" +
			      "server instance id `" + r->first + "' changed to `" + helper.getId() + "'");
	}
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
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
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	servers.erase(helper.getId());
    }    
    for(r = servers.begin(); r != servers.end(); ++r)
    {
	ServerInstanceHelper helper(r->second.getServerDefinition(), resolve); // Re-instantiate the server.
	if(helper.getId() != r->first)
	{
	    resolve.exception("invalid update in node `" + _name + "':\n" +
			      "server instance id `" + r->first + "' changed to `" + helper.getId() + "'");
	}	
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }

    //
    // Update the node descriptor with the new server instances and servers.
    //
    _definition.serverInstances.clear();
    for(r = _serverInstances.begin(); r != _serverInstances.end(); ++r)
    {
	_definition.serverInstances.push_back(r->second.getDefinition());
    }

    _definition.servers.clear();
    for(r = _servers.begin(); r != _servers.end(); ++r)
    {
	_definition.servers.push_back(r->second.getServerDefinition());
    }

    _instance = instantiate(resolve);
}

void
NodeHelper::instantiateServer(const ServerInstanceDescriptor& instance, const Resolver& appResolve)
{
    Resolver resolve(appResolve, _definition.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    ServerInstanceHelper helper(instance, resolve);
    if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
    {
	resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
    }

    //
    // Update the node descriptor with the new server instances and servers.
    //
    _definition.serverInstances.clear();
    for(ServerInstanceHelperDict::const_iterator r = _serverInstances.begin(); r != _serverInstances.end(); ++r)
    {
	_definition.serverInstances.push_back(r->second.getDefinition());
    }

    _instance = instantiate(resolve);
}

void
NodeHelper::getIds(multiset<string>& serverIds, multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    ServerInstanceHelperDict::const_iterator p;
    for(p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	serverIds.insert(p->first);
	p->second.getIds(adapterIds, objectIds);
    }
    for(p = _servers.begin(); p != _servers.end(); ++p)
    {
	serverIds.insert(p->first);
	p->second.getIds(adapterIds, objectIds);
    }    
}

const NodeDescriptor&
NodeHelper::getDescriptor() const
{
    return _definition;
}

const NodeDescriptor&
NodeHelper::getInstance() const
{
    return _instance;
}

void
NodeHelper::getServerInfos(const string& application, map<string, ServerInfo>& servers) const
{
    ServerInstanceHelperDict::const_iterator p;
    for(p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	ServerInfo info;
	info.node = _name;
	info.application = application;
	info.descriptor = p->second.getServerInstance();
	servers.insert(make_pair(p->second.getId(), info));
    }
    for(p = _servers.begin(); p != _servers.end(); ++p)
    {
	ServerInfo info;
	info.node = _name;
	info.application = application;
	info.descriptor = p->second.getServerInstance();
	servers.insert(make_pair(p->second.getId(), info));
    }
}

DistributionDescriptorDict
NodeHelper::getDistributions(const string& server) const
{
    //
    // Get the server distributions to patch.
    //
    DistributionDescriptorDict distribs;
    if(server.empty())
    {
	ServerInstanceHelperDict::const_iterator p;
	for(p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
	{
	    if(!p->second.getServerInstance()->distrib.icepatch.empty())
	    {
		distribs.insert(make_pair(p->first, p->second.getServerInstance()->distrib));
	    }
	}
	for(p = _servers.begin(); p != _servers.end(); ++p)
	{
	    if(!p->second.getServerInstance()->distrib.icepatch.empty())
	    {
		distribs.insert(make_pair(p->first, p->second.getServerInstance()->distrib));
	    }
	}
    }
    else
    {
	ServerInstanceHelperDict::const_iterator p = _serverInstances.find(server);
	if(p == _serverInstances.end())
	{
	    p = _servers.find(server);
	}
	if(p != _serverInstances.end())
	{
	    if(!p->second.getServerInstance()->distrib.icepatch.empty())
	    {
		distribs.insert(make_pair(server, p->second.getServerInstance()->distrib));
	    }
	}
    }
    return distribs;
}

bool
NodeHelper::hasServers() const
{
    return !_serverInstances.empty() || !_servers.empty();
}

bool
NodeHelper::hasServer(const string& name) const
{
    return _serverInstances.find(name) != _serverInstances.end() || _servers.find(name) != _servers.end();
}

void
NodeHelper::print(Output& out) const
{
    out << nl << "node '" << _name << "'";
    out << sb;
    if(!_instance.loadFactor.empty())
    {
	out << nl << "load factor = `" << _instance.loadFactor << "'";
    }
    if(!_instance.description.empty())
    {
	out << nl << "description = `" << _instance.description << "'";
    }
    if(!_instance.variables.empty())
    {
	out << nl << "variables";
	out << sb;
	for(StringStringDict::const_iterator q = _instance.variables.begin(); q != _instance.variables.end(); ++q)
	{
	    out << nl << q->first << " = `" << q->second << "'";
	}
	out << eb;
    }

    if(_serverInstances.empty() && _servers.empty())
    {
	return;
    }
    
    out << nl << "servers";
    out << sb;
    ServerInstanceHelperDict::const_iterator p;
    for(p = _serverInstances.begin(); p != _serverInstances.end(); ++p)
    {
	out << nl << p->first;
    }
    for(p = _servers.begin(); p != _servers.end(); ++p)
    {
	out << nl << p->first;
    }
    out << eb;
    out << eb;
}

void
NodeHelper::printDiff(Output& out, const NodeHelper& helper) const
{
    ServerInstanceHelperDict updated1 = getDictUpdatedElts(helper._serverInstances, _serverInstances);
    Ice::StringSeq removed1 = getDictRemovedElts(helper._serverInstances, _serverInstances);
    ServerInstanceHelperDict updated2 = getDictUpdatedElts(helper._servers, _servers);
    Ice::StringSeq removed2 = getDictRemovedElts(helper._servers, _servers);
    
    ServerInstanceHelperDict updated;
    Ice::StringSeq removed;
    updated.insert(updated1.begin(), updated1.end());
    removed.insert(removed.end(), removed1.begin(), removed1.end());
    updated.insert(updated2.begin(), updated2.end());
    removed.insert(removed.end(), removed2.begin(), removed2.end());

    map<string, string> variables = getDictUpdatedElts(helper._instance.variables, _instance.variables);
    Ice::StringSeq removeVariables = getDictRemovedElts(helper._instance.variables, _instance.variables);

    if(updated.empty() && removed.empty() &&
       variables.empty() && removeVariables.empty() &&
       _instance.loadFactor == helper._instance.loadFactor &&
       _instance.description == helper._instance.description)
    {
	return;
    }

    //
    // TODO: Show updated variables?
    //

    out << nl << "node `" + _name + "' updated";
    out << sb;

    if(_instance.loadFactor != helper._instance.loadFactor)
    {
	out << nl << "load factor udpated";
    }
    if(_instance.description != helper._instance.description)
    {
	out << nl << "description udpated";
    }
    if(!variables.empty() || !removeVariables.empty())
    {
	out << nl << "variables udpated";
    }
    if(!updated.empty() || !removed.empty())
    {
	out << nl << "servers";
	out << sb;
	ServerInstanceHelperDict::const_iterator p;
	for(p = updated.begin(); p != updated.end(); ++p)
	{
	    if(helper._serverInstances.find(p->first) == helper._serverInstances.end() &&
	       helper._servers.find(p->first) == helper._servers.end())
	    {
		out << nl << "server `" << p->first << "' added";
	    }
	}
	for(p = updated.begin(); p != updated.end(); ++p)
	{
	    if(helper._serverInstances.find(p->first) != helper._serverInstances.end() ||
	       helper._servers.find(p->first) != helper._servers.end())
	    {
		out << nl << "server `" << p->first << "' updated";
	    }
	}
	for(Ice::StringSeq::const_iterator q = removed.begin(); q != removed.end(); ++q)
	{
	    out << nl << "server `" << *q << "' removed";
	}    
	out << eb;
    }
    out << eb;
}

void
NodeHelper::validate(const Resolver& appResolve) const
{
    Resolver resolve(appResolve, _definition.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    for(StringStringDict::const_iterator p = _definition.variables.begin(); p != _definition.variables.end(); ++p)
    {
	if(p->first == "")
	{
	    resolve.exception("empty variable name");
	}
    }
}

ApplicationHelper::ApplicationHelper(const ApplicationDescriptor& desc) :
    _definition(desc)
{
    if(_definition.name.empty())
    {
	throw DeploymentException("invalid application: empty name");
    }

    Resolver resolve(*this, _definition.name, desc.variables);
    for(NodeDescriptorDict::const_iterator p = _definition.nodes.begin(); p != _definition.nodes.end(); ++p)
    {
	_nodes.insert(make_pair(p->first, NodeHelper(p->first, p->second, resolve)));
    }
    
    _instance = instantiate(resolve); 
    
    validate(resolve);
}

ApplicationDescriptor
ApplicationHelper::instantiate(const Resolver& resolve) const
{
    ApplicationDescriptor desc = _definition;

    desc.description = resolve(_definition.description, "description");
    desc.distrib.icepatch = resolve(desc.distrib.icepatch, "IcePatch2 server proxy");
    Ice::StringSeq::const_iterator p;
    desc.distrib.directories.clear();
    for(p = _definition.distrib.directories.begin(); p != _definition.distrib.directories.end(); ++p)
    {
	desc.distrib.directories.push_back(resolve(*p, "distribution source directory"));
    }

    ReplicaGroupDescriptorSeq::iterator r;
    for(r = desc.replicaGroups.begin(); r != desc.replicaGroups.end(); ++r)
    {
	r->description = resolve(r->description, "replica group description");
	if(r->loadBalancing)
	{
	    r->loadBalancing = LoadBalancingPolicyPtr::dynamicCast(r->loadBalancing->ice_clone());
	    r->loadBalancing->nReplicas = resolve(r->loadBalancing->nReplicas, "replica group number of replicas");
	    AdaptiveLoadBalancingPolicyPtr al = AdaptiveLoadBalancingPolicyPtr::dynamicCast(r->loadBalancing);
	    if(al)
	    {
		al->loadSample = resolve(al->loadSample, "replica group load sample");
		if(al->loadSample != "" && al->loadSample != "1" && al->loadSample != "5" && al->loadSample != "15")
		{
		    resolve.exception("invalid load sample value (allowed values are 1, 5 or 15)");
		}
	    }
	}

	for(ObjectDescriptorSeq::iterator q = r->objects.begin(); q != r->objects.end(); ++q)
	{
	    q->type = resolve(q->type, "object type");
	    q->id.name = resolve(q->id.name, "object identity name", false);
	    q->id.category = resolve(q->id.category, "object identity category");
	}
    }

    desc.nodes.clear();
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	desc.nodes.insert(make_pair(n->first, n->second.getInstance()));
    }
    
    return desc;
}

ApplicationUpdateDescriptor
ApplicationHelper::diff(const ApplicationHelper& helper)
{
    ApplicationUpdateDescriptor update;

    update.name = _definition.name;
    if(_definition.description != helper._definition.description)
    {
	update.description = new BoxedString(_definition.description);
    }

    update.variables = getDictUpdatedElts(helper._definition.variables, _definition.variables);
    update.removeVariables = getDictRemovedElts(helper._definition.variables, _definition.variables);

    if(_definition.distrib != helper._definition.distrib)
    {
	update.distrib = new BoxedDistributionDescriptor(_definition.distrib);
    }

    GetReplicaGroupId rk;
    ReplicaGroupEq req;
    update.replicaGroups = 
	getSeqUpdatedEltsWithEq(helper._definition.replicaGroups, _definition.replicaGroups, rk, req);
    update.removeReplicaGroups = getSeqRemovedElts(helper._definition.replicaGroups, _definition.replicaGroups, rk);

    TemplateDescriptorEqual tmpleq;
    update.serverTemplates = 
	getDictUpdatedEltsWithEq(helper._definition.serverTemplates, _definition.serverTemplates, tmpleq);
    update.removeServerTemplates =
	getDictRemovedElts(helper._definition.serverTemplates, _definition.serverTemplates);
    update.serviceTemplates = 
	getDictUpdatedEltsWithEq(helper._definition.serviceTemplates, _definition.serviceTemplates, tmpleq);
    update.removeServiceTemplates = 
	getDictRemovedElts(helper._definition.serviceTemplates, _definition.serviceTemplates);

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
	    nodeUpdate.loadFactor = new BoxedString(node.loadFactor);
	    nodeUpdate.description = new BoxedString(node.description);
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
	_definition.description = update.description->value;
    }

    _definition.replicaGroups = updateSeqElts(_definition.replicaGroups, update.replicaGroups, 
					      update.removeReplicaGroups, GetReplicaGroupId());
    
    _definition.variables = updateDictElts(_definition.variables, update.variables, update.removeVariables);

    if(update.distrib)
    {
	_definition.distrib = update.distrib->value;
    }

    _definition.serverTemplates = updateDictElts(_definition.serverTemplates, update.serverTemplates, 
						 update.removeServerTemplates);

    _definition.serviceTemplates = updateDictElts(_definition.serviceTemplates, update.serviceTemplates, 
						  update.removeServiceTemplates);

    for(Ice::StringSeq::const_iterator r = update.removeNodes.begin(); r != update.removeNodes.end(); ++r)
    {
	_nodes.erase(*r);
    }

    //
    // NOTE: It's important to create the resolver *after* updating
    // the application variables!
    //
    Resolver resolve(*this, _definition.name, _definition.variables);

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
	    desc.loadFactor = p->loadFactor ? p->loadFactor->value : "";
	    desc.description = p->description ? p->description->value : "";
	    _nodes.insert(make_pair(p->name, NodeHelper(p->name, desc, resolve)));
	}
	else
	{
	    q->second.update(*p, resolve);
	    _nodes.insert(make_pair(p->name, q->second));
	    nodes.erase(q);
	}
    }
    for(NodeHelperDict::const_iterator n = nodes.begin(); n != nodes.end(); ++n)
    {
	NodeHelper helper(n->first, n->second.getDescriptor(), resolve); // Re-instantiate the node.
	_nodes.insert(make_pair(n->first, helper));
    }

    //
    // Update the application descriptor.
    //
    _definition.nodes.clear();
    for(NodeHelperDict::const_iterator t = _nodes.begin(); t != _nodes.end(); ++t)
    {
	_definition.nodes.insert(make_pair(t->first, t->second.getDescriptor()));
    }

    //
    // Update the application descriptor instance.
    //
    _instance = instantiate(resolve);

    //
    // Validate the new application descriptor.
    //
    validate(resolve);
}

void
ApplicationHelper::instantiateServer(const string& node, const ServerInstanceDescriptor& instance)
{
    Resolver resolve(*this, _definition.name, _definition.variables);

    //
    // Get the node helper (add it if it doesn't exist.)
    //
    NodeHelperDict::iterator q = _nodes.find(node);
    if(q == _nodes.end())
    {
	q = _nodes.insert(q, NodeHelperDict::value_type(node, NodeHelper(node, NodeDescriptor(), resolve)));
    }
    q->second.instantiateServer(instance, resolve);

    //
    // Update the application definition.
    //
    _definition.nodes[node] = q->second.getDescriptor();

    //
    // Update the application descriptor instance.
    //
    _instance = instantiate(resolve);

    //
    // Validate the new application descriptor.
    //
    validate(resolve);    
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
    ReplicaGroupDescriptorSeq::const_iterator r;
    for(r = _definition.replicaGroups.begin(); r != _definition.replicaGroups.end(); ++r)
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
    return _definition;
}

const ApplicationDescriptor&
ApplicationHelper::getInstance() const
{
    return _instance;
}

TemplateDescriptor 
ApplicationHelper::getServerTemplate(const string& name) const
{
    TemplateDescriptorDict::const_iterator p = _definition.serverTemplates.find(name);
    if(p == _definition.serverTemplates.end())
    {
	throw DeploymentException("unknown server template `" + name + "'");
    }
    return p->second;
}

TemplateDescriptor 
ApplicationHelper::getServiceTemplate(const string& name) const
{
    TemplateDescriptorDict::const_iterator p = _definition.serviceTemplates.find(name);
    if(p == _definition.serviceTemplates.end())
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
	n->second.getServerInfos(_definition.name, servers);
    }
    return servers;
}

void
ApplicationHelper::getDistributions(DistributionDescriptor& distribution,
				    map<string, DistributionDescriptorDict>& nodeDistributions,
				    const string& server) const
{
    distribution = _instance.distrib;
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	DistributionDescriptorDict distrib = n->second.getDistributions(server);
	if(!distrib.empty())
	{
	    nodeDistributions.insert(make_pair(n->first, distrib));
	    if(!server.empty())
	    {
		break;
	    }
	}
	else if(!_instance.distrib.icepatch.empty() && 
		(server.empty() && n->second.hasServers() || n->second.hasServer(server)))
	{
	    nodeDistributions.insert(make_pair(n->first, distrib));
	}
    }
}

void
ApplicationHelper::print(Output& out) const
{
    out << "application '" << _instance.name << "'";
    out << sb;
    if(!_instance.description.empty())
    {
	out << nl << "description = " << _instance.description;
    }
    if(!_instance.variables.empty())
    {
	out << nl << "variables";
	out << sb;
	for(StringStringDict::const_iterator p = _instance.variables.begin(); p != _instance.variables.end(); 
	    ++p)
	{
	    out << nl << p->first << " = `" << p->second << "'";
	}
	out << eb;
    }
    if(!_instance.distrib.icepatch.empty())
    {
	out << nl << "distribution";
	out << sb;
	out << nl << "proxy = `" << _instance.distrib.icepatch << "'";
	out << nl << "directories = " << toString(_instance.distrib.directories);
	out << eb;
    }
    if(!_instance.replicaGroups.empty())
    {
	out << nl << "replica groups";
	out << sb;
	ReplicaGroupDescriptorSeq::const_iterator p;
	for(p = _instance.replicaGroups.begin(); p != _instance.replicaGroups.end(); ++p)
	{
	    out << nl << "id = `" << p->id << "' load balancing = `";
	    if(!p->loadBalancing)
	    {
		out << "default (return all endpoints)";
	    }
	    else if(RandomLoadBalancingPolicyPtr::dynamicCast(p->loadBalancing))
	    {
		out << "random";
	    }
	    else if(RoundRobinLoadBalancingPolicyPtr::dynamicCast(p->loadBalancing))
	    {
		out << "round-robin";
	    }
	    else if(AdaptiveLoadBalancingPolicyPtr::dynamicCast(p->loadBalancing))
	    {
		out << "adaptive" ;
	    }
	    else
	    {
		out << "<unknown load balancing policy>";
	    }
	    out << "'";
	}
	out << eb;
    }
    if(!_instance.serverTemplates.empty())
    {
	out << nl << "server templates";
	out << sb;
	TemplateDescriptorDict::const_iterator p;
	for(p = _instance.serverTemplates.begin(); p != _instance.serverTemplates.end(); ++p)
	{
	    out << nl << p->first;
	}
	out << eb;
    }
    if(!_instance.serviceTemplates.empty())
    {
	out << nl << "service templates";
	out << sb;
	TemplateDescriptorDict::const_iterator p;
	for(p = _instance.serviceTemplates.begin(); p != _instance.serviceTemplates.end(); ++p)
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
    out << "application `" << _instance.name << "'";
    out << sb;

    {
	map<string, string> variables = getDictUpdatedElts(helper._instance.variables, _instance.variables);
	Ice::StringSeq removeVariables = getDictRemovedElts(helper._instance.variables, _instance.variables);
	if(!variables.empty() || !removeVariables.empty())
	{
	    out << nl << "variables udpated";
	}    
    }
    {
	if(_definition.distrib != helper._definition.distrib)
	{
	    out << nl << "distribution updated";
	}
    }
    {
	GetReplicaGroupId rk;
	ReplicaGroupEq req;
	ReplicaGroupDescriptorSeq updated = 
	    getSeqUpdatedEltsWithEq(helper._definition.replicaGroups, _definition.replicaGroups, rk, req);
	Ice::StringSeq removed = getSeqRemovedElts(helper._definition.replicaGroups, _definition.replicaGroups, rk);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "replica groups";
	    out << sb;
	    ReplicaGroupDescriptorSeq::iterator p = updated.begin();
	    while(p != updated.end())
	    {
		ReplicaGroupDescriptorSeq::const_iterator r;
		for(r = helper._instance.replicaGroups.begin(); r != helper._instance.replicaGroups.end(); 
		    ++r)
		{
		    if(p->id == r->id)
		    {
			out << nl << "replica group `" << r->id << "' updated";
			p = updated.erase(p);
			break;
		    }
		}
		if(r == helper._instance.replicaGroups.end())
		{
		    ++p;
		}
	    }
	    for(p = updated.begin(); p != updated.end(); ++p)
	    {
		out << nl << "replica group `" << p->id << "' added";
	    }
	    for(Ice::StringSeq::const_iterator q = removed.begin(); q != removed.end(); ++q)
	    {
		out << nl << "replica group `" << *q << "' removed";
	    }
	    out << eb;
	}
    }

    {
	TemplateDescriptorEqual eq;
	TemplateDescriptorDict updated;
	updated = getDictUpdatedEltsWithEq(helper._instance.serverTemplates, _instance.serverTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._instance.serverTemplates, _instance.serverTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "server templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._instance.serverTemplates.find(p->first) == helper._instance.serverTemplates.end())
		{
		    out << nl << "server template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator q = updated.begin(); q != updated.end(); ++q)
	    {
		if(helper._instance.serverTemplates.find(q->first) != helper._instance.serverTemplates.end())
		{
		    out << nl << "server template `" << q->first << "' updated";
		}
	    }
	    for(Ice::StringSeq::const_iterator r = removed.begin(); r != removed.end(); ++r)
	    {
		out << nl << "server template `" << *r << "' removed";
	    }
	    out << eb;
	}
    }
    {
	TemplateDescriptorEqual eq;
	TemplateDescriptorDict updated;
	updated = getDictUpdatedEltsWithEq(helper._instance.serviceTemplates, _instance.serviceTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._instance.serviceTemplates, _instance.serviceTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "service templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._instance.serviceTemplates.find(p->first) == helper._instance.serviceTemplates.end())
		{
		    out << nl << "service template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator q = updated.begin(); q != updated.end(); ++q)
	    {
		if(helper._instance.serviceTemplates.find(q->first) != helper._instance.serviceTemplates.end())
		{
		    out << nl << "service template `" << q->first << "' updated";
		}
	    }
	    for(Ice::StringSeq::const_iterator r = removed.begin(); r != removed.end(); ++r)
	    {
		out << nl << "service template `" << *r << "' removed";
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
	    for(NodeHelperDict::const_iterator r = updated.begin(); r != updated.end(); ++r)
	    {
		NodeHelperDict::const_iterator q = helper._nodes.find(r->first);
		if(q != helper._nodes.end())
		{
		    r->second.printDiff(out, q->second);
		}
	    }
	    for(Ice::StringSeq::const_iterator s = removed.begin(); s != removed.end(); ++s)
	    {
		out << nl << "node `" << *s << "' removed";
	    }
	    out << eb;
	}
    }
    out << eb;
}

void
ApplicationHelper::validate(const Resolver& resolve) const
{
    for(StringStringDict::const_iterator p = _definition.variables.begin(); p != _definition.variables.end(); ++p)
    {
	if(p->first == "")
	{
	    resolve.exception("invalid application `" + _definition.name + "': empty variable name");
	}
    }
    
    TemplateDescriptorDict::const_iterator t;
    for(t = _definition.serverTemplates.begin(); t != _definition.serverTemplates.end(); ++t)
    {
	if(t->first == "")
	{
	    resolve.exception("empty server template id");
	}
    }
    for(t = _definition.serviceTemplates.begin(); t != _definition.serviceTemplates.end(); ++t)
    {
	if(t->first == "")
	{
	    resolve.exception("empty service template id");
	}
    }

    //
    // Ensure the unicity of object ids, adapter ids and server ids.
    //    
    multiset<string> serverIds;
    multiset<string> adapterIds;
    multiset<Ice::Identity> objectIds;
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	n->second.validate(resolve);
	n->second.getIds(serverIds, adapterIds, objectIds);
    }

    ReplicaGroupDescriptorSeq::const_iterator r;
    for(r = _definition.replicaGroups.begin(); r != _definition.replicaGroups.end(); ++r)
    {
	if(r->id.empty())
	{
	    throw DeploymentException("replica group id is empty");
	}
	if(adapterIds.find(r->id) != adapterIds.end())
	{
	    throw DeploymentException("duplicate replica group `" +  r->id + "'");
	}
	adapterIds.insert(r->id);
	for(ObjectDescriptorSeq::const_iterator o = r->objects.begin(); o != r->objects.end(); ++o)
	{
	    objectIds.insert(o->id);
	}
    }

    for(multiset<string>::const_iterator s = serverIds.begin(); s != serverIds.end(); ++s)
    {
	if(serverIds.count(*s) > 1)
	{
	    resolve.exception("duplicate server `" + *s + "'");
	}
    }
    for(multiset<string>::const_iterator a = adapterIds.begin(); a != adapterIds.end(); ++a)
    {
	if(adapterIds.count(*a) > 1)
	{
	    resolve.exception("duplicate adapter `" + *a + "'");
	}
    }
    for(multiset<Ice::Identity>::const_iterator o = objectIds.begin(); o != objectIds.end(); ++o)
    {
	if(objectIds.count(*o) > 1)
	{
	    resolve.exception("duplicate object `" + Ice::identityToString(*o) + "'");
	}
    }
}

bool
IceGrid::descriptorEqual(const ServerDescriptorPtr& lhs, const ServerDescriptorPtr& rhs)
{
    if(lhs->ice_id() != rhs->ice_id())
    {
	return false;
    }
    IceBoxDescriptorPtr lhsIceBox = IceBoxDescriptorPtr::dynamicCast(lhs);
    if(lhsIceBox)
    {
	return IceBoxHelper(lhsIceBox) == IceBoxHelper(IceBoxDescriptorPtr::dynamicCast(rhs));
    }
    else
    {
	return ServerHelper(lhs) == ServerHelper(rhs);
    }
}
