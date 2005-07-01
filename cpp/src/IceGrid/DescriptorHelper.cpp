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
using namespace IceGrid;

namespace IceGrid
{

struct Substitute : unary_function<string&, void>
{
    Substitute(const DescriptorVariablesPtr& variables, set<string>& missing) : 
	_variables(variables), _missing(missing)
    {
    } 

    void operator()(string& v)
    {
	v.assign(_variables->substituteWithMissing(v, _missing));
    }

    const DescriptorVariablesPtr& _variables;
    set<string>& _missing;
};

}

DescriptorVariables::DescriptorVariables()
{
}

DescriptorVariables::DescriptorVariables(const map<string, string>& variables)
{
    reset(variables, vector<string>());
}

string 
DescriptorVariables::substitute(const string& v)
{
    set<string> missing;
    string value = substituteImpl(v, missing);
    if(!missing.empty())
    {
	if(missing.size() == 1)
	{
	    throw "unknown variable `" + *missing.begin() + "'";
	}
	else
	{
	    ostringstream os;
	    os << "unknown variables: ";
	    copy(missing.begin(), missing.end(), ostream_iterator<string>(os, " "));
	    throw os.str();
	}
    }
    return value;
}

string
DescriptorVariables::substituteWithMissing(const string& v, set<string>& missing)
{
    return substituteImpl(v, missing);
}

void
DescriptorVariables::dumpVariables() const
{
    vector<VariableScope>::const_reverse_iterator p = _scopes.rbegin();
    while(p != _scopes.rend())
    {
	for(map<string, string>::const_iterator q = p->variables.begin(); q != p->variables.end(); ++q)
	{
	    cout << q->first << " = " << q->second << endl;
	}
	++p;
    }
}

string
DescriptorVariables::getVariable(const string& name)
{
    static const string empty;
    vector<VariableScope>::reverse_iterator p = _scopes.rbegin();
    while(p != _scopes.rend())
    {
	map<string, string>::const_iterator q = p->variables.find(name);
	if(q != p->variables.end())
	{
	    p->used.insert(name);
	    return q->second;
	}
	++p;
    }
    return empty;
}

bool
DescriptorVariables::hasVariable(const string& name) const
{
    vector<VariableScope>::const_reverse_iterator p = _scopes.rbegin();
    while(p != _scopes.rend())
    {
	map<string, string>::const_iterator q = p->variables.find(name);
	if(q != p->variables.end())
	{
	    return true;
	}
	++p;
    }
    return false;
}

void
DescriptorVariables::addVariable(const string& name, const string& value)
{
    if(_scopes.back().parameters.find(name) != _scopes.back().parameters.end())
    {
	throw "can't define variable `" + name + "': a parameter with the same was previously defined";
    }
    if(_scopes.back().used.find(name) != _scopes.back().used.end())
    {
	throw "can't redefine variable `" + name + "' after its use";
    }
    _scopes.back().variables[name] = value;
}

void
DescriptorVariables::remove(const string& name)
{
    _scopes.back().variables.erase(name);
}

void
DescriptorVariables::reset(const map<string, string>& vars, const vector<string>& targets)
{
    _scopes.clear();
    push(vars);

    _deploymentTargets = targets;
}

void
DescriptorVariables::push(const map<string, string>& vars)
{
    VariableScope scope;
    if(!_scopes.empty())
    {
	scope.substitution = _scopes.back().substitution;
    }
    else
    {
	scope.substitution = true;
    }
    scope.variables = vars;
    _scopes.push_back(scope);
}

void
DescriptorVariables::push()
{
    push(map<string, string>());
}

void
DescriptorVariables::pop()
{
    _scopes.pop_back();
}

map<string, string>
DescriptorVariables::getCurrentScopeVariables() const
{
    return _scopes.back().variables;
}

vector<string>
DescriptorVariables::getCurrentScopeParameters() const
{
    return vector<string>(_scopes.back().parameters.begin(), _scopes.back().parameters.end());
}

void
DescriptorVariables::addParameter(const string& name)
{
    if(_scopes.back().variables.find(name) != _scopes.back().variables.end())
    {
	throw "can't declare parameter `" + name + "': a variable with the same was previously defined";
    }
    _scopes.back().parameters.insert(name);
}

vector<string>
DescriptorVariables::getDeploymentTargets(const string& prefix) const
{
    if(prefix.empty())
    {
	return _deploymentTargets;
    }

    vector<string> targets;
    for(vector<string>::const_iterator p = _deploymentTargets.begin(); p != _deploymentTargets.end(); ++p)
    {
	string::size_type pos = p->find(prefix);
	if(pos != string::npos)
	{
	    targets.push_back(p->substr(prefix.size()));
	}
    }
    return targets;
}

void
DescriptorVariables::substitution(bool substitution)
{
    _scopes.back().substitution = substitution;
}

bool
DescriptorVariables::substitution() const
{
    return _scopes.back().substitution;
}

string
DescriptorVariables::substituteImpl(const string& v, set<string>& missing)
{
    if(!substitution())
    {
	return v;
    }

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
	    missing.insert(name);
	    ++beg;
	    continue;	   
	}
	else
	{
	    string val = getVariable(name);
	    value.replace(beg, end - beg + 1, val);
	    beg += val.length();
	}
    }

    return value;
}

DescriptorTemplates::DescriptorTemplates(const ApplicationDescriptorPtr& descriptor) : _application(descriptor)
{
}

void
DescriptorTemplates::setDescriptor(const ApplicationDescriptorPtr& desc)
{
    _application = desc;
}

ServerDescriptorPtr
DescriptorTemplates::instantiateServer(const DescriptorHelper& helper, 
				       const string& name, 
				       const map<string, string>& parameters)
{
    TemplateDescriptorDict::const_iterator p = _application->serverTemplates.find(name);
    if(p == _application->serverTemplates.end())
    {
	throw "unknown template `" + name + "'";
    }
    
    set<string> missing;
    Substitute substitute(helper.getVariables(), missing);
    map<string, string> params = parameters;

    set<string> unknown;
    for(map<string, string>::iterator q = params.begin(); q != params.end(); ++q)
    {
	if(find(p->second.parameters.begin(), p->second.parameters.end(), q->first) == p->second.parameters.end())
	{
	    unknown.insert(q->first);
	}
	substitute(q->second);
    }
    if(!unknown.empty())
    {
	ostringstream os;
	os << "server template instance unknown parameters: ";
	copy(unknown.begin(), unknown.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }

    set<string> missingParams;
    for(vector<string>::const_iterator q = p->second.parameters.begin(); q != p->second.parameters.end(); ++q)
    {
	if(params.find(*q) == params.end())
	{
	    missingParams.insert(*q);
	}
    }
    if(!missingParams.empty())
    {
	ostringstream os;
	os << "server template instance undefined parameters: ";
	copy(missingParams.begin(), missingParams.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }
    
    helper.getVariables()->push(params);
    ServerDescriptorPtr tmpl = ServerDescriptorPtr::dynamicCast(p->second.descriptor);
    assert(tmpl);
    ServerDescriptorPtr descriptor = ServerDescriptorHelper(helper, tmpl).instantiate(missing);
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
					const map<string, string>& parameters)
{
    TemplateDescriptorDict::const_iterator p = _application->serviceTemplates.find(name);
    if(p == _application->serviceTemplates.end())
    {
	throw "unknown template `" + name + "'";
    }
    
    set<string> missing;
    Substitute substitute(helper.getVariables(), missing);
    map<string, string> params = parameters;

    set<string> unknown;
    for(map<string, string>::iterator q = params.begin(); q != params.end(); ++q)
    {
	if(find(p->second.parameters.begin(), p->second.parameters.end(), q->first) == p->second.parameters.end())
	{
	    unknown.insert(q->first);
	}
	substitute(q->second);
    }
    if(!unknown.empty())
    {
	ostringstream os;
	os << "service template instance unknown parameters: ";
	copy(unknown.begin(), unknown.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }

    set<string> missingParams;
    for(vector<string>::const_iterator q = p->second.parameters.begin(); q != p->second.parameters.end(); ++q)
    {
	if(params.find(*q) == params.end())
	{
	    missingParams.insert(*q);
	}
    }
    if(!missingParams.empty())
    {
	ostringstream os;
	os << "service template instance undefined parameters: ";
	copy(missingParams.begin(), missingParams.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }
    for(map<string, string>::iterator q = params.begin(); q != params.end(); ++q)
    {
	substitute(q->second);
    }

    for(vector<string>::const_iterator q = p->second.parameters.begin(); q != p->second.parameters.end(); ++q)
    {
	if(params.find(*q) == params.end())
	{
	    missing.insert(*q);
	}
    }
    if(!missing.empty())
    {
	ostringstream os;
	os << "service template instance undefined parameters: ";
	copy(missing.begin(), missing.end(), ostream_iterator<string>(os, " "));
	throw os.str();
    }
    
    helper.getVariables()->push(params);
    ServiceDescriptorPtr tmpl = ServiceDescriptorPtr::dynamicCast(p->second.descriptor);
    assert(tmpl);
    ServiceDescriptorPtr descriptor = ServiceDescriptorHelper(helper, tmpl).instantiate(missing);
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
DescriptorTemplates::addServerTemplate(const string& id, const ServerDescriptorPtr& desc, const Ice::StringSeq& vars)
{
    //
    // Add the template to the application.
    //
    TemplateDescriptor tmpl;
    tmpl.descriptor = desc;
    tmpl.parameters = vars;
    _application->serverTemplates.insert(make_pair(id, tmpl));
}

void
DescriptorTemplates::addServiceTemplate(const string& id, const ServiceDescriptorPtr& desc, const Ice::StringSeq& vars)
{
    //
    // Add the template to the application.
    //
    TemplateDescriptor tmpl;
    tmpl.descriptor = desc;
    tmpl.parameters = vars;
    _application->serviceTemplates.insert(make_pair(id, tmpl));
}

ApplicationDescriptorPtr
DescriptorTemplates::getApplicationDescriptor() const
{
    return _application;
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
    _variables->push(descriptor->variables);
    _variables->addVariable("application", _descriptor->name);
}

ApplicationDescriptorHelper::ApplicationDescriptorHelper(const Ice::CommunicatorPtr& communicator,
							 const DescriptorVariablesPtr& variables,
							 const IceXML::Attributes& attrs) :
    DescriptorHelper(communicator, variables, new DescriptorTemplates(new ApplicationDescriptor())),
    _descriptor(_templates->getApplicationDescriptor())
{
    XmlAttributesHelper attributes(_variables, attrs);
    _descriptor->name = attributes("name");
    _descriptor->targets = _variables->getDeploymentTargets("");
    _variables->addVariable("application", _descriptor->name);
}

void
ApplicationDescriptorHelper::endParsing()
{
    _descriptor->variables = _variables->getCurrentScopeVariables();
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

void
ApplicationDescriptorHelper::addNode(const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    
    string node = attributes("name");	
    _variables->push();
    _variables->addVariable("node", node);
    for(NodeDescriptorSeq::const_iterator p = _descriptor->nodes.begin(); p != _descriptor->nodes.end(); ++p)
    {
	if(p->name == node)
	{
	    _variables->push(p->variables);
	    break;
	}
    }
}

void
ApplicationDescriptorHelper::endNodeParsing()
{
    NodeDescriptor node;
    node.name = _variables->getVariable("node");
    node.variables = _variables->getCurrentScopeVariables();
    _descriptor->nodes.push_back(node);
    _variables->pop();
}


void
ApplicationDescriptorHelper::addServer(const string& tmpl, const IceXML::Attributes& attrs)
{
    assert(_variables->hasVariable("node"));
    ServerInstanceDescriptor instance;
    instance._cpp_template = tmpl;
    instance.node = _variables->getVariable("node");
    instance.parameterValues = attrs;
    instance.parameterValues.erase("template");
    _descriptor->servers.push_back(instantiate(instance));
}

void
ApplicationDescriptorHelper::addServer(const ServerDescriptorPtr& descriptor)
{
    assert(_variables->hasVariable("node"));
    ServerInstanceDescriptor instance;
    instance.node = _variables->getVariable("node");
    instance.descriptor = descriptor;
    instance.targets = _variables->getDeploymentTargets(descriptor->name + ".");
    _descriptor->servers.push_back(instance);
}

auto_ptr<ServerDescriptorHelper>
ApplicationDescriptorHelper::addServerTemplate(const std::string& id, const IceXML::Attributes& attrs)
{
    return auto_ptr<ServerDescriptorHelper>(new ServerDescriptorHelper(*this, attrs, id));
}

auto_ptr<ServiceDescriptorHelper>
ApplicationDescriptorHelper::addServiceTemplate(const std::string& id, const IceXML::Attributes& attrs)
{
    return auto_ptr<ServiceDescriptorHelper>(new ServiceDescriptorHelper(*this, attrs, id));
}

ApplicationUpdateDescriptor
ApplicationDescriptorHelper::update(const ApplicationUpdateDescriptor& update)
{
    ApplicationUpdateDescriptor newUpdate = update;
    ApplicationDescriptorPtr newApp = new ApplicationDescriptor();
    ApplicationDescriptorPtr oldApp = _descriptor;
    _descriptor = newApp;
    _templates->setDescriptor(newApp);

    newApp->name = oldApp->name;
    newApp->comment = newUpdate.comment ? newUpdate.comment->value : oldApp->comment;
    newApp->targets = oldApp->targets;
    newApp->variables = oldApp->variables;
    Ice::StringSeq::const_iterator p;
    for(p = newUpdate.removeVariables.begin(); p != newUpdate.removeVariables.end(); ++p)
    {
	newApp->variables.erase(*p);
	_variables->remove(*p);
    }
    for(map<string, string>::const_iterator q = newUpdate.variables.begin(); q != newUpdate.variables.end(); ++q)
    {
	newApp->variables[q->first] = q->second;
	_variables->addVariable(q->first, q->second);
    }

    newApp->serverTemplates = oldApp->serverTemplates;
    for(p = newUpdate.removeServerTemplates.begin(); p != newUpdate.removeServerTemplates.end(); ++p)
    {
	newApp->serverTemplates.erase(*p);
    }
    TemplateDescriptorDict::const_iterator t;
    for(t = newUpdate.serverTemplates.begin(); t != newUpdate.serverTemplates.end(); ++t)
    {
	newApp->serverTemplates[t->first] = t->second;
    }

    newApp->serviceTemplates = oldApp->serviceTemplates;
    for(p = newUpdate.removeServiceTemplates.begin(); p != newUpdate.removeServiceTemplates.end(); ++p)
    {
	newApp->serviceTemplates.erase(*p);
    }
    for(t = newUpdate.serviceTemplates.begin(); t != newUpdate.serviceTemplates.end(); ++t)
    {
	newApp->serviceTemplates[t->first] = t->second;
    }

    newApp->nodes = newUpdate.nodes;
    set<string> removed(newUpdate.removeNodes.begin(), newUpdate.removeNodes.end());
    for(NodeDescriptorSeq::const_iterator q = oldApp->nodes.begin(); q != oldApp->nodes.end(); ++q)
    {
	if(removed.find(q->name) == removed.end())
	{
	    NodeDescriptorSeq::const_iterator r;
	    for(r = newUpdate.nodes.begin(); r != newUpdate.nodes.end(); ++r)
	    {
		if(q->name == r->name)
		{
		    break;
		}
	    }
	    if(r == newUpdate.nodes.end())
	    {
		newApp->nodes.push_back(*q);
	    }
	}
    }

    newApp->servers.clear();
    for(ServerInstanceDescriptorSeq::iterator q = newUpdate.servers.begin(); q != newUpdate.servers.end(); ++q)
    {
	*q = instantiate(*q);
	newApp->servers.push_back(*q);
    }

    removed = set<string>(newUpdate.removeServers.begin(), newUpdate.removeServers.end());
    set<string> updated;
    for_each(newApp->servers.begin(), newApp->servers.end(), AddServerName(updated));
    for(ServerInstanceDescriptorSeq::const_iterator q = oldApp->servers.begin(); q != oldApp->servers.end(); ++q)
    {
	ServerInstanceDescriptor inst = instantiate(*q); // Re-instantiate old servers.
	if(updated.find(inst.descriptor->name) == updated.end() && removed.find(inst.descriptor->name) == removed.end())
	{
	    if(q->node != inst.node ||
	       ServerDescriptorHelper(*this, q->descriptor) != ServerDescriptorHelper(*this, inst.descriptor))
	    {
		newUpdate.servers.push_back(inst);
	    }
	    newApp->servers.push_back(inst);
	}
    }

    return newUpdate;
}

ApplicationUpdateDescriptor
ApplicationDescriptorHelper::diff(const ApplicationDescriptorPtr& orig)
{
    ApplicationUpdateDescriptor update;
    update.comment = _descriptor->comment != orig->comment ? new BoxedComment(_descriptor->comment) : BoxedCommentPtr();
    update.targets = _descriptor->targets != orig->targets ? new BoxedTargets(_descriptor->targets) : BoxedTargetsPtr();

    update.variables = _descriptor->variables;
    map<string, string>::iterator p = update.variables.begin();
    while(p != update.variables.end())
    {
	map<string, string>::const_iterator q = orig->variables.find(p->first);
	if(q != orig->variables.end() && q->second == p->second)
	{
	    map<string, string>::iterator tmp = p++;
	    update.variables.erase(tmp);
	}
	else
	{
	    ++p;
	}
    }
    for(map<string, string>::const_iterator q = orig->variables.begin(); q != orig->variables.end(); ++q)
    {
	if(_descriptor->variables.find(q->first) == _descriptor->variables.end())
	{
	    update.removeVariables.push_back(q->first);
	}
    }

    update.serverTemplates = _descriptor->serverTemplates;
    TemplateDescriptorDict::iterator t = update.serverTemplates.begin();
    while(t != update.serverTemplates.end())
    {
	TemplateDescriptorDict::const_iterator q = orig->serverTemplates.find(t->first);
	if(q != orig->serverTemplates.end() && 
	   q->second.parameters == t->second.parameters &&
	   ServerDescriptorHelper(*this, ServerDescriptorPtr::dynamicCast(q->second.descriptor)) == 
	   ServerDescriptorHelper(*this, ServerDescriptorPtr::dynamicCast(t->second.descriptor)))
	{
	    TemplateDescriptorDict::iterator tmp = t++;
	    update.serverTemplates.erase(tmp);
	}
	else
	{
	    ++t;
	}
    }
    for(t = orig->serverTemplates.begin(); t != orig->serverTemplates.end(); ++t)
    {
	if(_descriptor->serverTemplates.find(t->first) == _descriptor->serverTemplates.end())
	{
	    update.removeServerTemplates.push_back(t->first);
	}
    }

    update.serviceTemplates = _descriptor->serviceTemplates;
    t = update.serviceTemplates.begin();
    while(t != update.serviceTemplates.end())
    {
	TemplateDescriptorDict::const_iterator q = orig->serviceTemplates.find(t->first);
	if(q != orig->serviceTemplates.end() && 
	   q->second.parameters == t->second.parameters &&
	   ServiceDescriptorHelper(*this, ServiceDescriptorPtr::dynamicCast(q->second.descriptor)) ==
	   ServiceDescriptorHelper(*this, ServiceDescriptorPtr::dynamicCast(t->second.descriptor)))
	{
	    TemplateDescriptorDict::iterator tmp = t++;
	    update.serviceTemplates.erase(tmp);
	}
	else
	{
	    ++t;
	}
    }
    for(t = orig->serviceTemplates.begin(); t != orig->serviceTemplates.end(); ++t)
    {
 	if(_descriptor->serviceTemplates.find(t->first) == _descriptor->serviceTemplates.end())
	{
	    update.removeServiceTemplates.push_back(t->first);
	}
    }

    update.nodes = _descriptor->nodes;
    NodeDescriptorSeq::iterator n = update.nodes.begin();
    while(n != update.nodes.end())
    {
	NodeDescriptorSeq::const_iterator q;
	for(q = orig->nodes.begin(); q != orig->nodes.end(); ++q)
	{
	    if(n->name == q->name)
	    {
		break;
	    }
	}
	if(q != orig->nodes.end() && *n == *q)
	{
	    n = update.nodes.erase(n);
	}
	else
	{
	    ++n;
	}
    }
    for(n = orig->nodes.begin(); n != orig->nodes.end(); ++n)
    {
	bool found = false;
	for(NodeDescriptorSeq::const_iterator q = orig->nodes.begin(); q != orig->nodes.end(); ++q)
	{
	    if(n->name == q->name)
	    {
		found = true;
		break;
	    }
	}
	if(!found)
	{
	    update.removeNodes.push_back(n->name);
	}
    }

    update.servers = _descriptor->servers;
    ServerInstanceDescriptorSeq::iterator i = update.servers.begin();
    while(i != update.servers.end())
    {
	ServerInstanceDescriptorSeq::const_iterator q;
	for(q = orig->servers.begin(); q != orig->servers.end(); ++q)
	{
	    if(i->descriptor->name == q->descriptor->name)
	    {
		break;
	    }
	}

	if(q != orig->servers.end() && 
	   i->_cpp_template == q->_cpp_template &&
	   i->parameterValues == q->parameterValues &&
	   i->targets == q->targets &&
	   ServerDescriptorHelper(*this, q->descriptor) == ServerDescriptorHelper(*this, i->descriptor))
	{
	    i = update.servers.erase(i);
	}
	else
	{
	    ++i;
	}
    }
    for(i = orig->servers.begin(); i != orig->servers.end(); ++i)
    {
	ServerInstanceDescriptorSeq::const_iterator q;
	for(q = _descriptor->servers.begin(); q != _descriptor->servers.end(); ++q)
	{
	    if(i->descriptor->name == q->descriptor->name)
	    {
		break;
	    }
	}
	if(q == _descriptor->servers.end())
	{
	    update.removeServers.push_back(i->descriptor->name);
	}
    }

    return update;
}

void
ApplicationDescriptorHelper::addServerInstance(const string& tmpl, 
					       const string& node, 
					       const map<string, string>& parameters)
{
    pushNodeVariables(node);

    ServerInstanceDescriptor instance;
    instance._cpp_template = tmpl;
    instance.node = node;
    instance.parameterValues = parameters;
    instance.descriptor = _templates->instantiateServer(*this, tmpl, instance.parameterValues);
    _descriptor->servers.push_back(instance);    

    _variables->pop();
}

void
ApplicationDescriptorHelper::instantiate()
{
    for(ServerInstanceDescriptorSeq::iterator p = _descriptor->servers.begin(); p != _descriptor->servers.end(); ++p)
    {
	*p = instantiate(*p);
    }
}

ServerInstanceDescriptor
ApplicationDescriptorHelper::instantiate(const ServerInstanceDescriptor& inst)
{
    ServerInstanceDescriptor instance = inst;
    pushNodeVariables(inst.node);
    if(instance._cpp_template.empty())
    {
	//
	// We can't re-instantiate here -- this would break escaped variables.
	//
// 	assert(instance.descriptor);
// 	set<string> missing;
// 	instance.descriptor = ServerDescriptorHelper(*this, instance.descriptor).instantiate(missing);
// 	if(!missing.empty())
// 	{
// 	    ostringstream os;
// 	    os << "server undefined variables: ";
// 	    copy(missing.begin(), missing.end(), ostream_iterator<string>(os, " "));
// 	    throw os.str();
// 	}
    }
    else
    {
	instance.descriptor = _templates->instantiateServer(*this, instance._cpp_template, instance.parameterValues);
    }
    _variables->pop();
    return instance;
}

void
ApplicationDescriptorHelper::pushNodeVariables(const string& node)
{
    NodeDescriptorSeq::const_iterator q;
    for(q = _descriptor->nodes.begin(); q != _descriptor->nodes.end(); ++q)
    {
	if(q->name == node)
	{
	    _variables->push(q->variables);
	    break;
	}
    }
    if(q == _descriptor->nodes.end())
    {
	_variables->push();
    }
    _variables->addVariable("node", node);
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

    if(_descriptor->variables != helper._descriptor->variables)
    {
	return false;
    }

    return true;
}

bool
ComponentDescriptorHelper::operator!=(const ComponentDescriptorHelper& helper) const
{
    return !operator==(helper);
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
	string fqn = "${server}";
	if(ServiceDescriptorPtr::dynamicCast(_descriptor))
	{
	    fqn += ".${service}";
	}
	desc.id = _variables->substitute(fqn) + "." + desc.name;
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
ComponentDescriptorHelper::instantiateImpl(const ComponentDescriptorPtr& desc, set<string>& missing) const
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
    _variables->push(_descriptor->variables);
}

ServerDescriptorHelper::ServerDescriptorHelper(const DescriptorHelper& helper, const IceXML::Attributes& attrs,
					       const string& id) :
    ComponentDescriptorHelper(helper),
    _templateId(id)
{
    XmlAttributesHelper attributes(_variables, attrs);

    _variables->push();
    if(!_templateId.empty())
    {
	_variables->substitution(false);
    }

    string interpreter = attributes("interpreter", "");
    if(interpreter == "icebox" || interpreter == "java-icebox")
    {
	_descriptor = new IceBoxDescriptor();
	_descriptor->exe = attributes("exe", "");
    }
    else
    {
	_descriptor = new ServerDescriptor();
	_descriptor->exe = attributes("exe");
    }
    _descriptor->interpreter = interpreter;

    ComponentDescriptorHelper::init(_descriptor, attrs);

    _descriptor->pwd = attributes("pwd", "");
    _descriptor->activation = attributes("activation", "manual");
    
    if(interpreter == "icebox" || interpreter == "java-icebox")
    {
	IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_descriptor);
	iceBox->endpoints = attributes("endpoints");
	
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

    if(_templateId.empty())
    {
	_variables->addVariable("server", _descriptor->name);
    }
}

ServerDescriptorHelper::~ServerDescriptorHelper()
{
    _variables->pop();    
}

void
ServerDescriptorHelper::endParsing()
{
    if(!_templateId.empty())
    {
	_descriptor->variables = _variables->getCurrentScopeVariables();
	_templates->addServerTemplate(_templateId, _descriptor, _variables->getCurrentScopeParameters());
	_variables->substitution(true);
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
    
    if(_descriptor->interpreter != helper._descriptor->interpreter)
    {
	return false;
    }

    if(set<string>(_descriptor->interpreterOptions.begin(), _descriptor->interpreterOptions.end()) != 
       set<string>(helper._descriptor->interpreterOptions.begin(), helper._descriptor->interpreterOptions.end()))
    {
	return false;
    }
    
    if(IceBoxDescriptorPtr::dynamicCast(_descriptor))
    {
	IceBoxDescriptorPtr ilhs = IceBoxDescriptorPtr::dynamicCast(_descriptor);
	IceBoxDescriptorPtr irhs = IceBoxDescriptorPtr::dynamicCast(helper._descriptor);
	
	if(ilhs->endpoints != irhs->endpoints)
	{
	    return false;
	}
	
	if(ilhs->services.size() != irhs->services.size())
	{
	    return false;
	}
	
	//
	// First we compare the service instances which have a
	// descriptor set (this is the case for services not based on
	// a template or server instances).
	//
	for(ServiceInstanceDescriptorSeq::const_iterator p = ilhs->services.begin(); p != ilhs->services.end(); ++p)
	{
	    if(p->descriptor)
	    {
		bool found = false;
		for(ServiceInstanceDescriptorSeq::const_iterator q = irhs->services.begin();
		    q != irhs->services.end();
		    ++q)
		{
		    if(q->descriptor && p->descriptor->name == q->descriptor->name)
		    {
			found = true;
			if(ServiceDescriptorHelper(*this, p->descriptor) !=
			   ServiceDescriptorHelper(*this, q->descriptor))
			{
			    return false;
			}
			break;
		    }
		}
		if(!found)
		{
		    return false;
		}
	    }
	}

	//
	// Then, we compare the service instances for which no
	// descriptor is set.
	//
	set<ServiceInstanceDescriptor> lsvcs;
	set<ServiceInstanceDescriptor> rsvcs;
	for(ServiceInstanceDescriptorSeq::const_iterator p = ilhs->services.begin(); p != ilhs->services.end(); ++p)
	{
	    if(!p->descriptor)
	    {
		ServiceInstanceDescriptor instance = *p;
		instance.descriptor = 0;
		lsvcs.insert(instance);
	    }
	}
	for(ServiceInstanceDescriptorSeq::const_iterator p = irhs->services.begin(); p != irhs->services.end(); ++p)
	{
	    if(!p->descriptor)
	    {
		ServiceInstanceDescriptor instance = *p;
		instance.descriptor = 0;
		rsvcs.insert(instance);
	    }
	}
	if(lsvcs != rsvcs)
	{
	    return false;
	}
    }
    return true;
}

bool
ServerDescriptorHelper::operator!=(const ServerDescriptorHelper& helper) const
{
    return !operator==(helper);
}

const ServerDescriptorPtr&
ServerDescriptorHelper::getDescriptor() const
{
    return _descriptor;
}

const string&
ServerDescriptorHelper::getTemplateId() const
{
    return _templateId;
}

auto_ptr<ServiceDescriptorHelper>
ServerDescriptorHelper::addServiceTemplate(const std::string& id, const IceXML::Attributes& attrs)
{
    return auto_ptr<ServiceDescriptorHelper>(new ServiceDescriptorHelper(*this, attrs, id));
}

void
ServerDescriptorHelper::addService(const string& tmpl, const IceXML::Attributes& attrs)
{
    XmlAttributesHelper attributes(_variables, attrs);
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_descriptor);
    if(!iceBox)
    {
	throw "element <service> can only be a child of an IceBox <server> element";
    }

    ServiceInstanceDescriptor instance;
    instance._cpp_template = tmpl;
    instance.parameterValues = attrs;
    instance.parameterValues.erase("template");
    instance.descriptor = _templates->instantiateService(*this, instance._cpp_template, instance.parameterValues);
    iceBox->services.push_back(instance);
}

void
ServerDescriptorHelper::addService(const ServiceDescriptorPtr& descriptor)
{
    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(_descriptor);
    if(!iceBox)
    {
	throw "element <service> can only be a child of an IceBox <server> element";
    }

    ServiceInstanceDescriptor instance;
    instance.descriptor = descriptor;
    if(_templateId.empty())
    {
	instance.targets = _variables->getDeploymentTargets(_descriptor->name + "." + descriptor->name + ".");
    }
    iceBox->services.push_back(instance);
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
ServerDescriptorHelper::addInterpreterOption(const string& option)
{
    if(_descriptor->interpreter.empty())
    {
	throw "element <interpreter-option> can only be specified if the interpreter attribute of the <server> "
	    "element is not empty";
    }

    _descriptor->interpreterOptions.push_back(option);
}

ServerDescriptorPtr
ServerDescriptorHelper::instantiate(set<string>& missing) const
{
    ServerDescriptorPtr desc = ServerDescriptorPtr::dynamicCast(_descriptor->ice_clone());
    instantiateImpl(desc, missing);
    return desc;    
}

void
ServerDescriptorHelper::instantiateImpl(const ServerDescriptorPtr& desc, set<string>& missing) const
{
    Substitute substitute(_variables, missing);
    substitute(desc->name);
    _variables->addVariable("server", desc->name);

    ComponentDescriptorHelper::instantiateImpl(desc, missing);

    substitute(desc->exe);
    substitute(desc->pwd);
    for_each(desc->options.begin(), desc->options.end(), substitute);
    for_each(desc->envs.begin(), desc->envs.end(), substitute);
    substitute(desc->interpreter);
    for_each(desc->interpreterOptions.begin(), desc->interpreterOptions.end(), substitute);

    IceBoxDescriptorPtr iceBox = IceBoxDescriptorPtr::dynamicCast(desc);
    if(iceBox)
    {
 	for(ServiceInstanceDescriptorSeq::iterator p = iceBox->services.begin(); p != iceBox->services.end(); ++p)
 	{
	    if(p->_cpp_template.empty())
	    {
		ServiceDescriptorPtr service = p->descriptor;
		assert(service);
		p->descriptor = ServiceDescriptorHelper(*this, service).instantiate(missing);
	    }
	    else
	    {
		p->descriptor = _templates->instantiateService(*this, p->_cpp_template, p->parameterValues);
	    }
 	}
    }
}

ServiceDescriptorHelper::ServiceDescriptorHelper(const DescriptorHelper& helper, const ServiceDescriptorPtr& desc) :
    ComponentDescriptorHelper(helper),
    _descriptor(desc)
{
    init(_descriptor);
    _variables->push(_descriptor->variables);
}

ServiceDescriptorHelper::ServiceDescriptorHelper(const DescriptorHelper& helper, const IceXML::Attributes& attrs,
						 const string& id) :
    ComponentDescriptorHelper(helper),
    _descriptor(new ServiceDescriptor()),
    _templateId(id)
{
    XmlAttributesHelper attributes(_variables, attrs);

    _variables->push();
    if(!_templateId.empty())
    {
	_variables->substitution(false);
    }

    init(_descriptor, attrs);

    _descriptor->entry = attributes("entry");
    if(_templateId.empty())
    {
	_variables->addVariable("service", _descriptor->name);
    }
}

ServiceDescriptorHelper::~ServiceDescriptorHelper()
{
    _variables->pop();
}

void
ServiceDescriptorHelper::endParsing()
{
    if(!_templateId.empty())
    {
	_descriptor->variables = _variables->getCurrentScopeVariables();
	_templates->addServiceTemplate(_templateId, _descriptor, _variables->getCurrentScopeParameters());
	_variables->substitution(true);
    }
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

bool
ServiceDescriptorHelper::operator!=(const ServiceDescriptorHelper& helper) const
{
    return !operator==(helper);
}

ServiceDescriptorPtr
ServiceDescriptorHelper::instantiate(set<string>& missing) const
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

const string&
ServiceDescriptorHelper::getTemplateId() const
{
    return _templateId;
}

void
ServiceDescriptorHelper::instantiateImpl(const ServiceDescriptorPtr& desc, set<string>& missing) const
{
    Substitute substitute(_variables, missing);
    substitute(desc->name);
    _variables->addVariable("service", desc->name);

    ComponentDescriptorHelper::instantiateImpl(desc, missing);

    substitute(desc->entry);
}
