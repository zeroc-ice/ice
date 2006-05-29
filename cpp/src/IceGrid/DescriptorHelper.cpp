// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
    TemplateDescriptorEqual(const Ice::CommunicatorPtr& comm) :
        communicator(comm)
    {
    }

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
		return IceBoxHelper(communicator, slhs) == IceBoxHelper(communicator, srhs);
	    }
	}
	{
	    ServerDescriptorPtr slhs = ServerDescriptorPtr::dynamicCast(lhs.descriptor);
	    ServerDescriptorPtr srhs = ServerDescriptorPtr::dynamicCast(rhs.descriptor);
	    if(slhs && srhs)
	    {
		return ServerHelper(communicator, slhs) == ServerHelper(communicator, srhs);
	    }
	}
	{
	    ServiceDescriptorPtr slhs = ServiceDescriptorPtr::dynamicCast(lhs.descriptor);
	    ServiceDescriptorPtr srhs = ServiceDescriptorPtr::dynamicCast(rhs.descriptor);
	    if(slhs && srhs)
	    {
		return ServiceHelper(communicator, slhs) == ServiceHelper(communicator, srhs);
	    }   
	}

	return false;
    }

    const Ice::CommunicatorPtr& communicator;
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
   return getSeqUpdatedElts(lseq, rseq, func, equal_to<Seq::value_type>());
#else
   return getSeqUpdatedElts(lseq, rseq, func, equal_to<typename Seq::value_type>());
#endif
}

template <typename GetKeyFunc, typename EqFunc, typename Seq> Seq
#if defined(_MSC_VER) && (_MSC_VER < 1300)
getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq)
#else
getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq)
#endif
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
    return getDictUpdatedElts(ldict, rdict, equal_to<Dict::mapped_type>());
#else
    return getDictUpdatedElts(ldict, rdict, equal_to<typename Dict::mapped_type>());
#endif
}

template<typename EqFunc, typename Dict> Dict
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

Resolver::Resolver(const ApplicationDescriptor& app, const Ice::CommunicatorPtr& communicator) : 
    _application(&app),
    _communicator(communicator),
    _escape(false),
    _context("application `" + app.name + "'"),
    _variables(app.variables),
    _reserved(getReserved())
{
    //
    // Make sure the variables don't override reserved variables.
    //
    checkReserved("variable", _variables);
    setReserved("application", app.name);

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

    for(StringStringDict::const_iterator v = _variables.begin(); v != _variables.end(); ++v)
    {
	if(v->first == "")
	{
	    exception("empty variable name");
	}
    }

    TemplateDescriptorDict::const_iterator t;
    for(t = _application->serverTemplates.begin(); t != _application->serverTemplates.end(); ++t)
    {
	if(t->first == "")
	{
	    exception("empty server template id");
	}
    }
    for(t = _application->serviceTemplates.begin(); t != _application->serviceTemplates.end(); ++t)
    {
	if(t->first == "")
	{
	    exception("empty service template id");
	}
    }
}

Resolver::Resolver(const Resolver& resolve, 
		   const map<string, string>& values, 
		   bool params) :
    _application(resolve._application),
    _communicator(resolve._communicator),
    _escape(resolve._escape),
    _context(resolve._context),
    _variables(params ? resolve._variables : values),
    _parameters(!params ? resolve._parameters : values),
    _propertySets(resolve._propertySets),
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

    for(StringStringDict::const_iterator v = _variables.begin(); v != _variables.end(); ++v)
    {
	if(v->first == "")
	{
	    exception("empty variable name");
	}
    }
}

Resolver::Resolver(const string& context, const map<string, string>& values, const Ice::CommunicatorPtr& com) :
    _application(0),
    _communicator(com),
    _escape(true),
    _context(context),
    _variables(values),
    _reserved(getReserved())
{
    checkReserved("variable", values);

    for(StringStringDict::const_iterator v = _variables.begin(); v != _variables.end(); ++v)
    {
	if(v->first == "")
	{
	    exception("empty variable name");
	}
    }
}

string 
Resolver::operator()(const string& value, const string& name, bool allowEmpty, bool useParams) const
{
    try
    {
	string val;
	try
	{
	    val = substitute(value, useParams);
	}
	catch(const string& reason)
	{
	    throw "invalid variable `" + value + "': " + reason;
	}
	catch(const char* reason)
	{
	    throw "invalid variable `" + value + "': " + reason;
	}

	if(!allowEmpty)
	{
	    if(value.empty())
	    {
		throw "empty string";
	    }
	    else if(val.empty())
	    {
		throw "the value of the variable `" + value + "' is an empty string";
	    }
	}
	return val;
    }
    catch(const string& reason)
    {
	exception("invalid value for attribute `" + name + "': " + reason);
    }
    catch(const char* reason)
    {
	exception("invalid value for attribute `" + name + "': " + reason);
    }
    return ""; // To prevent compiler warning.
}

Ice::StringSeq
Resolver::operator()(const Ice::StringSeq& values, const string& name) const
{
    Ice::StringSeq result;
    for(Ice::StringSeq::const_iterator p = values.begin(); p != values.end(); ++p)
    {
	result.push_back(operator()(*p, name));
    }
    return result;
}

DistributionDescriptor
Resolver::operator()(const DistributionDescriptor& desc) const
{
    DistributionDescriptor result;
    result.icepatch = operator()(desc.icepatch, "IcePatch2 server proxy");
    result.directories = operator()(desc.directories, "distribution source directory");
    return desc;
}

PropertyDescriptorSeq
Resolver::operator()(const PropertyDescriptorSeq& properties, const string& name) const 
{
    PropertyDescriptorSeq result;
    for(PropertyDescriptorSeq::const_iterator q = properties.begin(); q != properties.end(); ++q)
    {
	PropertyDescriptor prop;
	prop.name = operator()(q->name, name + " name");
	prop.value = operator()(q->value, name + " value");
	result.push_back(prop);
    }
    return result;
}

PropertySetDescriptorDict
Resolver::operator()(const PropertySetDescriptorDict& propertySets) const
{
    PropertySetDescriptorDict result;
    PropertySetDescriptorDict::const_iterator ps;
    for(ps = propertySets.begin(); ps != propertySets.end(); ++ps)
    {
	PropertySetDescriptor desc;
	desc.references = operator()(ps->second.references, "property set `" + ps->first + "' reference");
	desc.properties = operator()(ps->second.properties, "property set `" + ps->first + "' property");
	result.insert(make_pair(ps->first, desc));
    }
    return result;
}

ObjectDescriptorSeq
Resolver::operator()(const ObjectDescriptorSeq& objects) const
{
    ObjectDescriptorSeq result;
    for(ObjectDescriptorSeq::const_iterator q = objects.begin(); q != objects.end(); ++q)
    {
	ObjectDescriptor obj;
	obj.type = operator()(q->type, "object type");
	obj.id = operator()(q->id, "object identity");
	result.push_back(obj);
    }
    return result;
}

Ice::Identity
Resolver::operator()(const Ice::Identity& value, const string& name) const
{
    assert(_communicator);
    string str = operator()(_communicator->identityToString(value), name, false);
    Ice::Identity id = _communicator->stringToIdentity(str);
    if(id.name.empty())
    {
	exception("invalid object identity `" + _communicator->identityToString(value) + "': name empty");
    }
    return id;
}

PropertySetDescriptor
Resolver::operator()(const PropertySetDescriptor& desc) const
{
    PropertySetDescriptor result;
    result.properties = getProperties(desc.references);
    PropertyDescriptorSeq props = operator()(desc.properties);
    result.properties.insert(result.properties.end(), props.begin(), props.end());
    return result;
}

string
Resolver::asInt(const string& value, const string& name) const
{
    string v = operator()(value, name);
    if(!v.empty())
    {
	string::size_type beg = v.find_first_not_of(' ');
	string::size_type end = v.find_last_not_of(' ');
	v = v.substr(beg == string::npos ? 0 : beg, end == string::npos ? v.length() - 1 : end - beg + 1);

	int val;
	istringstream is(v);
	if(!(is >> val) || !is.eof())
	{
	    DeploymentException ex;
	    ex.reason = "invalid value `" + value + "' for `" + name + "' in " + _context + ": not an integer";
	    throw ex;
	}
    }
    return v;
}

string
Resolver::asFloat(const string& value, const string& name) const
{
    string v = operator()(value, name);
    if(!v.empty())
    {
	string::size_type beg = v.find_first_not_of(' ');
	string::size_type end = v.find_last_not_of(' ');
	v = v.substr(beg == string::npos ? 0 : beg, end == string::npos ? v.length() - 1 : end - beg + 1);

	float val;
	istringstream is(v);
	if(!(is >> val) || !is.eof())
	{
	    DeploymentException ex;
	    ex.reason = "invalid value `" + value + "' for `" + name + "' in " + _context + ": not a float";
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
Resolver::addPropertySets(const PropertySetDescriptorDict& propertySets)
{
    PropertySetDescriptorDict oldPropertySets;
    oldPropertySets.swap(_propertySets);
    PropertySetDescriptorDict::const_iterator p;
    for(p = propertySets.begin(); p != propertySets.end(); ++p)
    {
 	if(!_propertySets.insert(*p).second)
 	{
 	    exception("property set with id `" + p->first + "' is already defined at this scope");
 	}
    }
    _propertySets.insert(oldPropertySets.begin(), oldPropertySets.end());

    //
    // Validate the new property set references.
    //
    for(p = propertySets.begin(); p != propertySets.end(); ++p)
    {
	getProperties(p->second.references);
    }
}

const PropertySetDescriptor&
Resolver::getPropertySet(const string& id) const
{
    PropertySetDescriptorDict::const_iterator p = _propertySets.find(id);
    if(p == _propertySets.end())
    {
	exception("invalid reference to property set, property set `" + id + "' doesn't exist");
    }
    return p->second;
}

PropertyDescriptorSeq
Resolver::getProperties(const Ice::StringSeq& references) const
{
    set<string> resolved;
    return getProperties(references, resolved);
}

void
Resolver::addIgnored(const string& name)
{
    _ignore.insert(name);
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
    TemplateDescriptorDict::const_iterator p = _application->serverTemplates.find(tmpl);
    if(p == _application->serverTemplates.end())
    {
	throw DeploymentException("unknown server template `" + tmpl + "'");
    }
    return p->second;
}

TemplateDescriptor
Resolver::getServiceTemplate(const string& tmpl) const
{
    assert(_application);
    TemplateDescriptorDict::const_iterator p = _application->serviceTemplates.find(tmpl);
    if(p == _application->serviceTemplates.end())
    {
	throw DeploymentException("unknown service template `" + tmpl + "'");
    }
    return p->second;
}

bool
Resolver::hasReplicaGroup(const string& id) const
{
    if(!_application)
    {
	//
	// If we don't know the application descrpitor we assume that
	// the replica group exists (this is possible if the resolver
	// wasn't built from an application helper, that's the case if
	// it's built from NodeCache just to resolve ${node.*} and
	// ${session.*} variables.
	//
	return true;
    }
    ReplicaGroupDescriptorSeq::const_iterator p;
    for(p = _application->replicaGroups.begin(); p != _application->replicaGroups.end(); ++p)
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
	    throw "malformed variable name `" + value + "'";
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
    return ""; // To keep the compiler happy.
}

PropertyDescriptorSeq
Resolver::getProperties(const Ice::StringSeq& references, set<string>& resolved) const
{
    PropertyDescriptorSeq properties;
    for(Ice::StringSeq::const_iterator p = references.begin(); p != references.end(); ++p)
    {
	if(resolved.find(*p) != resolved.end())
	{
	    exception("detected circular dependency with property reference `" + *p + "'");
	}

	PropertySetDescriptor desc = getPropertySet(*p);
	if(!desc.references.empty())
	{
	    resolved.insert(*p);
	    PropertyDescriptorSeq p = getProperties(desc.references, resolved);
	    properties.insert(properties.end(), p.begin(), p.end());
	}
	
	PropertyDescriptorSeq p = operator()(desc.properties);
	properties.insert(properties.end(), p.begin(), p.end());
    }
    return properties;
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
    reserved["session.id"] = "";
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

CommunicatorHelper::CommunicatorHelper(const Ice::CommunicatorPtr& com, const CommunicatorDescriptorPtr& desc) : 
    _communicator(com),
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

    if(_desc->propertySet != helper._desc->propertySet)
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
    instance->propertySet = resolve(_desc->propertySet);

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
	adapter.objects = resolve(p->objects);
	instance->adapters.push_back(adapter);
    }

    for(DbEnvDescriptorSeq::const_iterator s = _desc->dbEnvs.begin(); s != _desc->dbEnvs.end(); ++s)
    {
	DbEnvDescriptor dbEnv;
	dbEnv.name = resolve(s->name, "database environment name", false);
	dbEnv.description = resolve(s->description, "database environment description");
	dbEnv.dbHome = resolve(s->dbHome, "database environment home directory");
	dbEnv.properties = resolve(s->properties, "database environment property");
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
    printPropertySet(out, _desc->propertySet);
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
    out << nl << "database environment `" << dbEnv.name << "'";
    if(!dbEnv.dbHome.empty() || !dbEnv.properties.empty() || !dbEnv.description.empty())
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
    out << nl << "adapter `" << adapter.name << "'";
    out << sb;
    if(!adapter.id.empty())
    {
	out << nl << "id = `" << adapter.id << "'";
    }
    if(!adapter.replicaGroupId.empty())
    {
	out << nl << "replica group id = `" << adapter.replicaGroupId << "'";
    }
    string endpoints = getProperty(adapter.name + ".Endpoints");
    if(!endpoints.empty())
    {
	out << nl << "endpoints = `" << endpoints << "'";
    }
    out << nl << "register process = `" << (adapter.registerProcess ? "true" : "false") << "'";
    out << nl << "wait for activation = `" << (adapter.waitForActivation ? "true" : "false") << "'";
    for(ObjectDescriptorSeq::const_iterator p = adapter.objects.begin(); p != adapter.objects.end(); ++p)
    {
	out << nl << "object";
	if(!p->type.empty())
	{
	    out << sb;
	    out << nl << "identity = `" << _communicator->identityToString(p->id) << "' ";
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
CommunicatorHelper::printPropertySet(Output& out, const PropertySetDescriptor& propertySet) const
{
    if(propertySet.properties.empty() && propertySet.references.empty())
    {
	return;
    }

    out << nl << "properties";
    out << sb;
    if(!propertySet.references.empty())
    {
	out << nl << "references = " << toString(propertySet.references);
    }
    PropertyDescriptorSeq::const_iterator q;
    for(q = propertySet.properties.begin(); q != propertySet.properties.end(); ++q)
    {
	out << nl << q->name << " = `" << q->value << "'";
    }
    out << eb;
}

string
CommunicatorHelper::getProperty(const string& name) const
{
    return IceGrid::getProperty(_desc->propertySet, name);
}

ServiceHelper::ServiceHelper(const Ice::CommunicatorPtr& communicator, const ServiceDescriptorPtr& descriptor) :
    CommunicatorHelper(communicator, descriptor),
    _desc(descriptor)
{
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
ServiceHelper::instantiate(const Resolver& resolver, const PropertyDescriptorSeq& props) const
{
    ServiceDescriptorPtr service = new ServiceDescriptor();
    instantiateImpl(service, resolver, props);
    return service;
}

void
ServiceHelper::instantiateImpl(const ServiceDescriptorPtr& instance, 
			       const Resolver& resolve, 
			       const PropertyDescriptorSeq& props) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);
    instance->name = resolve(_desc->name, "name", false);
    instance->entry = resolve(_desc->entry, "entry", false);
    instance->propertySet.properties.insert(instance->propertySet.properties.end(), props.begin(), props.end());
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

ServerHelper::ServerHelper(const Ice::CommunicatorPtr& communicator, const ServerDescriptorPtr& descriptor) :
    CommunicatorHelper(communicator, descriptor),
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

    if(_desc->distrib != helper._desc->distrib)
    {
	return false;
    }

    if(_desc->allocatable != helper._desc->allocatable)
    {
	return false;
    }

    if(_desc->user != helper._desc->user)
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

ServerDescriptorPtr
ServerHelper::getDescriptor() const
{
    return _desc;
}

ServerDescriptorPtr
ServerHelper::instantiate(const Resolver& resolver, const PropertyDescriptorSeq& props) const
{
    ServerDescriptorPtr server = new ServerDescriptor();
    instantiateImpl(server, resolver, props);
    return server;
}

void
ServerHelper::print(Output& out) const
{
    print(out, "", "");
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
	out << nl << "activationTimeout = `" << _desc->activationTimeout << "'";
    }
    if(!_desc->deactivationTimeout.empty() && _desc->deactivationTimeout != "0")
    {
	out << nl << "deactivationTimeout = `" << _desc->deactivationTimeout << "'";
    }
    if(!_desc->user.empty())
    {
	out << nl << "user = `" << _desc->user << "'";
    }
    if(!_desc->applicationDistrib)
    {
	out << nl << "application distribution = `false'";
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
	if(!_desc->distrib.directories.empty())
	{
	    out << nl << "directories = `" << toString(_desc->distrib.directories) << "'";
	}
	out << eb;
    }
    CommunicatorHelper::print(out);
}

void
ServerHelper::instantiateImpl(const ServerDescriptorPtr& instance, 
			      const Resolver& resolve, 
			      const PropertyDescriptorSeq& props) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);

    instance->id = resolve(_desc->id, "id", false);
    instance->exe = resolve(_desc->exe, "executable", false);
    instance->pwd = resolve(_desc->pwd, "working directory path");
    instance->activation = resolve(_desc->activation, "activation");
    instance->applicationDistrib = _desc->applicationDistrib;
    instance->allocatable = _desc->allocatable;
    instance->user = resolve(_desc->user, "user");
    if(!instance->activation.empty() && 
       instance->activation != "manual" &&
       instance->activation != "on-demand" &&
       instance->activation != "always" &&
       instance->activation != "session")
    {
	resolve.exception("unknown activation `" + instance->activation + "'");
    }
    instance->activationTimeout = resolve.asInt(_desc->activationTimeout, "activation timeout");
    instance->deactivationTimeout = resolve.asInt(_desc->deactivationTimeout, "deactivation timeout");
    instance->options = resolve(_desc->options, "option");
    instance->envs = resolve(_desc->envs, "environment variable");
    instance->distrib = resolve(_desc->distrib);
    instance->propertySet.properties.insert(instance->propertySet.properties.end(), props.begin(), props.end());
}

IceBoxHelper::IceBoxHelper(const Ice::CommunicatorPtr& communicator, const IceBoxDescriptorPtr& descriptor) :
    ServerHelper(communicator, descriptor),
    _desc(descriptor)
{
    for(ServiceInstanceDescriptorSeq::const_iterator p = _desc->services.begin(); p != _desc->services.end(); ++p)
    {
	_services.push_back(ServiceInstanceHelper(communicator, *p));
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
IceBoxHelper::instantiate(const Resolver& resolver, const PropertyDescriptorSeq& props) const
{
    IceBoxDescriptorPtr iceBox = new IceBoxDescriptor();
    instantiateImpl(iceBox, resolver, props);
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
IceBoxHelper::print(Output& out) const
{
    print(out, "", "");
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

void
IceBoxHelper::instantiateImpl(const IceBoxDescriptorPtr& instance, 
			      const Resolver& resolver, 
			      const PropertyDescriptorSeq& props) const
{
    ServerHelper::instantiateImpl(instance, resolver, props);
    for(vector<ServiceInstanceHelper>::const_iterator p = _services.begin(); p != _services.end(); ++p)
    {
	instance->services.push_back(p->instantiate(resolver));
    }
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

ServiceInstanceHelper::ServiceInstanceHelper(const Ice::CommunicatorPtr& com, const ServiceInstanceDescriptor& desc) :
    _def(desc)
{
    //
    // If the service instance is not a template instance, its
    // descriptor must be set and contain the definition of the
    // service.
    //
    if(_def._cpp_template.empty() && !_def.descriptor)
    {
	throw DeploymentException("invalid service instance: no template defined");
    }
    if(_def.descriptor)
    {
	_service = ServiceHelper(com, _def.descriptor);
    }
}

bool
ServiceInstanceHelper::operator==(const ServiceInstanceHelper& helper) const
{
    if(_def._cpp_template.empty())
    {
	return _service == helper._service;
    }
    else
    {
	return _def._cpp_template == helper._def._cpp_template && 
	    _def.parameterValues == helper._def.parameterValues &&
	    _def.propertySet == helper._def.propertySet;
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
    std::map<std::string, std::string> parameterValues;
    if(!def.getDescriptor())
    {
	assert(!_def._cpp_template.empty());
	TemplateDescriptor tmpl = resolve.getServiceTemplate(_def._cpp_template);
	def = ServiceHelper(resolve.getCommunicator(), ServiceDescriptorPtr::dynamicCast(tmpl.descriptor));
	parameterValues = instantiateParams(resolve, 
					    _def._cpp_template, 
					    _def.parameterValues,
					    tmpl.parameters, 
					    tmpl.parameterDefaults);
    }

    //
    // Setup the resolver.
    //
    Resolver svcResolve(resolve, parameterValues, !_service.getDescriptor());
    svcResolve.setReserved("service", svcResolve(def.getDescriptor()->name, "service name", false));
    svcResolve.setContext("service `${service}' from server `${server}'");

    //
    // Instantiate the service instance.
    //
    ServiceInstanceDescriptor desc;
    desc.descriptor = def.instantiate(svcResolve, svcResolve(_def.propertySet).properties);    

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
	assert(!_def._cpp_template.empty());
	out << "service instance";
	out << sb;
	out << nl << "template = `" << _def._cpp_template << "'";
	out << nl << "parameters";
	out << sb;
	for(StringStringDict::const_iterator p = _def.parameterValues.begin(); 
	    p != _def.parameterValues.end(); ++p)
	{
	    out << nl << p->first << " = `" << p->second << "'";
	}
	out << eb;
	out << eb;
    }
}

ServerInstanceHelper::ServerInstanceHelper(const ServerInstanceDescriptor& desc,
					   const Resolver& resolve,
					   bool instantiate) :
    _def(desc)
{
    init(0, resolve, instantiate);
}

ServerInstanceHelper::ServerInstanceHelper(const ServerDescriptorPtr& definition, 
					   const Resolver& resolve,
					   bool instantiate)
{
    init(definition, resolve, instantiate);
}

void
ServerInstanceHelper::init(const ServerDescriptorPtr& definition, const Resolver& resolve, bool instantiate)
{
    //
    // Get the server definition if it's not provided.
    //
    ServerDescriptorPtr def = definition;
    std::map<std::string, std::string> parameterValues;
    if(!def)
    {
	if(_def._cpp_template.empty())
	{
	    resolve.exception("invalid server instance: template is not defined");
	}
	
	//
	// Get the server definition and the template property sets.
	//
	TemplateDescriptor tmpl = resolve.getServerTemplate(_def._cpp_template);
	def = ServerDescriptorPtr::dynamicCast(tmpl.descriptor);
	parameterValues = instantiateParams(resolve, 
					    _def._cpp_template, 
					    _def.parameterValues, 
					    tmpl.parameters, 
					    tmpl.parameterDefaults);
    }
    assert(def);

    //
    // Setup the resolver.
    //
    Resolver svrResolve(resolve, parameterValues, true);
    svrResolve.setReserved("server", svrResolve(def->id, "server id", false));
    svrResolve.setContext("server `${server}'");
    _id = svrResolve("${server}");

    //
    // Set the server definition.
    //
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(def);
    if(iceBox)
    {
	_serverDefinition = new IceBoxHelper(svrResolve.getCommunicator(), iceBox);
    }
    else
    {
	_serverDefinition = new ServerHelper(resolve.getCommunicator(), def);
    }

    if(!instantiate)
    {
	return; // We're done.
    }

    //
    // Ignore undefined session.id variable if the activation mode is
    // 'session', it will get defined when the server is allocated.
    //
    if(svrResolve(def->activation, "server activation", true) == "session")
    {
	svrResolve.addIgnored("session.id"); 
    }
	
    //
    // Instantiate the server instance definition (we use the server
    // resolver above, so using parameters in properties is possible).
    //
    if(!_def._cpp_template.empty())
    {
	_instance._cpp_template = _def._cpp_template;
	_instance.parameterValues = parameterValues;
	_instance.propertySet = svrResolve(_def.propertySet);
    }
    
    //
    // Instantiate the server definition.
    //
    ServerDescriptorPtr inst = _serverDefinition->instantiate(svrResolve, _instance.propertySet.properties);
    if(iceBox)
    {
	_serverInstance = new IceBoxHelper(svrResolve.getCommunicator(), IceBoxDescriptorPtr::dynamicCast(inst));
    }
    else
    {
	_serverInstance = new ServerHelper(svrResolve.getCommunicator(), inst);
    }
}

bool
ServerInstanceHelper::operator==(const ServerInstanceHelper& helper) const
{
    if(_def._cpp_template.empty())
    {
	return *_serverDefinition == *helper._serverDefinition;
    }
    else
    {
	return _def._cpp_template == helper._def._cpp_template && 
	    _def.parameterValues == helper._def.parameterValues &&
	    _def.propertySet == helper._def.propertySet;
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
    return _id;
}

ServerInstanceDescriptor
ServerInstanceHelper::getDefinition() const
{
    assert(!_def._cpp_template.empty());
    return _def;
}

ServerInstanceDescriptor
ServerInstanceHelper::getInstance() const
{
    assert(!_def._cpp_template.empty() && !_instance._cpp_template.empty());
    return _instance;
}

ServerDescriptorPtr
ServerInstanceHelper::getServerDefinition() const
{
    assert(_def._cpp_template.empty());
    return _serverDefinition->getDescriptor();
}

ServerDescriptorPtr
ServerInstanceHelper::getServerInstance() const
{
    assert(_serverInstance);
    return _serverInstance->getDescriptor();
}

void
ServerInstanceHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_serverInstance);
    _serverInstance->getIds(adapterIds, objectIds);
}

NodeHelper::NodeHelper(const string& name, const NodeDescriptor& descriptor, const Resolver& appResolve) : 
    _name(name),
    _def(descriptor)
{
    if(_name.empty())
    {
	appResolve.exception("invalid node: empty name");
    }

    Resolver resolve(appResolve, _def.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    //
    // Instantiate the node definition.
    //
    _instance.variables = _def.variables;
    _instance.loadFactor = resolve.asFloat(_def.loadFactor, "load factor");
    _instance.description = resolve(_def.description, "description");
    _instance.propertySets = resolve(_def.propertySets);

    //
    // Set the named property sets on the resolver. We use the
    // instantiated named property sets here -- named property sets
    // must be fully definied at the node level.
    //
    resolve.addPropertySets(_instance.propertySets);

    //
    // Instantiate the node servers and server instances.
    //
    ServerInstanceDescriptorSeq::const_iterator p;
    for(p = _def.serverInstances.begin(); p != _def.serverInstances.end(); ++p)
    {
	ServerInstanceHelper helper(*p, resolve, true);
	if(!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	_instance.serverInstances.push_back(helper.getInstance());
    }

    ServerDescriptorSeq::const_iterator q;
    for(q = _def.servers.begin(); q != _def.servers.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve, true);
	if(!_servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
	_instance.servers.push_back(helper.getServerInstance());
    }

    //
    // Other misc. validation...
    //

    //
    // Validate each property set references.
    //
    PropertySetDescriptorDict::const_iterator ps;
    for(ps = _def.propertySets.begin(); ps != _def.propertySets.end(); ++ps)
    {
	resolve.getProperties(ps->second.references);
    }
}

bool
NodeHelper::operator==(const NodeHelper& helper) const
{
    if(_def.variables != helper._def.variables)
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

    if(_def.loadFactor != helper._def.loadFactor)
    {
	return false;
    }

    if(_def.description != helper._def.description)
    {
	return false;
    }

    if(_def.propertySets != helper._def.propertySets)
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
    if(_def.loadFactor != helper._def.loadFactor)
    {
	update.loadFactor = new BoxedString(_def.loadFactor);
    }

    if(_def.description != helper._def.description)
    {
	update.description = new BoxedString(_def.description);
    }

    update.variables = getDictUpdatedElts(helper._def.variables, _def.variables);
    update.removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);

    update.propertySets = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
    update.removePropertySets = getDictRemovedElts(helper._def.propertySets, _def.propertySets);
    
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

NodeDescriptor
NodeHelper::update(const NodeUpdateDescriptor& update, const Resolver& appResolve) const
{
    NodeDescriptor def = _def; // TODO: XXX deep copy ?!
    assert(update.name == _name);

    //
    // Update the variables, property sets, load factor, description.
    //
    def.variables = updateDictElts(def.variables, update.variables, update.removeVariables);
    def.propertySets = updateDictElts(def.propertySets, update.propertySets, update.removePropertySets);
    if(update.loadFactor)
    {
	def.loadFactor = update.loadFactor->value;
    }    
    if(update.description)
    {
	def.description = update.description->value;
    }
    
    //
    // NOTE: It's important to create the resolver *after* updating
    // the node variables!
    //
    Resolver resolve(appResolve, def.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node `" + _name + "'");

    //
    // Update the node servers and server instances. The update is in 2 steps:
    //
    //  * first we instantiate the servers from the update descriptor.
    //  * then we add the servers from the node which were not updated or removed.
    //

    ServerInstanceHelperDict serverInstances;
    ServerInstanceHelperDict servers;
    ServerInstanceHelperDict::const_iterator r;
    ServerInstanceDescriptorSeq::const_iterator q;

    set<string> removeServers(update.removeServers.begin(), update.removeServers.end());

    for(q = update.serverInstances.begin(); q != update.serverInstances.end(); ++q)
    {
	ServerInstanceHelper helper(*q, resolve, false);
	if(!serverInstances.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }
    for(r = _serverInstances.begin(); r != _serverInstances.end(); ++r)
    {
	if(removeServers.find(r->first) != removeServers.end() ||
	   serverInstances.find(r->first) != serverInstances.end())
	{
	    continue;
	} 

	//
	// Re-instantiate the server. Make sure the server ID didn't
	// change, if the ID of a server changes the update descriptor
	// has to remove the server and add an update entry for it.
	//
	ServerInstanceHelper helper(r->second.getDefinition(), resolve, false);
	if(helper.getId() != r->first)
	{
	    resolve.exception("invalid update in node `" + _name + "':\n" + "server instance id `" + r->first +
			      "' changed to `" + helper.getId() + "'");
	}
	serverInstances.insert(make_pair(helper.getId(), helper));
    }

    for(ServerDescriptorSeq::const_iterator s = update.servers.begin(); s != update.servers.end(); ++s)
    {
	ServerInstanceHelper helper(*s, resolve, false);
	if(!servers.insert(make_pair(helper.getId(), helper)).second)
	{
	    resolve.exception("duplicate server `" + helper.getId() + "' in node `" + _name + "'");
	}
    }    
    for(r = _servers.begin(); r != _servers.end(); ++r)
    {
	if(removeServers.find(r->first) != removeServers.end() ||
	   serverInstances.find(r->first) != serverInstances.end())
	{
	    continue;
	} 

	//
	// Re-instantiate the server. Make sure the server ID didn't
	// change, if the ID of a server changes the update descriptor
	// has to remove the server and add an update entry for it.
	//
	ServerInstanceHelper helper(r->second.getServerDefinition(), resolve, false);
	if(helper.getId() != r->first)
	{
	    resolve.exception("invalid update in node `" + _name + "':\nserver instance id `" + r->first + 
			      "' changed to `" + helper.getId() + "'");
	}	
	servers.insert(make_pair(helper.getId(), helper));
    }

    def.serverInstances.clear();
    for(r = serverInstances.begin(); r != serverInstances.end(); ++r)
    {
	def.serverInstances.push_back(r->second.getDefinition());
    }

    def.servers.clear();
    for(r = servers.begin(); r != servers.end(); ++r)
    {
	def.servers.push_back(r->second.getServerDefinition());
    }

    return def;
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
    return _def;
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

bool
NodeHelper::hasDistributions(const string& server) const
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
		return true;
	    }
	}
	for(p = _servers.begin(); p != _servers.end(); ++p)
	{
	    if(!p->second.getServerInstance()->distrib.icepatch.empty())
	    {
		return true;
	    }
	}
    }
    else
    {
	ServerInstanceHelperDict::const_iterator p = _serverInstances.find(server);
	if(p == _serverInstances.end())
	{
	    p = _servers.find(server);
	    if(p == _servers.end())
	    {
		p = _serverInstances.end();
	    }
	}
	if(p != _serverInstances.end())
	{
	    if(!p->second.getServerInstance()->distrib.icepatch.empty())
	    {
		return true;
	    }
	}
    }

    return false;
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
    out << nl << "node `" << _name << "'";
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
    if(!_instance.propertySets.empty())
    {
	PropertySetDescriptorDict::const_iterator q;
	for(q = _instance.propertySets.begin(); q != _instance.propertySets.end(); ++q)
	{
	    out << nl << "properties `" << q->first << "'";
	    out << sb;
	    if(!q->second.references.empty())
	    {
		out << nl << "references = " << toString(q->second.references);
	    }
	    PropertyDescriptorSeq::const_iterator r;
	    for(r = q->second.properties.begin(); r != q->second.properties.end(); ++r)
	    {
		out << nl << r->name << " = `" << r->value << "'";
	    }
	    out << eb;
	}
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

    map<string, string> variables = getDictUpdatedElts(helper._def.variables, _def.variables);
    Ice::StringSeq removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);

    PropertySetDescriptorDict updatedPs = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
    Ice::StringSeq removedPs = getDictRemovedElts(helper._def.propertySets, _def.propertySets);

    if(updated.empty() && removed.empty() &&
       variables.empty() && removeVariables.empty() &&
       updatedPs.empty() && removedPs.empty() &&
       _def.loadFactor == helper._def.loadFactor &&
       _def.description == helper._def.description)
    {
	return;
    }

    //
    // TODO: Show updated variables?
    //

    out << nl << "node `" + _name + "' updated";
    out << sb;

    if(_def.loadFactor != helper._def.loadFactor)
    {
	out << nl << "load factor udpated";
    }
    if(_def.description != helper._def.description)
    {
	out << nl << "description udpated";
    }
    if(!updatedPs.empty() || !removedPs.empty())
    {
	out << nl << "property sets udpated";
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

ApplicationHelper::ApplicationHelper(const Ice::CommunicatorPtr& communicator, const ApplicationDescriptor& desc) :
    _communicator(communicator),
    _def(desc)
{
    if(_def.name.empty())
    {
	throw DeploymentException("invalid application: empty name");
    }

    Resolver resolve(_def, communicator);

    //
    // Instantiate the application definition.
    //
    _instance.variables = _def.variables;
    _instance.serverTemplates = _def.serverTemplates;
    _instance.serviceTemplates = _def.serviceTemplates;
    _instance.description = resolve(_def.description, "description");
    _instance.distrib = resolve(_def.distrib);
    _instance.propertySets = resolve(_def.propertySets);

    ReplicaGroupDescriptorSeq::iterator r;
    for(r = _def.replicaGroups.begin(); r != _def.replicaGroups.end(); ++r)
    {
	ReplicaGroupDescriptor desc;
	desc.id = r->id;
	desc.description = resolve(r->description, "replica group description");
	desc.objects = resolve(r->objects);

	if(r->loadBalancing)
	{
	    desc.loadBalancing = LoadBalancingPolicyPtr::dynamicCast(r->loadBalancing->ice_clone());
	    desc.loadBalancing->nReplicas = resolve(r->loadBalancing->nReplicas, "replica group number of replicas");
	    AdaptiveLoadBalancingPolicyPtr al = AdaptiveLoadBalancingPolicyPtr::dynamicCast(desc.loadBalancing);
	    if(al)
	    {
		al->loadSample = resolve(al->loadSample, "replica group load sample");
		if(al->loadSample != "" && al->loadSample != "1" && al->loadSample != "5" && al->loadSample != "15")
		{
		    resolve.exception("invalid load sample value (allowed values are 1, 5 or 15)");
		}
	    }
	}	
	_instance.replicaGroups.push_back(desc);
    }
    
    //
    // Set the named property sets on the resolver. We use the
    // instantiated named property sets here -- named property sets
    // must be fully definied at the application level.
    //
    resolve.addPropertySets(_instance.propertySets);

    //
    // Instantiate the nodes.
    //
    _instance.nodes.clear();
    NodeHelperDict::const_iterator n;
    for(NodeDescriptorDict::const_iterator p = _def.nodes.begin(); p != _def.nodes.end(); ++p)
    {
	n = _nodes.insert(make_pair(p->first, NodeHelper(p->first, p->second, resolve))).first;
	_instance.nodes.insert(make_pair(n->first, n->second.getInstance()));
    }

    //
    // Ensure the unicity of object ids, adapter ids and server ids.
    //    
    multiset<string> serverIds;
    multiset<string> adapterIds;
    multiset<Ice::Identity> objectIds;
    for(n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	n->second.getIds(serverIds, adapterIds, objectIds);
    }

    for(r = _def.replicaGroups.begin(); r != _def.replicaGroups.end(); ++r)
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
	    resolve.exception("duplicate object `" + _communicator->identityToString(*o) + "'");
	}
    }
}

ApplicationUpdateDescriptor
ApplicationHelper::diff(const ApplicationHelper& helper) const
{
    ApplicationUpdateDescriptor updt;

    updt.name = _def.name;
    if(_def.description != helper._def.description)
    {
	updt.description = new BoxedString(_def.description);
    }

    updt.variables = getDictUpdatedElts(helper._def.variables, _def.variables);
    updt.removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);

    updt.propertySets = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
    updt.removePropertySets = getDictRemovedElts(helper._def.propertySets, _def.propertySets);

    if(_def.distrib != helper._def.distrib)
    {
	updt.distrib = new BoxedDistributionDescriptor(_def.distrib);
    }

    GetReplicaGroupId rk;
    ReplicaGroupEq req;
    updt.replicaGroups = getSeqUpdatedElts(helper._def.replicaGroups, _def.replicaGroups, rk, req);
    updt.removeReplicaGroups = getSeqRemovedElts(helper._def.replicaGroups, _def.replicaGroups, rk);

    TemplateDescriptorEqual eq(_communicator);
    updt.serverTemplates = getDictUpdatedElts(helper._def.serverTemplates, _def.serverTemplates, eq);
    updt.removeServerTemplates = getDictRemovedElts(helper._def.serverTemplates, _def.serverTemplates);
    updt.serviceTemplates = getDictUpdatedElts(helper._def.serviceTemplates, _def.serviceTemplates, eq);
    updt.removeServiceTemplates = getDictRemovedElts(helper._def.serviceTemplates, _def.serviceTemplates);

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
	    updt.nodes.push_back(nodeUpdate);
	}
	else
	{
	    updt.nodes.push_back(p->second.diff(q->second));
	}
    }
    updt.removeNodes = getDictRemovedElts(helper._nodes, _nodes);

    return updt;
}

ApplicationDescriptor
ApplicationHelper::update(const ApplicationUpdateDescriptor& updt) const
{
    ApplicationDescriptor def = _def; // TODO: Deep copy??? XXX

    if(updt.description)
    {
	def.description = updt.description->value;
    }

    def.replicaGroups =
	updateSeqElts(def.replicaGroups, updt.replicaGroups, updt.removeReplicaGroups, GetReplicaGroupId());
    
    def.variables = updateDictElts(def.variables, updt.variables, updt.removeVariables);
    def.propertySets = updateDictElts(def.propertySets, updt.propertySets, updt.removePropertySets);

    if(updt.distrib)
    {
	def.distrib = updt.distrib->value;
    }

    def.serverTemplates = updateDictElts(def.serverTemplates, updt.serverTemplates, updt.removeServerTemplates);
    def.serviceTemplates = updateDictElts(def.serviceTemplates, updt.serviceTemplates, updt.removeServiceTemplates);

    for(Ice::StringSeq::const_iterator r = updt.removeNodes.begin(); r != updt.removeNodes.end(); ++r)
    {
 	def.nodes.erase(*r);
    }

    Resolver resolve(def, _communicator); // A resolver based on the *updated* application descriptor.
    for(NodeUpdateDescriptorSeq::const_iterator p = updt.nodes.begin(); p != updt.nodes.end(); ++p)
    {
	NodeDescriptorDict::iterator q = def.nodes.find(p->name);
	if(q == def.nodes.end())
	{
	    NodeDescriptor desc;
	    desc.variables = p->variables;
	    desc.propertySets = p->propertySets;
	    desc.servers = p->servers;
	    desc.serverInstances = p->serverInstances;
	    desc.loadFactor = p->loadFactor ? p->loadFactor->value : "";
	    desc.description = p->description ? p->description->value : "";
	    def.nodes.insert(make_pair(p->name, desc));
	}
	else
	{
	    NodeHelperDict::const_iterator n = _nodes.find(q->first);
	    assert(n != _nodes.end());
	    q->second = n->second.update(*p, resolve); // The resolver is required to compute server IDs.
	}
    }

    return def;
}

ApplicationDescriptor
ApplicationHelper::instantiateServer(const string& node, const ServerInstanceDescriptor& instance) const
{
    ApplicationDescriptor def = _def; // TODO: Deep copy??? XXX
    NodeDescriptorDict::iterator q = def.nodes.find(node);
    if(q == def.nodes.end())
    {
	NodeDescriptor desc;
	desc.serverInstances.push_back(instance);
	def.nodes.insert(make_pair(node, desc));
    }
    else
    {
	q->second.serverInstances.push_back(instance);
    }    
    return def;
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
    for(r = _def.replicaGroups.begin(); r != _def.replicaGroups.end(); ++r)
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
    return _def;
}

const ApplicationDescriptor&
ApplicationHelper::getInstance() const
{
    return _instance;
}

map<string, ServerInfo>
ApplicationHelper::getServerInfos() const
{
    map<string, ServerInfo> servers;
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	n->second.getServerInfos(_def.name, servers);
    }
    return servers;
}

void
ApplicationHelper::getDistributions(DistributionDescriptor& distribution, 
				    vector<string>& nodes,
				    const string& server) const
{
    distribution = _instance.distrib;
    for(NodeHelperDict::const_iterator n = _nodes.begin(); n != _nodes.end(); ++n)
    {
	if(n->second.hasDistributions(server))
	{
	    nodes.push_back(n->first);
	    if(!server.empty())
	    {
		break;
	    }
	}
	else if(!_instance.distrib.icepatch.empty() && 
		(server.empty() && n->second.hasServers() || n->second.hasServer(server)))
	{
	    nodes.push_back(n->first);
	}
    }
}

void
ApplicationHelper::print(Output& out) const
{
    out << "application `" << _instance.name << "'";
    out << sb;
    if(!_instance.description.empty())
    {
	out << nl << "description = `" << _instance.description << "'";
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
    if(!_instance.propertySets.empty())
    {
	PropertySetDescriptorDict::const_iterator q;
	for(q = _instance.propertySets.begin(); q != _instance.propertySets.end(); ++q)
	{
	    out << nl << "properties `" << q->first << "'";
	    out << sb;
	    if(!q->second.references.empty())
	    {
		out << nl << "references = " << toString(q->second.references);
	    }
	    PropertyDescriptorSeq::const_iterator r;
	    for(r = q->second.properties.begin(); r != q->second.properties.end(); ++r)
	    {
		out << nl << r->name << " = `" << r->value << "'";
	    }
	    out << eb;
	}
    }
    if(!_instance.distrib.icepatch.empty())
    {
	out << nl << "distribution";
	out << sb;
	out << nl << "proxy = `" << _instance.distrib.icepatch << "'";
	if(!_instance.distrib.directories.empty())
	{
	    out << nl << "directories = `" << toString(_instance.distrib.directories) << "'";
	}
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
    out << "application `" << _def.name << "'";
    out << sb;

    {
	map<string, string> variables = getDictUpdatedElts(helper._def.variables, _def.variables);
	Ice::StringSeq removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);
	if(!variables.empty() || !removeVariables.empty())
	{
	    out << nl << "variables udpated";
	}    
    }
    {
	if(_def.distrib != helper._def.distrib)
	{
	    out << nl << "distribution updated";
	}
    }
    {
	PropertySetDescriptorDict updt = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
	Ice::StringSeq removed = getDictRemovedElts(helper._def.propertySets, _def.propertySets);
	if(!updt.empty() || !removed.empty())
	{
	    out << nl << "property sets udpated";
	}
    }
    {
	GetReplicaGroupId rk;
	ReplicaGroupEq req;
	ReplicaGroupDescriptorSeq updated = 
	    getSeqUpdatedElts(helper._def.replicaGroups, _def.replicaGroups, rk, req);
	Ice::StringSeq removed = getSeqRemovedElts(helper._def.replicaGroups, _def.replicaGroups, rk);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "replica groups";
	    out << sb;
	    ReplicaGroupDescriptorSeq::iterator p = updated.begin();
	    while(p != updated.end())
	    {
		ReplicaGroupDescriptorSeq::const_iterator r;
		for(r = helper._def.replicaGroups.begin(); r != helper._def.replicaGroups.end(); 
		    ++r)
		{
		    if(p->id == r->id)
		    {
			out << nl << "replica group `" << r->id << "' updated";
			p = updated.erase(p);
			break;
		    }
		}
		if(r == helper._def.replicaGroups.end())
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
	TemplateDescriptorEqual eq(_communicator);
	TemplateDescriptorDict updated;
	updated = getDictUpdatedElts(helper._def.serverTemplates, _def.serverTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._def.serverTemplates, _def.serverTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "server templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._def.serverTemplates.find(p->first) == helper._def.serverTemplates.end())
		{
		    out << nl << "server template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator q = updated.begin(); q != updated.end(); ++q)
	    {
		if(helper._def.serverTemplates.find(q->first) != helper._def.serverTemplates.end())
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
	TemplateDescriptorEqual eq(_communicator);
	TemplateDescriptorDict updated;
	updated = getDictUpdatedElts(helper._def.serviceTemplates, _def.serviceTemplates, eq);
	Ice::StringSeq removed = getDictRemovedElts(helper._def.serviceTemplates, _def.serviceTemplates);
	if(!updated.empty() || !removed.empty())
	{
	    out << nl << "service templates";
	    out << sb;
	    for(TemplateDescriptorDict::const_iterator p = updated.begin(); p != updated.end(); ++p)
	    {
		if(helper._def.serviceTemplates.find(p->first) == helper._def.serviceTemplates.end())
		{
		    out << nl << "service template `" << p->first << "' added";
		}
	    }
	    for(TemplateDescriptorDict::const_iterator q = updated.begin(); q != updated.end(); ++q)
	    {
		if(helper._def.serviceTemplates.find(q->first) != helper._def.serviceTemplates.end())
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

bool
IceGrid::descriptorEqual(const Ice::CommunicatorPtr& com, const ServerDescriptorPtr& lhs, 
			 const ServerDescriptorPtr& rhs)
{
    IceBoxDescriptorPtr lhsIceBox = IceBoxDescriptorPtr::dynamicCast(lhs);
    IceBoxDescriptorPtr rhsIceBox = IceBoxDescriptorPtr::dynamicCast(rhs);
    if(lhsIceBox && rhsIceBox)
    {
	return IceBoxHelper(com, lhsIceBox) == IceBoxHelper(com, rhsIceBox);
    }
    else if(!lhsIceBox && !rhsIceBox)
    {
	return ServerHelper(com, lhs) == ServerHelper(com, rhs);
    }
    else
    {
	return false;
    }
}
