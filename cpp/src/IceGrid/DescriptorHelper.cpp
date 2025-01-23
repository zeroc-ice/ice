// Copyright (c) ZeroC, Inc.

#include "DescriptorHelper.h"
#include "../Ice/TimeUtil.h"
#include "Ice/Ice.h"
#include "Util.h"

#include <iterator>
#include <stdexcept>

using namespace std;
using namespace Ice;
using namespace IceInternal;
using namespace IceGrid;

namespace
{
    const string& getReplicaGroupId(const ReplicaGroupDescriptor& desc) { return desc.id; }

    const string& getAdapterId(const AdapterDescriptor& desc) { return desc.id; }

    const Ice::Identity& getObjectId(const ObjectDescriptor& desc) { return desc.id; }

    template<typename GetKeyFunc, typename Seq, typename EqFunc>
    bool isSeqEqual(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq = equal_to<typename Seq::value_type>())
    {
        if (rseq.size() != lseq.size())
        {
            return false;
        }

        for (auto p = rseq.begin(); p != rseq.end(); ++p)
        {
            auto q = lseq.begin();
            for (; q != lseq.end(); ++q)
            {
                if (func(*p) == func(*q))
                {
                    break;
                }
            }
            if (q == lseq.end() || !eq(*p, *q))
            {
                return false;
            }
        }
        return true;
    }

    bool templateDescriptorEqual(const TemplateDescriptor& lhs, const TemplateDescriptor& rhs)
    {
        if (lhs.parameters != rhs.parameters)
        {
            return false;
        }

        if (lhs.parameterDefaults != rhs.parameterDefaults)
        {
            return false;
        }

        {
            auto slhs = dynamic_pointer_cast<IceBoxDescriptor>(lhs.descriptor);
            auto srhs = dynamic_pointer_cast<IceBoxDescriptor>(rhs.descriptor);
            if (slhs && srhs)
            {
                return IceBoxHelper(slhs) == IceBoxHelper(srhs);
            }
        }

        {
            auto slhs = dynamic_pointer_cast<ServerDescriptor>(lhs.descriptor);
            auto srhs = dynamic_pointer_cast<ServerDescriptor>(rhs.descriptor);
            if (slhs && srhs)
            {
                return ServerHelper(slhs) == ServerHelper(srhs);
            }
        }
        {
            auto slhs = dynamic_pointer_cast<ServiceDescriptor>(lhs.descriptor);
            auto srhs = dynamic_pointer_cast<ServiceDescriptor>(rhs.descriptor);
            if (slhs && srhs)
            {
                return ServiceHelper(slhs) == ServiceHelper(srhs);
            }
        }

        return false;
    }

    bool objectDescriptorEqual(const ObjectDescriptor& lhs, const ObjectDescriptor& rhs)
    {
        if (lhs.id != rhs.id)
        {
            return false;
        }
        if (lhs.type != rhs.type)
        {
            return false;
        }
        if (lhs.proxyOptions != rhs.proxyOptions)
        {
            return false;
        }
        return true;
    }

    bool adapterEqual(const AdapterDescriptor& lhs, const AdapterDescriptor& rhs)
    {
        if (lhs.id != rhs.id)
        {
            return false;
        }
        if (lhs.name != rhs.name)
        {
            return false;
        }
        if (lhs.description != rhs.description)
        {
            return false;
        }
        if (lhs.replicaGroupId != rhs.replicaGroupId)
        {
            return false;
        }
        if (lhs.priority != rhs.priority)
        {
            return false;
        }
        if (lhs.registerProcess != rhs.registerProcess)
        {
            return false;
        }
        if (lhs.serverLifetime != rhs.serverLifetime)
        {
            return false;
        }
        if (!isSeqEqual(lhs.objects, rhs.objects, getObjectId, objectDescriptorEqual))
        {
            return false;
        }
        if (!isSeqEqual(lhs.allocatables, rhs.allocatables, getObjectId, objectDescriptorEqual))
        {
            return false;
        }
        return true;
    }

    bool replicaGroupEqual(const ReplicaGroupDescriptor& lhs, const ReplicaGroupDescriptor& rhs)
    {
        if (lhs.id != rhs.id)
        {
            return false;
        }
        if (lhs.proxyOptions != rhs.proxyOptions)
        {
            return false;
        }
        if (lhs.filter != rhs.filter)
        {
            return false;
        }
        if (!isSeqEqual(lhs.objects, rhs.objects, getObjectId, objectDescriptorEqual))
        {
            return false;
        }
        if (lhs.loadBalancing && rhs.loadBalancing)
        {
            if (lhs.loadBalancing->ice_id() != rhs.loadBalancing->ice_id())
            {
                return false;
            }
            if (lhs.loadBalancing->nReplicas != rhs.loadBalancing->nReplicas)
            {
                return false;
            }
            auto alhs = dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(lhs.loadBalancing);
            auto arhs = dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(rhs.loadBalancing);
            if (alhs && arhs && alhs->loadSample != arhs->loadSample)
            {
                return false;
            }
        }
        else if (lhs.loadBalancing || rhs.loadBalancing)
        {
            return false;
        }

        return true;
    }

    template<typename GetKeyFunc, typename Seq> Seq getSeqUpdatedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func)
    {
        return getSeqUpdatedEltsWithEq(lseq, rseq, func, equal_to<typename Seq::value_type>());
    }

    template<typename GetKeyFunc, typename EqFunc, typename Seq>
    Seq getSeqUpdatedEltsWithEq(const Seq& lseq, const Seq& rseq, GetKeyFunc func, EqFunc eq)
    {
        Seq result;
        for (auto p = rseq.begin(); p != rseq.end(); ++p)
        {
            auto q = lseq.begin();
            for (; q != lseq.end(); ++q)
            {
                if (func(*p) == func(*q))
                {
                    break;
                }
            }
            if (q == lseq.end() || !eq(*p, *q))
            {
                result.push_back(*p);
            }
        }
        return result;
    }

    template<typename GetKeyFunc, typename Seq>
    Ice::StringSeq getSeqRemovedElts(const Seq& lseq, const Seq& rseq, GetKeyFunc func)
    {
        Ice::StringSeq removed;
        for (auto p = lseq.begin(); p != lseq.end(); ++p)
        {
            typename Seq::const_iterator q;
            for (q = rseq.begin(); q != rseq.end(); ++q)
            {
                if (func(*p) == func(*q))
                {
                    break;
                }
            }
            if (q == rseq.end())
            {
                removed.push_back(func(*p));
            }
        }
        return removed;
    }

    template<typename GetKeyFunc, typename Seq>
    Seq updateSeqElts(const Seq& seq, const Seq& update, const Ice::StringSeq& remove, GetKeyFunc func)
    {
        Seq result = update;
        set<string> removed(remove.begin(), remove.end());
        for (auto p = seq.begin(); p != seq.end(); ++p)
        {
            if (removed.find(func(*p)) == removed.end())
            {
                auto q = update.begin();
                for (; q != update.end(); ++q)
                {
                    if (func(*p) == func(*q))
                    {
                        break;
                    }
                }
                if (q == update.end())
                {
                    result.push_back(*p);
                }
            }
        }
        return result;
    }

    template<typename EqFunc, typename Dict>
    Dict getDictUpdatedEltsWithEq(const Dict& ldict, const Dict& rdict, EqFunc eq)
    {
        Dict result;
        for (auto p = rdict.begin(); p != rdict.end(); ++p)
        {
            auto q = ldict.find(p->first);
            if (q == ldict.end() || !eq(p->second, q->second))
            {
                result.insert(*p);
            }
        }
        return result;
    }

    template<typename Dict> Dict getDictUpdatedElts(const Dict& ldict, const Dict& rdict)
    {
        return getDictUpdatedEltsWithEq(ldict, rdict, equal_to<typename Dict::mapped_type>());
    }

    template<typename Dict> Ice::StringSeq getDictRemovedElts(const Dict& ldict, const Dict& rdict)
    {
        Ice::StringSeq removed;
        for (auto p = ldict.begin(); p != ldict.end(); ++p)
        {
            if (rdict.find(p->first) == rdict.end())
            {
                removed.push_back(p->first);
            }
        }
        return removed;
    }

    template<typename Dict> Dict updateDictElts(const Dict& dict, const Dict& update, const Ice::StringSeq& remove)
    {
        Dict result = dict;
        for (const auto& p : remove)
        {
            result.erase(p);
        }
        for (auto q = update.begin(); q != update.end(); ++q)
        {
            result[q->first] = q->second;
        }
        return result;
    }

    void validateProxyOptions(const Resolver& resolver, const string& proxyOptions)
    {
        if (!proxyOptions.empty())
        {
            try
            {
                resolver.getCommunicator()->stringToProxy("dummy " + proxyOptions);
            }
            catch (const Ice::ParseException& ex)
            {
                string reason = ex.what();
                size_t pos = reason.find("dummy ");
                if (pos != string::npos)
                {
                    reason = reason.replace(pos, 6, "");
                }
                resolver.exception("invalid proxy options: " + reason);
            }
        }
    }
}

Resolver::Resolver(
    const ApplicationDescriptor& app,
    const shared_ptr<Ice::Communicator>& communicator,
    bool enableWarning)
    : _application(&app),
      _communicator(communicator),
      _escape(false),
      _enableWarning(enableWarning),
      _context("application '" + app.name + "'"),
      _variables(app.variables),
      _reserved(getReserved()),
      _version(0)
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
    _ignore.insert("node.data");
    _ignore.insert("node.ice.soversion");

    //
    // Deprecated variables
    //
    _deprecated["node.datadir"] = "node.data";

    for (const auto& variable : _variables)
    {
        if (variable.first == "")
        {
            exception("empty variable name");
        }
    }

    for (const auto& serverTemplate : _application->serverTemplates)
    {
        if (serverTemplate.first == "")
        {
            exception("empty server template id");
        }
        if (!serverTemplate.second.descriptor)
        {
            exception("invalid server template '" + serverTemplate.first + "': server definition is empty");
        }

        Ice::StringSeq params = serverTemplate.second.parameters;
        sort(params.begin(), params.end());
        Ice::StringSeq wdups = params;
        Ice::StringSeq dups;
        set_difference(
            wdups.begin(),
            wdups.end(),
            params.begin(),
            unique(params.begin(), params.end()),
            back_inserter(dups));
        if (!dups.empty())
        {
            dups.erase(unique(dups.begin(), dups.end()), dups.end());
            exception("invalid server template '" + serverTemplate.first + "': duplicate parameters " + toString(dups));
        }
    }
    for (const auto& serviceTemplate : _application->serviceTemplates)
    {
        if (serviceTemplate.first == "")
        {
            exception("empty service template id");
        }
        if (!serviceTemplate.second.descriptor)
        {
            exception("invalid service template '" + serviceTemplate.first + "': service definition is empty");
        }
        Ice::StringSeq params = serviceTemplate.second.parameters;
        sort(params.begin(), params.end());
        Ice::StringSeq wdups = params;
        Ice::StringSeq dups;
        set_difference(
            wdups.begin(),
            wdups.end(),
            params.begin(),
            unique(params.begin(), params.end()),
            back_inserter(dups));
        if (!dups.empty())
        {
            dups.erase(unique(dups.begin(), dups.end()), dups.end());
            exception(
                "invalid server template '" + serviceTemplate.first + "': duplicate parameters " + toString(dups));
        }
    }
}

Resolver::Resolver(const Resolver& resolve, const map<string, string>& values, bool params)
    : _application(resolve._application),
      _communicator(resolve._communicator),
      _escape(resolve._escape),
      _enableWarning(resolve._enableWarning),
      _context(resolve._context),
      _variables(params ? resolve._variables : values),
      _parameters(!params ? resolve._parameters : values),
      _propertySets(resolve._propertySets),
      _reserved(resolve._reserved),
      _deprecated(resolve._deprecated),
      _ignore(resolve._ignore),
      _version(resolve._version)
{
    if (params)
    {
        checkReserved("parameter", values);
    }
    else
    {
        _variables.insert(resolve._variables.begin(), resolve._variables.end());
        checkReserved("variable", values);
    }

    for (const auto& variable : _variables)
    {
        if (variable.first == "")
        {
            exception("empty variable name");
        }
    }
}

Resolver::Resolver(const shared_ptr<InternalNodeInfo>& info, const shared_ptr<Ice::Communicator>& com)
    : _application(nullptr),
      _communicator(com),
      _escape(true),
      _enableWarning(false),
      _context("node '" + info->name + "'"),
      _reserved(getReserved()),
      _version(0)
{
    setReserved("node", info->name);
    setReserved("node.os", info->os);
    setReserved("node.hostname", info->hostname);
    setReserved("node.release", info->release);
    setReserved("node.version", info->version);
    setReserved("node.machine", info->machine);
    setReserved("node.datadir", info->dataDir);
    setReserved("node.data", info->dataDir);
    if (info->iceSoVersion)
    {
        setReserved("node.ice.soversion", *info->iceSoVersion);
    }
    else
    {
        // Since the node didn't supply this info, we're guessing it's a 3.7 node, but we don't know for sure.
        setReserved("node.ice.soversion", "37");
    }
}

string
Resolver::operator()(const string& value, const string& name, bool allowEmpty) const
{
    string val;
    try
    {
        val = substitute(value, true, true);
    }
    catch (const std::exception& ex)
    {
        exception("invalid value for attribute '" + name + "':\ninvalid variable '" + value + "':\n " + ex.what());
    }

    if (!allowEmpty)
    {
        if (value.empty())
        {
            exception("invalid value for attribute '" + name + "':\nempty string");
        }
        else if (val.empty())
        {
            exception("invalid value for attribute '" + name + "':\nthe value of '" + value + "' is an empty string");
        }
    }
    return val;
}

Ice::StringSeq
Resolver::operator()(const Ice::StringSeq& values, const string& name) const
{
    Ice::StringSeq result;
    for (const auto& value : values)
    {
        result.push_back(operator()(value, name));
    }
    return result;
}

PropertyDescriptorSeq
Resolver::operator()(const PropertyDescriptorSeq& properties, const string& name) const
{
    PropertyDescriptorSeq result;
    for (const auto& p : properties)
    {
        PropertyDescriptor prop;
        prop.name = operator()(p.name, name + " name");
        prop.value = operator()(p.value, name + " value");
        result.push_back(prop);
    }
    return result;
}

PropertySetDescriptorDict
Resolver::operator()(const PropertySetDescriptorDict& propertySets) const
{
    PropertySetDescriptorDict result;
    for (const auto& propertySet : propertySets)
    {
        PropertySetDescriptor desc;
        desc.references = operator()(
            propertySet.second.references,
            "property set '" + propertySet.first + "' reference");
        desc.properties = operator()(
            propertySet.second.properties,
            "property set '" + propertySet.first + "' property");
        result.insert(make_pair(propertySet.first, desc));
    }
    return result;
}

ObjectDescriptorSeq
Resolver::operator()(const ObjectDescriptorSeq& objects, const string& proxyOptions, const string& type) const
{
    ObjectDescriptorSeq result;
    for (const auto& object : objects)
    {
        ObjectDescriptor obj;
        obj.type = operator()(object.type, type + " object type");
        obj.id = operator()(object.id, type + " object identity");
        if (!object.proxyOptions.empty())
        {
            obj.proxyOptions = IceInternal::trim(operator()(object.proxyOptions, type + " object proxy options"));
        }
        else if (!proxyOptions.empty())
        {
            obj.proxyOptions = IceInternal::trim(operator()(proxyOptions, type + " object proxy options"));
        }
        validateProxyOptions(*this, obj.proxyOptions);
        result.push_back(obj);
    }
    return result;
}

Ice::Identity
Resolver::operator()(const Ice::Identity& value, const string& name) const
{
    assert(_communicator);
    string str = asId(_communicator->identityToString(value), name, false);
    Ice::Identity id = Ice::stringToIdentity(str);
    if (id.name.empty())
    {
        exception("invalid object identity '" + _communicator->identityToString(value) + "': name empty");
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
    if (!v.empty())
    {
        string::size_type beg = v.find_first_not_of(' ');
        string::size_type end = v.find_last_not_of(' ');
        v = v.substr(beg == string::npos ? 0 : beg, end == string::npos ? v.length() - 1 : end - beg + 1);

        int val;
        istringstream is(v);
        if (!(is >> val) || !is.eof())
        {
            exception("invalid value '" + value + "' for '" + name + "':\nnot an integer");
        }

        ostringstream os;
        os << val;
        v = os.str();
    }
    return v;
}

string
Resolver::asFloat(const string& value, const string& name) const
{
    string v = operator()(value, name);
    if (!v.empty())
    {
        string::size_type beg = v.find_first_not_of(' ');
        string::size_type end = v.find_last_not_of(' ');
        v = v.substr(beg == string::npos ? 0 : beg, end == string::npos ? v.length() - 1 : end - beg + 1);

        float val;
        istringstream is(v);
        if (!(is >> val) || !is.eof())
        {
            exception("invalid value '" + value + "' for '" + name + "':\nnot a float");
        }
    }
    return v;
}

string
Resolver::asId(const string& value, const string& name, bool allowEmpty) const
{
    if (!allowEmpty && value.empty())
    {
        exception("invalid value for attribute '" + name + "':\nempty string");
    }

    string val;
    try
    {
        val = substitute(value, true, false);
    }
    catch (const std::exception& ex)
    {
        exception("invalid value for attribute '" + name + "':\ninvalid variable '" + value + "':\n" + ex.what());
    }

    if (!allowEmpty && val.empty())
    {
        exception("invalid value for attribute '" + name + "':\nthe value of '" + value + "' is an empty string");
    }
    return val;
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
        _context = substitute(context, true, true);
    }
    catch (const std::exception& ex)
    {
        exception(ex.what());
    }
}

void
Resolver::addPropertySets(const PropertySetDescriptorDict& propertySets)
{
    PropertySetDescriptorDict oldPropertySets;
    oldPropertySets.swap(_propertySets);
    for (const auto& propertySet : propertySets)
    {
        if (!_propertySets.insert(propertySet).second)
        {
            exception("property set with id '" + propertySet.first + "' is already defined at this scope");
        }
    }
    _propertySets.insert(oldPropertySets.begin(), oldPropertySets.end());

    //
    // Validate the new property set references.
    //
    for (const auto& propertySet : propertySets)
    {
        [[maybe_unused]] auto _ = getProperties(propertySet.second.references);
    }
}

const PropertySetDescriptor&
Resolver::getPropertySet(const string& id) const
{
    auto p = _propertySets.find(id);
    if (p == _propertySets.end())
    {
        exception("invalid reference to property set, property set '" + id + "' doesn't exist");
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
Resolver::setVersion(const string& version)
{
    string v = operator()(version, "ice version");
    if (!v.empty())
    {
        _version = getMMVersion(v);
        if (_version < 0)
        {
            exception("invalid ice version: " + v);
        }
        else if (_version > ICE_INT_VERSION && warningEnabled())
        {
            Ice::Warning out(_communicator->getLogger());
            out << "invalid ice version: " << _version << " is superior to the IceGrid ";
            out << "registry version (" << ICE_STRING_VERSION << ")";
        }
    }
}

int
Resolver::getVersion() const
{
    return _version;
}

void
Resolver::exception(const string& reason) const
{
    throw DeploymentException(_context + ":\n" + reason);
}

TemplateDescriptor
Resolver::getServerTemplate(const string& tmpl) const
{
    assert(_application);
    auto p = _application->serverTemplates.find(tmpl);
    if (p == _application->serverTemplates.end())
    {
        throw DeploymentException("unknown server template '" + tmpl + "'");
    }
    return p->second;
}

TemplateDescriptor
Resolver::getServiceTemplate(const string& tmpl) const
{
    assert(_application);
    auto p = _application->serviceTemplates.find(tmpl);
    if (p == _application->serviceTemplates.end())
    {
        throw DeploymentException("unknown service template '" + tmpl + "'");
    }
    return p->second;
}

bool
Resolver::hasReplicaGroup(const string& id) const
{
    if (!_application)
    {
        //
        // If we don't know the application descriptor we assume that
        // the replica group exists (this is possible if the resolver
        // wasn't built from an application helper, that's the case if
        // it's built from NodeCache just to resolve ${node.*} and
        // ${session.*} variables.
        //
        return true;
    }

    for (const auto& replicaGroup : _application->replicaGroups)
    {
        if (replicaGroup.id == id)
        {
            return true;
        }
    }
    return false;
}

string
Resolver::substitute(const string& v, bool useParams, bool useIgnored) const
{
    string value(v);
    string::size_type beg = 0;
    string::size_type end = 0;

    while ((beg = value.find("${", beg)) != string::npos)
    {
        if (beg > 0 && value[beg - 1] == '$')
        {
            string::size_type escape = beg - 1;
            while (escape > 0 && value[escape - 1] == '$')
            {
                --escape;
            }

            if ((beg - escape) % 2)
            {
                if (_escape)
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

        end = value.find('}', beg);
        if (end == string::npos)
        {
            throw invalid_argument("malformed variable name '" + value + "'");
        }

        //
        // Get the name of the variable and get its value if the
        // variable is not currently ignored (in which case we do
        // nothing, the variable will be substituted later). If the
        // name referred to a parameter we don't do any recursive
        // substitution: the parameter value is computed at the point
        // of definition.
        //
        string name = value.substr(beg + 2, end - beg - 2);
        if (_ignore.find(name) != _ignore.end())
        {
            checkDeprecated(name);
            if (useIgnored)
            {
                ++beg;
                continue;
            }
            else
            {
                throw invalid_argument("use of the '" + name + "' variable is now allowed here");
            }
        }

        bool param;
        string val = getVariable(name, useParams, param);
        if (!param)
        {
            val = substitute(val, false, useIgnored); // Recursive resolution
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
    auto p = _reserved.find(name);
    if (p != _reserved.end())
    {
        checkDeprecated(name);
        if (p->second.empty())
        {
            throw invalid_argument("undefined variable '" + name + "'");
        }
        return p->second;
    }
    if (checkParams)
    {
        p = _parameters.find(name);
        if (p != _parameters.end())
        {
            param = true;
            return p->second;
        }
    }
    p = _variables.find(name);
    if (p == _variables.end())
    {
        throw invalid_argument("undefined variable '" + name + "'");
    }
    return p->second;
}

PropertyDescriptorSeq
Resolver::getProperties(const Ice::StringSeq& references, set<string>& resolved) const
{
    PropertyDescriptorSeq properties;
    for (const auto& reference : references)
    {
        if (resolved.find(reference) != resolved.end())
        {
            exception("detected circular dependency with property reference '" + reference + "'");
        }

        PropertySetDescriptor desc = getPropertySet(reference);
        if (!desc.references.empty())
        {
            resolved.insert(reference);
            PropertyDescriptorSeq q = getProperties(desc.references, resolved);
            properties.insert(properties.end(), q.begin(), q.end());
        }

        PropertyDescriptorSeq pds = operator()(desc.properties);
        properties.insert(properties.end(), pds.begin(), pds.end());
    }
    return properties;
}

map<string, string>
Resolver::getReserved()
{
    // Allowed reserved variables (reserved variables can't be overridden, in this implementation an empty reserved
    // variable is considered to be undefined (see getVariable))
    map<string, string> reserved;
    reserved["application"] = "";
    reserved["node"] = "";
    reserved["node.os"] = "";
    reserved["node.hostname"] = "";
    reserved["node.release"] = "";
    reserved["node.version"] = "";
    reserved["node.machine"] = "";
    reserved["node.datadir"] = "";
    reserved["node.data"] = "";
    reserved["node.ice.soversion"] = "";
    reserved["session.id"] = "";
    reserved["server"] = "";
    reserved["server.data"] = "${node.data}/servers/${server}/data";
    reserved["service"] = "";
    reserved["service.data"] = "${node.data}/servers/${server}/data_${service}";
    return reserved;
}

void
Resolver::checkReserved(const string& type, const map<string, string>& values) const
{
    for (const auto& value : values)
    {
        if (_reserved.find(value.first) != _reserved.end())
        {
            exception("invalid " + type + " '" + value.first + "': reserved variable name");
        }
    }
}

void
Resolver::checkDeprecated(const string& name) const
{
    if (!warningEnabled())
    {
        return;
    }

    auto q = _deprecated.find(name);
    if (q != _deprecated.end())
    {
        Ice::Warning out(_communicator->getLogger());
        out << "variable '" << name << "' is deprecated";
        if (!q->second.empty())
        {
            out << ", use '" << q->second << "' instead";
        }
    }
}

CommunicatorHelper::CommunicatorHelper(const shared_ptr<CommunicatorDescriptor>& desc, bool ignoreProps)
    : _desc(desc),
      _ignoreProps(ignoreProps)
{
}

bool
CommunicatorHelper::operator==(const CommunicatorHelper& helper) const
{
    if (_desc->ice_id() != helper._desc->ice_id())
    {
        return false;
    }

    if (_desc->description != helper._desc->description)
    {
        return false;
    }

    if (!isSeqEqual(_desc->adapters, helper._desc->adapters, getAdapterId, adapterEqual))
    {
        return false;
    }

    if (!_ignoreProps)
    {
        if (_desc->propertySet != helper._desc->propertySet)
        {
            return false;
        }
    }

    if (_desc->logs != helper._desc->logs)
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
    for (const auto& adapter : _desc->adapters)
    {
        if (!adapter.id.empty())
        {
            adapterIds.insert(adapter.id);
        }

        set<Ice::Identity> ids;
        for (const auto& object : adapter.objects)
        {
            ids.insert(object.id);
            objectIds.insert(object.id);
        }
        for (const auto& allocatable : adapter.allocatables)
        {
            if (ids.find(allocatable.id) == ids.end())
            {
                objectIds.insert(allocatable.id);
            }
            else
            {
                ids.erase(allocatable.id);
            }
        }
    }
}

void
CommunicatorHelper::getReplicaGroups(set<string>& replicaGroups) const
{
    for (const auto& adapter : _desc->adapters)
    {
        if (!adapter.replicaGroupId.empty())
        {
            replicaGroups.insert(adapter.replicaGroupId);
        }
    }
}

void
CommunicatorHelper::instantiateImpl(const shared_ptr<CommunicatorDescriptor>& instance, const Resolver& resolve) const
{
    instance->description = resolve(_desc->description, "description");
    instance->propertySet = resolve(_desc->propertySet);

    for (auto p = _desc->adapters.begin(); p != _desc->adapters.end(); ++p)
    {
        AdapterDescriptor adapter;
        adapter.name = resolve(p->name, "object adapter name", false);
        adapter.description = resolve(p->description, "object adapter description");
        adapter.id = resolve.asId(p->id, "object adapter id");
        adapter.registerProcess = p->registerProcess;
        adapter.serverLifetime = p->serverLifetime;
        adapter.replicaGroupId = resolve.asId(p->replicaGroupId, "object adapter replica group id", true);

        //
        // Don't check for unknown replica groups here. This check is
        // instead done by the database before the application is
        // added. It's legal for an OA to refer to a replica group
        // from another application.
        //
        // if(!adapter.replicaGroupId.empty() && !resolve.hasReplicaGroup(adapter.replicaGroupId))
        //{
        // resolve.exception("unknown replica group '" + adapter.replicaGroupId + "'");
        //}

        // Default proxy options to set on object descriptors if none is set.
        string proxyOptions = IceGrid::getProperty(instance->propertySet.properties, adapter.name + ".ProxyOptions");
        if (proxyOptions.empty())
        {
            string encoding;
            if (resolve.getVersion() > 0 && resolve.getVersion() < 30500)
            {
                encoding = "1.0";
            }
            else
            {
                encoding = "1.1";
            }
            encoding = IceGrid::getProperty(instance->propertySet.properties, "Ice.Default.EncodingVersion", encoding);
            proxyOptions = "-e " + encoding;
        }

        adapter.priority = resolve.asInt(p->priority, "object adapter priority");
        adapter.objects = resolve(p->objects, proxyOptions, "well-known");
        adapter.allocatables = resolve(p->allocatables, proxyOptions, "allocatable");
        instance->adapters.push_back(adapter);

        //
        // Make sure the endpoints are defined.
        //
        if (IceGrid::getProperty(instance->propertySet.properties, adapter.name + ".Endpoints").empty())
        {
            resolve.exception("invalid endpoints for adapter '" + adapter.name + "': empty string");
        }
    }

    for (auto l = _desc->logs.begin(); l != _desc->logs.end(); ++l)
    {
        instance->logs.push_back(resolve(*l, "log path", false));
    }
}

void
CommunicatorHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out) const
{
    if (!_desc->description.empty())
    {
        out << nl << "description";
        out << sb;
        out << nl << _desc->description;
        out << eb;
    }
    set<string> hiddenProperties;
    {
        for (const auto& adapter : _desc->adapters)
        {
            hiddenProperties.insert(adapter.name + ".Endpoints");
            hiddenProperties.insert(adapter.name + ".ProxyOptions");
            printObjectAdapter(communicator, out, adapter);
        }
    }
    {
        for (const auto& log : _desc->logs)
        {
            out << nl << "log '" << log << "'";
        }
    }
    if (!_desc->propertySet.properties.empty() || !_desc->propertySet.references.empty())
    {
        out << nl << "properties";
        out << sb;
        if (!_desc->propertySet.references.empty())
        {
            out << nl << "references = " << toString(_desc->propertySet.references);
        }

        for (const auto& prop : _desc->propertySet.properties)
        {
            if (hiddenProperties.find(prop.name) == hiddenProperties.end())
            {
                out << nl << prop.name << " = '" << prop.value << "'";
            }
        }
        out << eb;
    }
}

void
CommunicatorHelper::printObjectAdapter(
    const shared_ptr<Ice::Communicator>& communicator,
    Output& out,
    const AdapterDescriptor& adapter) const
{
    out << nl << "adapter '" << adapter.name << "'";
    out << sb;
    if (!adapter.id.empty())
    {
        out << nl << "id = '" << adapter.id << "'";
    }
    if (!adapter.replicaGroupId.empty())
    {
        out << nl << "replica group id = '" << adapter.replicaGroupId << "'";
    }
    if (!adapter.priority.empty())
    {
        out << nl << "priority = '" << adapter.priority << "'";
    }

    string endpoints = getProperty(adapter.name + ".Endpoints");
    if (!endpoints.empty())
    {
        out << nl << "endpoints = '" << endpoints << "'";
    }
    string proxyOptions = getProperty(adapter.name + ".ProxyOptions");
    if (!proxyOptions.empty())
    {
        out << nl << "proxy options = '" << proxyOptions << "'";
    }
    out << nl << "register process = '" << (adapter.registerProcess ? "true" : "false") << "'";
    out << nl << "server lifetime = '" << (adapter.serverLifetime ? "true" : "false") << "'";
    for (const auto& object : adapter.objects)
    {
        out << nl << "well-known object";
        out << sb;
        out << nl << "identity = '" << communicator->identityToString(object.id) << "' ";
        if (!object.type.empty())
        {
            out << nl << "type = '" << object.type << "'";
        }
        if (!object.proxyOptions.empty())
        {
            out << nl << "proxy options = '" << object.proxyOptions << "'";
        }
        out << eb;
    }
    for (const auto& allocatable : adapter.allocatables)
    {
        out << nl << "allocatable";
        out << sb;
        out << nl << "identity = '" << communicator->identityToString(allocatable.id) << "' ";
        if (!allocatable.type.empty())
        {
            out << nl << "type = '" << allocatable.type << "'";
        }
        if (!allocatable.proxyOptions.empty())
        {
            out << nl << "proxy options = '" << allocatable.proxyOptions << "'";
        }
        out << eb;
    }
    if (!adapter.description.empty())
    {
        out << nl << "description = '" << adapter.description << "'";
    }
    out << eb;
}

string
CommunicatorHelper::getProperty(const string& name) const
{
    return IceGrid::getProperty(_desc->propertySet.properties, name);
}

ServiceHelper::ServiceHelper(const shared_ptr<ServiceDescriptor>& descriptor, bool ignoreProps)
    : CommunicatorHelper(descriptor, ignoreProps),
      _desc(descriptor)
{
}

bool
ServiceHelper::operator==(const CommunicatorHelper& h) const
{
    const auto* helper = dynamic_cast<const ServiceHelper*>(&h);
    if (!helper || !CommunicatorHelper::operator==(h))
    {
        return false;
    }

    if (_desc->name != helper->_desc->name)
    {
        return false;
    }

    if (_desc->entry != helper->_desc->entry)
    {
        return false;
    }

    return true;
}

shared_ptr<ServiceDescriptor>
ServiceHelper::getDescriptor() const
{
    return _desc;
}

shared_ptr<ServiceDescriptor>
ServiceHelper::instantiate(
    const Resolver& resolver,
    const PropertyDescriptorSeq& props,
    const PropertySetDescriptorDict& serviceProps) const
{
    auto service = make_shared<ServiceDescriptor>();
    instantiateImpl(service, resolver, props, serviceProps);
    return service;
}

void
ServiceHelper::instantiateImpl(
    const shared_ptr<ServiceDescriptor>& instance,
    const Resolver& resolve,
    const PropertyDescriptorSeq& props,
    const PropertySetDescriptorDict& serviceProps) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);
    instance->name = resolve(_desc->name, "name", false);
    instance->entry = resolve(_desc->entry, "entry", false);
    instance->propertySet.properties.insert(instance->propertySet.properties.end(), props.begin(), props.end());
    auto p = serviceProps.find(instance->name);
    if (p != serviceProps.end())
    {
        instance->propertySet.properties.insert(
            instance->propertySet.properties.end(),
            p->second.properties.begin(),
            p->second.properties.end());
    }
}

void
ServiceHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out) const
{
    out << "service '" + _desc->name + "'";
    out << sb;
    out << nl << "entry = '" << _desc->entry << "'";
    CommunicatorHelper::print(communicator, out);
    out << eb;
}

ServerHelper::ServerHelper(const shared_ptr<ServerDescriptor>& descriptor, bool ignoreProps)
    : CommunicatorHelper(descriptor, ignoreProps),
      _desc(descriptor)
{
}

bool
ServerHelper::operator==(const CommunicatorHelper& h) const
{
    const auto* helper = dynamic_cast<const ServerHelper*>(&h);
    if (!helper || !CommunicatorHelper::operator==(h))
    {
        return false;
    }

    if (_desc->id != helper->_desc->id)
    {
        return false;
    }

    if (_desc->exe != helper->_desc->exe)
    {
        return false;
    }

    if (_desc->iceVersion != helper->_desc->iceVersion)
    {
        return false;
    }

    if (_desc->pwd != helper->_desc->pwd)
    {
        return false;
    }

    if (set<string>(_desc->options.begin(), _desc->options.end()) !=
        set<string>(helper->_desc->options.begin(), helper->_desc->options.end()))
    {
        return false;
    }

    if (set<string>(_desc->envs.begin(), _desc->envs.end()) !=
        set<string>(helper->_desc->envs.begin(), helper->_desc->envs.end()))
    {
        return false;
    }

    if (_desc->activation != helper->_desc->activation)
    {
        return false;
    }

    if (_desc->activationTimeout != helper->_desc->activationTimeout)
    {
        return false;
    }

    if (_desc->deactivationTimeout != helper->_desc->deactivationTimeout)
    {
        return false;
    }

    if (_desc->allocatable != helper->_desc->allocatable)
    {
        return false;
    }

    if (_desc->user != helper->_desc->user)
    {
        return false;
    }

    return true;
}

shared_ptr<ServerDescriptor>
ServerHelper::getDescriptor() const
{
    return _desc;
}

shared_ptr<ServerDescriptor>
ServerHelper::instantiate(
    const Resolver& resolver,
    const PropertyDescriptorSeq& props,
    const PropertySetDescriptorDict& serviceProps) const
{
    if (!serviceProps.empty())
    {
        resolver.exception("service property sets are only allowed in IceBox server instances");
    }

    auto server = make_shared<ServerDescriptor>();
    instantiateImpl(server, resolver, props);
    return server;
}

void
ServerHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out) const
{
    print(communicator, out, ServerInfo());
}

void
ServerHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out, const ServerInfo& info) const
{
    out << "server '" + _desc->id + "'";
    out << sb;
    printImpl(communicator, out, info);
    out << eb;
}

void
ServerHelper::printImpl(const shared_ptr<Ice::Communicator>& communicator, Output& out, const ServerInfo& info) const
{
    if (!info.application.empty())
    {
        out << nl << "application = '" << info.application << "'";
        out << nl << "application uuid = '" << info.uuid << "'";
        out << nl << "application revision = '" << info.revision << "'";
    }
    if (!info.node.empty())
    {
        out << nl << "node = '" << info.node << "'";
    }
    if (!info.sessionId.empty())
    {
        out << nl << "session id = '" << info.sessionId << "'";
    }
    out << nl << "exe = '" << _desc->exe << "'";

    if (!_desc->iceVersion.empty())
    {
        out << nl << "ice version = '" << _desc->iceVersion << "'";
    }

    if (!_desc->pwd.empty())
    {
        out << nl << "pwd = '" << _desc->pwd << "'";
    }
    out << nl << "activation = '" << _desc->activation << "'";
    if (!_desc->activationTimeout.empty() && _desc->activationTimeout != "0")
    {
        out << nl << "activationTimeout = '" << _desc->activationTimeout << "'";
    }
    if (!_desc->deactivationTimeout.empty() && _desc->deactivationTimeout != "0")
    {
        out << nl << "deactivationTimeout = '" << _desc->deactivationTimeout << "'";
    }
    if (!_desc->user.empty())
    {
        out << nl << "user = '" << _desc->user << "'";
    }
    if (!_desc->options.empty())
    {
        out << nl << "options = '" << toString(_desc->options) << "'";
    }
    if (!_desc->envs.empty())
    {
        out << nl << "envs = '" << toString(_desc->envs) << "'";
    }
    CommunicatorHelper::print(communicator, out);
}

void
ServerHelper::instantiateImpl(
    const shared_ptr<ServerDescriptor>& instance,
    const Resolver& resolve,
    const PropertyDescriptorSeq& props) const
{
    CommunicatorHelper::instantiateImpl(instance, resolve);

    instance->id = resolve.asId(_desc->id, "id", false);
    instance->exe = resolve(_desc->exe, "executable", false);
    instance->iceVersion = resolve(_desc->iceVersion, "ice version");
    instance->pwd = resolve(_desc->pwd, "working directory path");
    instance->activation = resolve(_desc->activation, "activation");
    instance->allocatable = _desc->allocatable;
    instance->user = resolve(_desc->user, "user");
    if (!instance->activation.empty() && instance->activation != "manual" && instance->activation != "on-demand" &&
        instance->activation != "always" && instance->activation != "session")
    {
        resolve.exception("unknown activation '" + instance->activation + "'");
    }
    instance->activationTimeout = resolve.asInt(_desc->activationTimeout, "activation timeout");
    instance->deactivationTimeout = resolve.asInt(_desc->deactivationTimeout, "deactivation timeout");
    instance->options = resolve(_desc->options, "option");
    instance->envs = resolve(_desc->envs, "environment variable");
    instance->propertySet.properties.insert(instance->propertySet.properties.end(), props.begin(), props.end());
}

IceBoxHelper::IceBoxHelper(const shared_ptr<IceBoxDescriptor>& descriptor, bool ignoreProps)
    : ServerHelper(descriptor, ignoreProps),
      _desc(descriptor)
{
    for (const auto& service : _desc->services)
    {
        _services.emplace_back(service, ignoreProps);
    }
}

bool
IceBoxHelper::operator==(const CommunicatorHelper& h) const
{
    const auto* helper = dynamic_cast<const IceBoxHelper*>(&h);
    if (!helper || !ServerHelper::operator==(h))
    {
        return false;
    }

    if (_services != helper->_services)
    {
        return false;
    }

    return true;
}

shared_ptr<ServerDescriptor>
IceBoxHelper::instantiate(
    const Resolver& resolver,
    const PropertyDescriptorSeq& props,
    const PropertySetDescriptorDict& serviceProps) const
{
    auto iceBox = make_shared<IceBoxDescriptor>();
    instantiateImpl(iceBox, resolver, props, serviceProps);
    return iceBox;
}

void
IceBoxHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    CommunicatorHelper::getIds(adapterIds, objectIds);
    for (const auto& service : _services)
    {
        service.getIds(adapterIds, objectIds);
    }
}

void
IceBoxHelper::getReplicaGroups(set<string>& replicaGroups) const
{
    CommunicatorHelper::getReplicaGroups(replicaGroups);
    for (const auto& service : _services)
    {
        service.getReplicaGroups(replicaGroups);
    }
}

void
IceBoxHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out) const
{
    print(communicator, out, ServerInfo());
}

void
IceBoxHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out, const ServerInfo& info) const
{
    out << "icebox '" + _desc->id + "'";
    out << sb;

    printImpl(communicator, out, info);
    out << nl << "services";
    out << sb;
    for (const auto& service : _desc->services)
    {
        assert(service.descriptor);
        out << nl << service.descriptor->name;
    }
    out << eb;
    out << eb;
}

void
IceBoxHelper::instantiateImpl(
    const shared_ptr<IceBoxDescriptor>& instance,
    const Resolver& resolver,
    const PropertyDescriptorSeq& props,
    const PropertySetDescriptorDict& serviceProps) const
{
    ServerHelper::instantiateImpl(instance, resolver, props);
    set<string> serviceNames;
    for (const auto& service : _services)
    {
        ServiceInstanceDescriptor desc = service.instantiate(resolver, serviceProps);
        assert(desc.descriptor);
        serviceNames.insert(desc.descriptor->name);
        instance->services.push_back(desc);
    }
    for (const auto& serviceProp : serviceProps)
    {
        if (serviceNames.find(serviceProp.first) == serviceNames.end())
        {
            resolver.exception("invalid service property set: service '" + serviceProp.first + "' doesn't exist");
        }
    }
}

map<string, string>
InstanceHelper::instantiateParams(
    const Resolver& resolve,
    const string& tmpl,
    const map<string, string>& parameters,
    const vector<string>& requiredParameters,
    const map<string, string>& defaults) const
{
    map<string, string> params;

    set<string> required(requiredParameters.begin(), requiredParameters.end());
    set<string> unknown;
    for (const auto& parameter : parameters)
    {
        if (required.find(parameter.first) == required.end())
        {
            unknown.insert(parameter.first);
        }
        params.insert(make_pair(parameter.first, resolve(parameter.second, "parameter '" + parameter.first + "'")));
    }
    if (!unknown.empty())
    {
        ostringstream os;
        os << "unknown parameters when instantiating '" + tmpl + "' template: ";
        copy(unknown.begin(), unknown.end(), ostream_iterator<string>(os, " "));
        resolve.exception(os.str());
    }

    set<string> missingParams;
    for (const auto& q : required)
    {
        if (params.find(q) == params.end())
        {
            auto r = defaults.find(q);
            if (r == defaults.end())
            {
                missingParams.insert(q);
            }
            else
            {
                params.insert(make_pair(r->first, resolve(r->second, "default parameter '" + r->first + "'")));
            }
        }
    }
    if (!missingParams.empty())
    {
        ostringstream os;
        os << "undefined parameters when instantiating '" + tmpl + "' template: ";
        copy(missingParams.begin(), missingParams.end(), ostream_iterator<string>(os, " "));
        resolve.exception(os.str());
    }

    return params;
}

ServiceInstanceHelper::ServiceInstanceHelper(ServiceInstanceDescriptor desc, bool ignoreProps) : _def(std::move(desc))
{
    //
    // If the service instance is not a template instance, its
    // descriptor must be set and contain the definition of the
    // service.
    //
    if (_def.templateName.empty() && !_def.descriptor)
    {
        throw DeploymentException("invalid service instance: no template defined");
    }

    if (_def.descriptor)
    {
        _service = ServiceHelper(_def.descriptor, ignoreProps);
    }
}

bool
ServiceInstanceHelper::operator==(const ServiceInstanceHelper& helper) const
{
    if (_def.templateName.empty())
    {
        return _service == helper._service;
    }
    else
    {
        return _def.templateName == helper._def.templateName && _def.parameterValues == helper._def.parameterValues &&
               _def.propertySet == helper._def.propertySet;
    }
}

bool
ServiceInstanceHelper::operator!=(const ServiceInstanceHelper& helper) const
{
    return !operator==(helper);
}

ServiceInstanceDescriptor
ServiceInstanceHelper::instantiate(const Resolver& resolve, const PropertySetDescriptorDict& serviceProps) const
{
    ServiceHelper def = _service;
    std::map<std::string, std::string> parameterValues;
    if (!def.getDescriptor())
    {
        assert(!_def.templateName.empty());
        TemplateDescriptor tmpl = resolve.getServiceTemplate(_def.templateName);
        def = ServiceHelper(dynamic_pointer_cast<ServiceDescriptor>(tmpl.descriptor));
        parameterValues = instantiateParams(
            resolve,
            _def.templateName,
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
    desc.descriptor = def.instantiate(svcResolve, svcResolve(_def.propertySet).properties, serviceProps);

    //
    // NOTE: We can't keep the following attributes in the service
    // instance otherwise the instance comparison would be based on
    // the template + parameters which would be wrong (if the template
    // changed the instance also changed.)
    //
    // desc.templateName = _template;
    // desc.parameterValues = _parameters;
    return desc;
}

void
ServiceInstanceHelper::getIds(multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_service.getDescriptor());
    _service.getIds(adapterIds, objectIds);
}

void
ServiceInstanceHelper::getReplicaGroups(set<string>& replicaGroups) const
{
    assert(_service.getDescriptor());
    _service.getReplicaGroups(replicaGroups);
}

void
ServiceInstanceHelper::print(const shared_ptr<Ice::Communicator>& communicator, Output& out) const
{
    if (_service.getDescriptor())
    {
        _service.print(communicator, out);
    }
    else
    {
        assert(!_def.templateName.empty());
        out << "service instance";
        out << sb;
        out << nl << "template = '" << _def.templateName << "'";
        out << nl << "parameters";
        out << sb;
        for (const auto& parameterValue : _def.parameterValues)
        {
            out << nl << parameterValue.first << " = '" << parameterValue.second << "'";
        }
        out << eb;
        out << eb;
    }
}

ServerInstanceHelper::ServerInstanceHelper(ServerInstanceDescriptor desc, const Resolver& resolve, bool instantiate)
    : _def(std::move(desc))
{
    init(nullptr, resolve, instantiate);
}

ServerInstanceHelper::ServerInstanceHelper(
    const shared_ptr<ServerDescriptor>& definition,
    const Resolver& resolve,
    bool instantiate)
    : _def(ServerInstanceDescriptor())
{
    init(definition, resolve, instantiate);
}

void
ServerInstanceHelper::init(const shared_ptr<ServerDescriptor>& definition, const Resolver& resolve, bool instantiate)
{
    //
    // Get the server definition if it's not provided.
    //
    auto def = definition;
    std::map<std::string, std::string> parameterValues;
    if (!def)
    {
        if (_def.templateName.empty())
        {
            resolve.exception("invalid server instance: template is not defined");
        }

        //
        // Get the server definition and the template property sets.
        //
        TemplateDescriptor tmpl = resolve.getServerTemplate(_def.templateName);
        def = dynamic_pointer_cast<ServerDescriptor>(tmpl.descriptor);
        parameterValues = instantiateParams(
            resolve,
            _def.templateName,
            _def.parameterValues,
            tmpl.parameters,
            tmpl.parameterDefaults);
    }
    assert(def);

    //
    // Setup the resolver.
    //
    Resolver svrResolve(resolve, parameterValues, true);
    svrResolve.setReserved("server", svrResolve.asId(def->id, "server id", false));
    svrResolve.setContext("server `${server}'");
    svrResolve.setVersion(def->iceVersion);
    _id = svrResolve("${server}");

    //
    // Set the server definition.
    //
    _serverDefinition = createHelper(def);

    if (!instantiate)
    {
        return; // We're done.
    }

    //
    // Ignore undefined session.id variable if the activation mode is
    // 'session', it will get defined when the server is allocated.
    //
    if (svrResolve(def->activation, "server activation", true) == "session")
    {
        svrResolve.addIgnored("session.id");
    }

    //
    // Instantiate the server instance definition (we use the server
    // resolver above, so using parameters in properties is possible).
    //
    if (!_def.templateName.empty())
    {
        _instance.templateName = _def.templateName;
        _instance.parameterValues = parameterValues;
        _instance.propertySet = svrResolve(_def.propertySet);
        for (const auto& servicePropertySet : _def.servicePropertySets)
        {
            _instance.servicePropertySets.insert(
                make_pair(svrResolve(servicePropertySet.first), svrResolve(servicePropertySet.second)));
        }
    }

    //
    // Instantiate the server definition.
    //
    shared_ptr<ServerDescriptor> inst =
        _serverDefinition->instantiate(svrResolve, _instance.propertySet.properties, _instance.servicePropertySets);
    _serverInstance = createHelper(inst);
}

bool
ServerInstanceHelper::operator==(const ServerInstanceHelper& helper) const
{
    if (_def.templateName.empty())
    {
        return *_serverDefinition == *helper._serverDefinition;
    }
    else
    {
        return _def.templateName == helper._def.templateName && _def.parameterValues == helper._def.parameterValues &&
               _def.propertySet == helper._def.propertySet &&
               _def.servicePropertySets == helper._def.servicePropertySets;
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
    assert(!_def.templateName.empty());
    return _def;
}

ServerInstanceDescriptor
ServerInstanceHelper::getInstance() const
{
    assert(!_def.templateName.empty() && !_instance.templateName.empty());
    return _instance;
}

shared_ptr<ServerDescriptor>
ServerInstanceHelper::getServerDefinition() const
{
    assert(_def.templateName.empty());
    return _serverDefinition->getDescriptor();
}

shared_ptr<ServerDescriptor>
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

void
ServerInstanceHelper::getReplicaGroups(set<string>& replicaGroups) const
{
    assert(_serverInstance);
    _serverInstance->getReplicaGroups(replicaGroups);
}

NodeHelper::NodeHelper(string name, NodeDescriptor descriptor, const Resolver& appResolve, bool instantiate)
    : _name(std::move(name)),
      _def(std::move(descriptor)),
      _instantiated(instantiate)
{
    if (_name.empty())
    {
        appResolve.exception("invalid node: empty name");
    }

    Resolver resolve(appResolve, _def.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node '" + _name + "'");

    if (instantiate)
    {
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
    }

    for (const auto& serverInstance : _def.serverInstances)
    {
        ServerInstanceHelper helper(serverInstance, resolve, instantiate);
        if (!_serverInstances.insert(make_pair(helper.getId(), helper)).second)
        {
            resolve.exception("duplicate server '" + helper.getId() + "' in node '" + _name + "'");
        }
        if (instantiate)
        {
            _instance.serverInstances.push_back(helper.getInstance());
        }
    }

    for (const auto& server : _def.servers)
    {
        ServerInstanceHelper helper(server, resolve, instantiate);
        if (!_servers.insert(make_pair(helper.getId(), helper)).second)
        {
            resolve.exception("duplicate server '" + helper.getId() + "' in node '" + _name + "'");
        }
        if (instantiate)
        {
            _instance.servers.push_back(helper.getServerInstance());
        }
    }
}

bool
NodeHelper::operator==(const NodeHelper& helper) const
{
    if (_def.variables != helper._def.variables)
    {
        return false;
    }

    if (_serverInstances != helper._serverInstances)
    {
        return false;
    }

    if (_servers != helper._servers)
    {
        return false;
    }

    if (_def.loadFactor != helper._def.loadFactor)
    {
        return false;
    }

    if (_def.description != helper._def.description)
    {
        return false;
    }

    if (_def.propertySets != helper._def.propertySets)
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
    if (_def.loadFactor != helper._def.loadFactor)
    {
        update.loadFactor = make_shared<BoxedString>(_def.loadFactor);
    }

    if (_def.description != helper._def.description)
    {
        update.description = make_shared<BoxedString>(_def.description);
    }

    update.variables = getDictUpdatedElts(helper._def.variables, _def.variables);
    update.removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);

    update.propertySets = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
    update.removePropertySets = getDictRemovedElts(helper._def.propertySets, _def.propertySets);

    ServerInstanceHelperDict updated = getDictUpdatedElts(helper._serverInstances, _serverInstances);
    for (const auto& p : updated)
    {
        update.serverInstances.push_back(p.second.getDefinition());
    }
    update.removeServers = getDictRemovedElts(helper._serverInstances, _serverInstances);

    updated = getDictUpdatedElts(helper._servers, _servers);
    for (const auto& q : updated)
    {
        update.servers.push_back(q.second.getServerDefinition());
    }
    Ice::StringSeq removed = getDictRemovedElts(helper._servers, _servers);
    update.removeServers.insert(update.removeServers.end(), removed.begin(), removed.end());
    return update;
}

NodeDescriptor
NodeHelper::update(const NodeUpdateDescriptor& update, const Resolver& appResolve) const
{
    NodeDescriptor def;
    assert(update.name == _name);

    //
    // Update the variables, property sets, load factor, description.
    //
    def.variables = updateDictElts(_def.variables, update.variables, update.removeVariables);
    def.propertySets = updateDictElts(_def.propertySets, update.propertySets, update.removePropertySets);
    def.loadFactor = update.loadFactor ? update.loadFactor->value : _def.loadFactor;
    def.description = update.description ? update.description->value : _def.description;

    //
    // NOTE: It's important to create the resolver *after* updating
    // the node variables!
    //
    Resolver resolve(appResolve, def.variables, false);
    resolve.setReserved("node", _name);
    resolve.setContext("node '" + _name + "'");

    //
    // Update the node servers and server instances. The update is in 2 steps:
    //
    //  * first we instantiate the servers from the update descriptor.
    //  * then we add the servers from the node which were not updated or removed.
    //

    set<string> added;
    set<string> removed(update.removeServers.begin(), update.removeServers.end());

    for (const auto& serverInstance : update.serverInstances)
    {
        ServerInstanceHelper helper(serverInstance, resolve, false);
        if (!added.insert(helper.getId()).second)
        {
            resolve.exception("duplicate server '" + helper.getId() + "' in node '" + _name + "'");
        }
        def.serverInstances.push_back(helper.getDefinition());
    }
    for (const auto& serverInstance : _serverInstances)
    {
        if (removed.find(serverInstance.first) != removed.end() || added.find(serverInstance.first) != added.end())
        {
            continue;
        }

        //
        // Re-instantiate the server. Make sure the server ID didn't
        // change, if the ID of a server changes the update descriptor
        // has to remove the server and add an update entry for it.
        //
        ServerInstanceHelper helper(serverInstance.second.getDefinition(), resolve, false);
        if (helper.getId() != serverInstance.first)
        {
            resolve.exception(
                "invalid update in node '" + _name + "':\n" + "server instance id '" + serverInstance.first +
                "' changed to '" + helper.getId() + "'");
        }
        def.serverInstances.push_back(helper.getDefinition());
    }

    added.clear();
    for (const auto& server : update.servers)
    {
        ServerInstanceHelper helper(server, resolve, false);
        if (!added.insert(helper.getId()).second)
        {
            resolve.exception("duplicate server '" + helper.getId() + "' in node '" + _name + "'");
        }
        def.servers.push_back(helper.getServerDefinition());
    }
    for (const auto& server : _servers)
    {
        if (removed.find(server.first) != removed.end() || added.find(server.first) != added.end())
        {
            continue;
        }

        //
        // Re-instantiate the server. Make sure the server ID didn't
        // change, if the ID of a server changes the update descriptor
        // has to remove the server and add an update entry for it.
        //
        ServerInstanceHelper helper(server.second.getServerDefinition(), resolve, false);
        if (helper.getId() != server.first)
        {
            resolve.exception(
                "invalid update in node '" + _name + "':\nserver instance id '" + server.first + "' changed to '" +
                helper.getId() + "'");
        }
        def.servers.push_back(helper.getServerDefinition());
    }
    return def;
}

void
NodeHelper::getIds(multiset<string>& serverIds, multiset<string>& adapterIds, multiset<Ice::Identity>& objectIds) const
{
    assert(_instantiated);
    for (const auto& serverInstance : _serverInstances)
    {
        serverIds.insert(serverInstance.first);
        serverInstance.second.getIds(adapterIds, objectIds);
    }
    for (const auto& server : _servers)
    {
        serverIds.insert(server.first);
        server.second.getIds(adapterIds, objectIds);
    }
}

void
NodeHelper::getReplicaGroups(set<string>& replicaGroups) const
{
    assert(_instantiated);
    for (const auto& serverInstance : _serverInstances)
    {
        serverInstance.second.getReplicaGroups(replicaGroups);
    }
    for (const auto& server : _servers)
    {
        server.second.getReplicaGroups(replicaGroups);
    }
}

const NodeDescriptor&
NodeHelper::getDefinition() const
{
    return _def;
}

const NodeDescriptor&
NodeHelper::getInstance() const
{
    assert(_instantiated);
    return _instance;
}

void
NodeHelper::getServerInfos(const string& app, const string& uuid, int revision, map<string, ServerInfo>& servers) const
{
    assert(_instantiated);
    for (const auto& serverInstance : _serverInstances)
    {
        ServerInfo info;
        info.node = _name;
        info.application = app;
        info.uuid = uuid;
        info.revision = revision;
        info.descriptor = serverInstance.second.getServerInstance();
        servers.insert(make_pair(serverInstance.second.getId(), info));
    }
    for (const auto& server : _servers)
    {
        ServerInfo info;
        info.node = _name;
        info.application = app;
        info.uuid = uuid;
        info.revision = revision;
        info.descriptor = server.second.getServerInstance();
        servers.insert(make_pair(server.second.getId(), info));
    }
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
    assert(_instantiated);

    out << nl << "node '" << _name << "'";
    out << sb;
    if (!_instance.loadFactor.empty())
    {
        out << nl << "load factor = '" << _instance.loadFactor << "'";
    }
    if (!_instance.description.empty())
    {
        out << nl << "description = '" << _instance.description << "'";
    }
    if (!_instance.variables.empty())
    {
        out << nl << "variables";
        out << sb;
        for (const auto& variable : _instance.variables)
        {
            out << nl << variable.first << " = '" << variable.second << "'";
        }
        out << eb;
    }
    if (!_instance.propertySets.empty())
    {
        for (const auto& propertySet : _instance.propertySets)
        {
            out << nl << "properties '" << propertySet.first << "'";
            out << sb;
            if (!propertySet.second.references.empty())
            {
                out << nl << "references = " << toString(propertySet.second.references);
            }
            PropertyDescriptorSeq::const_iterator r;
            for (r = propertySet.second.properties.begin(); r != propertySet.second.properties.end(); ++r)
            {
                out << nl << r->name << " = '" << r->value << "'";
            }
            out << eb;
        }
    }

    if (_serverInstances.empty() && _servers.empty())
    {
        out << eb;
        return;
    }

    out << nl << "servers";
    out << sb;
    for (const auto& serverInstance : _serverInstances)
    {
        out << nl << serverInstance.first;
    }
    for (const auto& server : _servers)
    {
        out << nl << server.first;
    }
    out << eb;
    out << eb;
}

void
NodeHelper::printDiff(Output& out, const NodeHelper& helper) const
{
    assert(_instantiated);

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

    if (updated.empty() && removed.empty() && variables.empty() && removeVariables.empty() && updatedPs.empty() &&
        removedPs.empty() && _def.loadFactor == helper._def.loadFactor && _def.description == helper._def.description)
    {
        return;
    }

    //
    // TODO: Show updated variables?
    //

    out << nl << "node '" + _name + "' updated";
    out << sb;

    if (_def.loadFactor != helper._def.loadFactor)
    {
        out << nl << "load factor updated";
    }
    if (_def.description != helper._def.description)
    {
        out << nl << "description updated";
    }
    if (!updatedPs.empty() || !removedPs.empty())
    {
        out << nl << "property sets updated";
    }
    if (!variables.empty() || !removeVariables.empty())
    {
        out << nl << "variables updated";
    }
    if (!updated.empty() || !removed.empty())
    {
        out << nl << "servers";
        out << sb;
        for (const auto& p : updated)
        {
            if (helper._serverInstances.find(p.first) == helper._serverInstances.end() &&
                helper._servers.find(p.first) == helper._servers.end())
            {
                out << nl << "server '" << p.first << "' added";
            }
        }
        for (const auto& p : updated)
        {
            if (helper._serverInstances.find(p.first) != helper._serverInstances.end() ||
                helper._servers.find(p.first) != helper._servers.end())
            {
                out << nl << "server '" << p.first << "' updated";
            }
        }
        for (const auto& q : removed)
        {
            out << nl << "server '" << q << "' removed";
        }
        out << eb;
    }
    out << eb;
}

ApplicationHelper::ApplicationHelper(
    const shared_ptr<Ice::Communicator>& communicator,
    ApplicationDescriptor appDesc,
    bool enableWarning,
    bool instantiate)
    : _communicator(communicator),
      _def(std::move(appDesc))
{
    if (_def.name.empty())
    {
        throw DeploymentException("invalid application: empty name");
    }

    Resolver resolve(_def, communicator, enableWarning);

    if (instantiate)
    {
        //
        // Instantiate the application definition.
        //
        _instance.name = _def.name;
        _instance.variables = _def.variables;
        _instance.serverTemplates = _def.serverTemplates;
        _instance.serviceTemplates = _def.serviceTemplates;
        _instance.description = resolve(_def.description, "description");
        _instance.propertySets = resolve(_def.propertySets);

        for (const auto& replicaGroup : _def.replicaGroups)
        {
            ReplicaGroupDescriptor desc;
            desc.id = resolve.asId(replicaGroup.id, "replica group id", false);
            desc.description = resolve(replicaGroup.description, "replica group description");
            desc.proxyOptions = resolve(replicaGroup.proxyOptions, "replica group proxy options");
            validateProxyOptions(resolve, desc.proxyOptions);
            desc.filter = resolve(replicaGroup.filter, "replica group filter");
            desc.objects = resolve(replicaGroup.objects, replicaGroup.proxyOptions, "replica group well-known");
            if (!replicaGroup.loadBalancing)
            {
                resolve.exception("replica group load balancing is not set");
            }
            desc.loadBalancing = dynamic_pointer_cast<LoadBalancingPolicy>(replicaGroup.loadBalancing->ice_clone());
            desc.loadBalancing->nReplicas =
                resolve.asInt(replicaGroup.loadBalancing->nReplicas, "replica group number of replicas");
            if (desc.loadBalancing->nReplicas.empty())
            {
                resolve.exception("invalid replica group load balancing number of replicas value: empty value");
            }
            else if (desc.loadBalancing->nReplicas[0] == '-')
            {
                resolve.exception("invalid replica group load balancing number of replicas value: inferior to 0");
            }
            auto al = dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(desc.loadBalancing);
            if (al)
            {
                al->loadSample = resolve(al->loadSample, "replica group load sample");
                if (al->loadSample != "" && al->loadSample != "1" && al->loadSample != "5" && al->loadSample != "15")
                {
                    resolve.exception("invalid load sample value (allowed values are 1, 5 or 15)");
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
    }

    //
    // Create the node helpers.
    //
    NodeHelperDict::const_iterator n;
    for (const auto& node : _def.nodes)
    {
        n = _nodes.insert(make_pair(node.first, NodeHelper(node.first, node.second, resolve, instantiate))).first;
        if (instantiate)
        {
            _instance.nodes.insert(make_pair(n->first, n->second.getInstance()));
        }
    }

    //
    // If the application is instantiated, ensure the unicity of
    // object ids, adapter ids and server ids.
    //
    if (instantiate)
    {
        multiset<string> serverIds;
        multiset<string> adapterIds;
        multiset<Ice::Identity> objectIds;
        for (n = _nodes.begin(); n != _nodes.end(); ++n)
        {
            n->second.getIds(serverIds, adapterIds, objectIds);
        }

        for (const auto& replicaGroup : _def.replicaGroups)
        {
            if (replicaGroup.id.empty())
            {
                throw DeploymentException("replica group id is empty");
            }
            if (adapterIds.find(replicaGroup.id) != adapterIds.end())
            {
                throw DeploymentException("duplicate replica group '" + replicaGroup.id + "'");
            }
            adapterIds.insert(replicaGroup.id);
            for (const auto& object : replicaGroup.objects)
            {
                objectIds.insert(object.id);
            }
        }

        for (auto s = serverIds.begin(); s != serverIds.end(); ++s)
        {
            if (serverIds.count(*s) > 1)
            {
                resolve.exception("duplicate server '" + *s + "'");
            }
        }
        for (auto a = adapterIds.begin(); a != adapterIds.end(); ++a)
        {
            if (adapterIds.count(*a) > 1)
            {
                resolve.exception("duplicate adapter '" + *a + "'");
            }
        }
        for (auto o = objectIds.begin(); o != objectIds.end(); ++o)
        {
            if (objectIds.count(*o) > 1)
            {
                resolve.exception("duplicate object '" + _communicator->identityToString(*o) + "'");
            }
        }
    }
}

ApplicationUpdateDescriptor
ApplicationHelper::diff(const ApplicationHelper& helper) const
{
    ApplicationUpdateDescriptor updt;
    assert(helper._def.name == _def.name);

    updt.name = _def.name;
    if (_def.description != helper._def.description)
    {
        updt.description = make_shared<BoxedString>(_def.description);
    }

    updt.variables = getDictUpdatedElts(helper._def.variables, _def.variables);
    updt.removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);

    updt.propertySets = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
    updt.removePropertySets = getDictRemovedElts(helper._def.propertySets, _def.propertySets);

    updt.replicaGroups =
        getSeqUpdatedEltsWithEq(helper._def.replicaGroups, _def.replicaGroups, getReplicaGroupId, replicaGroupEqual);
    updt.removeReplicaGroups = getSeqRemovedElts(helper._def.replicaGroups, _def.replicaGroups, getReplicaGroupId);

    updt.serverTemplates =
        getDictUpdatedEltsWithEq(helper._def.serverTemplates, _def.serverTemplates, templateDescriptorEqual);
    updt.removeServerTemplates = getDictRemovedElts(helper._def.serverTemplates, _def.serverTemplates);
    updt.serviceTemplates =
        getDictUpdatedEltsWithEq(helper._def.serviceTemplates, _def.serviceTemplates, templateDescriptorEqual);
    updt.removeServiceTemplates = getDictRemovedElts(helper._def.serviceTemplates, _def.serviceTemplates);

    NodeHelperDict updated = getDictUpdatedElts(helper._nodes, _nodes);
    for (const auto& p : updated)
    {
        auto q = helper._nodes.find(p.first);
        if (q == helper._nodes.end())
        {
            NodeUpdateDescriptor nodeUpdate;
            const NodeDescriptor& node = p.second.getDefinition();
            nodeUpdate.name = p.first;
            nodeUpdate.variables = node.variables;
            nodeUpdate.servers = node.servers;
            nodeUpdate.serverInstances = node.serverInstances;
            nodeUpdate.loadFactor = make_shared<BoxedString>(node.loadFactor);
            nodeUpdate.description = make_shared<BoxedString>(node.description);
            updt.nodes.push_back(nodeUpdate);
        }
        else
        {
            updt.nodes.push_back(p.second.diff(q->second));
        }
    }
    updt.removeNodes = getDictRemovedElts(helper._nodes, _nodes);

    return updt;
}

ApplicationDescriptor
ApplicationHelper::update(const ApplicationUpdateDescriptor& updt) const
{
    ApplicationDescriptor def;

    def.name = _def.name;
    def.description = updt.description ? updt.description->value : _def.description;
    def.replicaGroups =
        updateSeqElts(_def.replicaGroups, updt.replicaGroups, updt.removeReplicaGroups, getReplicaGroupId);
    def.variables = updateDictElts(_def.variables, updt.variables, updt.removeVariables);
    def.propertySets = updateDictElts(_def.propertySets, updt.propertySets, updt.removePropertySets);
    def.serverTemplates = updateDictElts(_def.serverTemplates, updt.serverTemplates, updt.removeServerTemplates);
    def.serviceTemplates = updateDictElts(_def.serviceTemplates, updt.serviceTemplates, updt.removeServiceTemplates);

    Resolver resolve(def, _communicator, false); // A resolver based on the *updated* application descriptor.
    for (const auto& node : updt.nodes)
    {
        auto q = _nodes.find(node.name);
        if (q != _nodes.end()) // Updated node
        {
            NodeDescriptor desc = q->second.update(node, resolve);
            def.nodes.insert(make_pair(node.name, q->second.update(node, resolve)));
        }
        else // New node
        {
            NodeDescriptor desc;
            desc.variables = node.variables;
            if (!node.removeVariables.empty())
            {
                resolve.exception("can't remove variables for node '" + node.name + "': node doesn't exist");
            }
            desc.propertySets = node.propertySets;
            if (!node.removePropertySets.empty())
            {
                resolve.exception("can't remove property sets for node '" + node.name + "': node doesn't exist");
            }
            desc.servers = node.servers;
            desc.serverInstances = node.serverInstances;
            if (!node.removeServers.empty())
            {
                resolve.exception("can't remove servers for node '" + node.name + "': node doesn't exist");
            }
            desc.loadFactor = node.loadFactor ? node.loadFactor->value : string("");
            desc.description = node.description ? node.description->value : string("");
            def.nodes.insert(make_pair(node.name, desc));
        }
    }
    set<string> removedNodes(updt.removeNodes.begin(), updt.removeNodes.end());
    for (const auto& node : _nodes)
    {
        if (removedNodes.find(node.first) != removedNodes.end() || def.nodes.find(node.first) != def.nodes.end())
        {
            continue; // Node was removed or updated.
        }
        def.nodes.insert(make_pair(node.first, node.second.getDefinition()));
    }

    return def;
}

ApplicationDescriptor
ApplicationHelper::instantiateServer(const string& node, const ServerInstanceDescriptor& instance) const
{
    //
    // Copy this application descriptor definition and add a server
    // instance to the given node. The caller should then construct
    // an application helper with the new application definition to
    // ensure it's valid.
    //
    ApplicationDescriptor def = _def;
    auto q = def.nodes.find(node);
    if (q == def.nodes.end())
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

    for (const auto& node : _nodes)
    {
        node.second.getIds(sIds, aIds, oIds);
    }
    for (const auto& replicaGroup : _def.replicaGroups)
    {
        aIds.insert(replicaGroup.id);
        for (const auto& object : replicaGroup.objects)
        {
            oIds.insert(object.id);
        }
    }

    copy(sIds.begin(), sIds.end(), inserter(serverIds, serverIds.begin()));
    copy(aIds.begin(), aIds.end(), inserter(adapterIds, adapterIds.begin()));
    copy(oIds.begin(), oIds.end(), inserter(objectIds, objectIds.begin()));
}

void
ApplicationHelper::getReplicaGroups(set<string>& replicaGroups, set<string>& adapterReplicaGroups) const
{
    for (const auto& replicaGroup : _def.replicaGroups)
    {
        replicaGroups.insert(replicaGroup.id);
    }

    set<string> allAdapterReplicaGroups;
    for (const auto& node : _nodes)
    {
        node.second.getReplicaGroups(allAdapterReplicaGroups);
    }

    //
    // Only return references to replica groups which don't belong to
    // this application.
    //
    set_difference(
        allAdapterReplicaGroups.begin(),
        allAdapterReplicaGroups.end(),
        replicaGroups.begin(),
        replicaGroups.end(),
        set_inserter(adapterReplicaGroups));
}

const ApplicationDescriptor&
ApplicationHelper::getDefinition() const
{
    return _def;
}

const ApplicationDescriptor&
ApplicationHelper::getInstance() const
{
    assert(!_instance.name.empty());
    return _instance;
}

map<string, ServerInfo>
ApplicationHelper::getServerInfos(const string& uuid, int revision) const
{
    assert(!_instance.name.empty());

    map<string, ServerInfo> servers;
    for (const auto& node : _nodes)
    {
        node.second.getServerInfos(_def.name, uuid, revision, servers);
    }
    return servers;
}

void
ApplicationHelper::print(Output& out, const ApplicationInfo& info) const
{
    assert(!_instance.name.empty());

    out << "application '" << _instance.name << "'";
    out << sb;
    out << nl << "uuid = '" << info.uuid << "'";
    out << nl << "revision = '" << info.revision << "'";
    out << nl << "creation time = '"
        << timePointToDateTimeString(chrono::system_clock::time_point(chrono::milliseconds(info.createTime))) << "'";
    out << nl << "created by = '" << info.createUser << "'";
    out << nl << "update time = '"
        << timePointToDateTimeString(chrono::system_clock::time_point(chrono::milliseconds(info.updateTime))) << "'";
    out << nl << "updated by = '" << info.updateUser << "'";

    if (!_instance.description.empty())
    {
        out << nl << "description = '" << _instance.description << "'";
    }
    if (!_instance.variables.empty())
    {
        out << nl << "variables";
        out << sb;
        for (const auto& variable : _instance.variables)
        {
            out << nl << variable.first << " = '" << variable.second << "'";
        }
        out << eb;
    }
    if (!_instance.propertySets.empty())
    {
        for (const auto& propertySet : _instance.propertySets)
        {
            out << nl << "properties '" << propertySet.first << "'";
            out << sb;
            if (!propertySet.second.references.empty())
            {
                out << nl << "references = " << toString(propertySet.second.references);
            }
            for (const auto& prop : propertySet.second.properties)
            {
                out << nl << prop.name << " = '" << prop.value << "'";
            }
            out << eb;
        }
    }
    if (!_instance.replicaGroups.empty())
    {
        out << nl << "replica groups";
        out << sb;
        for (const auto& replicaGroup : _instance.replicaGroups)
        {
            out << nl << "id = '" << replicaGroup.id << "' load balancing = '";
            if (!replicaGroup.loadBalancing)
            {
                out << "default (return all endpoints)";
            }
            else if (dynamic_pointer_cast<RandomLoadBalancingPolicy>(replicaGroup.loadBalancing))
            {
                out << "random";
            }
            else if (dynamic_pointer_cast<RoundRobinLoadBalancingPolicy>(replicaGroup.loadBalancing))
            {
                out << "round-robin";
            }
            else if (dynamic_pointer_cast<AdaptiveLoadBalancingPolicy>(replicaGroup.loadBalancing))
            {
                out << "adaptive";
            }
            else
            {
                out << "<unknown load balancing policy>";
            }
            if (!replicaGroup.proxyOptions.empty())
            {
                out << nl << "proxy options = '" << replicaGroup.proxyOptions << "'";
            }
            if (!replicaGroup.filter.empty())
            {
                out << nl << "filter = '" << replicaGroup.filter << "'";
            }
            out << "'";
        }
        out << eb;
    }
    if (!_instance.serverTemplates.empty())
    {
        out << nl << "server templates";
        out << sb;
        for (const auto& serverTemplate : _instance.serverTemplates)
        {
            out << nl << serverTemplate.first;
        }
        out << eb;
    }
    if (!_instance.serviceTemplates.empty())
    {
        out << nl << "service templates";
        out << sb;
        for (const auto& serviceTemplate : _instance.serviceTemplates)
        {
            out << nl << serviceTemplate.first;
        }
        out << eb;
    }
    if (!_nodes.empty())
    {
        for (const auto& node : _nodes)
        {
            node.second.print(out);
        }
    }
    out << eb;
}

void
ApplicationHelper::printDiff(Output& out, const ApplicationHelper& helper) const
{
    assert(!_instance.name.empty());

    out << "application '" << _def.name << "'";
    out << sb;

    {
        map<string, string> variables = getDictUpdatedElts(helper._def.variables, _def.variables);
        Ice::StringSeq removeVariables = getDictRemovedElts(helper._def.variables, _def.variables);
        if (!variables.empty() || !removeVariables.empty())
        {
            out << nl << "variables updated";
        }
    }
    {
        PropertySetDescriptorDict updt = getDictUpdatedElts(helper._def.propertySets, _def.propertySets);
        Ice::StringSeq removed = getDictRemovedElts(helper._def.propertySets, _def.propertySets);
        if (!updt.empty() || !removed.empty())
        {
            out << nl << "property sets updated";
        }
    }
    {
        ReplicaGroupDescriptorSeq updated = getSeqUpdatedEltsWithEq(
            helper._def.replicaGroups,
            _def.replicaGroups,
            getReplicaGroupId,
            replicaGroupEqual);
        Ice::StringSeq removed = getSeqRemovedElts(helper._def.replicaGroups, _def.replicaGroups, getReplicaGroupId);
        if (!updated.empty() || !removed.empty())
        {
            out << nl << "replica groups";
            out << sb;

            for (auto p = updated.begin(); p != updated.end();)
            {
                ReplicaGroupDescriptorSeq::const_iterator r;
                for (r = helper._def.replicaGroups.begin(); r != helper._def.replicaGroups.end(); ++r)
                {
                    if (p->id == r->id)
                    {
                        out << nl << "replica group '" << r->id << "' updated";
                        p = updated.erase(p);
                        break;
                    }
                }
                if (r == helper._def.replicaGroups.end())
                {
                    ++p;
                }
            }
            for (const auto& p : updated)
            {
                out << nl << "replica group '" << p.id << "' added";
            }
            for (const auto& q : removed)
            {
                out << nl << "replica group '" << q << "' removed";
            }
            out << eb;
        }
    }

    {
        TemplateDescriptorDict updated;
        updated = getDictUpdatedEltsWithEq(helper._def.serverTemplates, _def.serverTemplates, templateDescriptorEqual);
        Ice::StringSeq removed = getDictRemovedElts(helper._def.serverTemplates, _def.serverTemplates);
        if (!updated.empty() || !removed.empty())
        {
            out << nl << "server templates";
            out << sb;
            for (const auto& p : updated)
            {
                if (helper._def.serverTemplates.find(p.first) == helper._def.serverTemplates.end())
                {
                    out << nl << "server template '" << p.first << "' added";
                }
            }
            for (const auto& q : updated)
            {
                if (helper._def.serverTemplates.find(q.first) != helper._def.serverTemplates.end())
                {
                    out << nl << "server template '" << q.first << "' updated";
                }
            }
            for (const auto& r : removed)
            {
                out << nl << "server template '" << r << "' removed";
            }
            out << eb;
        }
    }
    {
        TemplateDescriptorDict updated;
        updated =
            getDictUpdatedEltsWithEq(helper._def.serviceTemplates, _def.serviceTemplates, templateDescriptorEqual);
        Ice::StringSeq removed = getDictRemovedElts(helper._def.serviceTemplates, _def.serviceTemplates);
        if (!updated.empty() || !removed.empty())
        {
            out << nl << "service templates";
            out << sb;
            for (const auto& p : updated)
            {
                if (helper._def.serviceTemplates.find(p.first) == helper._def.serviceTemplates.end())
                {
                    out << nl << "service template '" << p.first << "' added";
                }
            }
            for (const auto& q : updated)
            {
                if (helper._def.serviceTemplates.find(q.first) != helper._def.serviceTemplates.end())
                {
                    out << nl << "service template '" << q.first << "' updated";
                }
            }
            for (const auto& r : removed)
            {
                out << nl << "service template '" << r << "' removed";
            }
            out << eb;
        }
    }
    {
        NodeHelperDict updated = getDictUpdatedElts(helper._nodes, _nodes);
        Ice::StringSeq removed = getDictRemovedElts(helper._nodes, _nodes);
        if (!updated.empty() || !removed.empty())
        {
            out << nl << "nodes";
            out << sb;
            for (const auto& p : updated)
            {
                auto q = helper._nodes.find(p.first);
                if (q == helper._nodes.end())
                {
                    p.second.print(out);
                }
            }
            for (const auto& r : updated)
            {
                auto q = helper._nodes.find(r.first);
                if (q != helper._nodes.end())
                {
                    r.second.printDiff(out, q->second);
                }
            }
            for (const auto& s : removed)
            {
                out << nl << "node '" << s << "' removed";
            }
            out << eb;
        }
    }
    out << eb;
}

bool
IceGrid::descriptorEqual(
    const shared_ptr<ServerDescriptor>& lhs,
    const shared_ptr<ServerDescriptor>& rhs,
    bool ignoreProps)
{
    auto lhsIceBox = dynamic_pointer_cast<IceBoxDescriptor>(lhs);
    auto rhsIceBox = dynamic_pointer_cast<IceBoxDescriptor>(rhs);
    if (lhsIceBox && rhsIceBox)
    {
        return IceBoxHelper(lhsIceBox, ignoreProps) == IceBoxHelper(rhsIceBox, ignoreProps);
    }
    else if (!lhsIceBox && !rhsIceBox)
    {
        return ServerHelper(lhs, ignoreProps) == ServerHelper(rhs, ignoreProps);
    }
    else
    {
        return false;
    }
}

shared_ptr<ServerHelper>
IceGrid::createHelper(const shared_ptr<ServerDescriptor>& desc)
{
    auto iceBox = dynamic_pointer_cast<IceBoxDescriptor>(desc);
    if (iceBox)
    {
        return make_shared<IceBoxHelper>(iceBox);
    }
    else
    {
        return make_shared<ServerHelper>(desc);
    }
}

bool
IceGrid::isServerUpdated(const ServerInfo& lhs, const ServerInfo& rhs, bool ignoreProps)
{
    if (lhs.node != rhs.node)
    {
        return true;
    }
    return !descriptorEqual(lhs.descriptor, rhs.descriptor, ignoreProps);
}
