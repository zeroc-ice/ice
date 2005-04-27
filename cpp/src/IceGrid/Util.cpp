// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <Ice/Ice.h>
#include <IcePatch2/Util.h>
#include <IceGrid/Util.h>
#include <IceGrid/Admin.h>

#include <fstream>

using namespace std;
using namespace Ice;
using namespace IceGrid;

namespace IceGrid
{

bool equal(const ServiceDescriptorPtr&, const ServiceDescriptorPtr&);
void instantiateComponentTemplate(const ComponentDescriptorPtr&, const vector<map<string, string> >&, vector<string>&);

}

struct Substitute : std::unary_function<string&, void>
{
    Substitute(const vector<map<string, string> >& variables, vector<string>& missing) : 
	_variables(variables), _missing(missing)
    {
    } 

    void operator()(string& v)
    {
	v.assign(substitute(v, _variables, true, _missing));
    }

    const vector<map<string, string> >& _variables;
    vector<string>& _missing;
};

ServiceDescriptorSeq
IceGrid::getServices(const ComponentDescriptorPtr& descriptor)
{
    CppIceBoxDescriptorPtr cppIceBox = CppIceBoxDescriptorPtr::dynamicCast(descriptor);
    if(cppIceBox)
    {
	return cppIceBox->services;
    }
    
    JavaIceBoxDescriptorPtr javaIceBox = JavaIceBoxDescriptorPtr::dynamicCast(descriptor);
    if(javaIceBox)
    {
	return javaIceBox->services;
    }

    return ServiceDescriptorSeq();
}

bool
IceGrid::equal(const ServiceDescriptorPtr& lhs, const ServiceDescriptorPtr& rhs)
{
    if(lhs->ice_id() != rhs->ice_id())
    {
	return false;
    }

    if(lhs->name != rhs->name)
    {
	return false;
    }

    if(lhs->comment != rhs->comment)
    {
	return false;
    }

    if(lhs->entry != rhs->entry)
    {
	return false;
    }

    if(set<AdapterDescriptor>(lhs->adapters.begin(), lhs->adapters.end())  != 
       set<AdapterDescriptor>(rhs->adapters.begin(), rhs->adapters.end()))
    {
	return false;
    }

    if(set<PropertyDescriptor>(lhs->properties.begin(), lhs->properties.end()) != 
       set<PropertyDescriptor>(rhs->properties.begin(), rhs->properties.end()))
    {
	return false;
    }

    if(set<DbEnvDescriptor>(lhs->dbEnvs.begin(), lhs->dbEnvs.end()) != 
       set<DbEnvDescriptor>(rhs->dbEnvs.begin(), rhs->dbEnvs.end()))
    {
	return false;
    }

    return true;
}

bool
IceGrid::equal(const ServerDescriptorPtr& lhs, const ServerDescriptorPtr& rhs)
{
    if(lhs->ice_id() != rhs->ice_id())
    {
	return false;
    }

    if(lhs->name != rhs->name)
    {
	return false;
    }

    if(lhs->comment != rhs->comment)
    {
	return false;
    }

    if(lhs->exe != rhs->exe)
    {
	return false;
    }

    if(lhs->pwd != rhs->pwd)
    {
	return false;
    }

    if(lhs->node != rhs->node)
    {
	return false;
    }
    
    if(lhs->application != rhs->application)
    {
	return false;
    }

    if(set<AdapterDescriptor>(lhs->adapters.begin(), lhs->adapters.end())  != 
       set<AdapterDescriptor>(rhs->adapters.begin(), rhs->adapters.end()))
    {
	return false;
    }

    if(set<PropertyDescriptor>(lhs->properties.begin(), lhs->properties.end()) != 
       set<PropertyDescriptor>(rhs->properties.begin(), rhs->properties.end()))
    {
	return false;
    }

    if(set<DbEnvDescriptor>(lhs->dbEnvs.begin(), lhs->dbEnvs.end()) != 
       set<DbEnvDescriptor>(rhs->dbEnvs.begin(), rhs->dbEnvs.end()))
    {
	return false;
    }

    if(set<string>(lhs->options.begin(), lhs->options.end()) != set<string>(rhs->options.begin(), rhs->options.end()))
    {
	return false;
    }

    if(set<string>(lhs->envs.begin(), lhs->envs.end()) != set<string>(rhs->envs.begin(), rhs->envs.end()))
    {
	return false;
    }
    
    //
    // TODO: perhaps if would be better to define an equal operation on the Slice class?
    //
    ServiceDescriptorSeq slhs;
    ServiceDescriptorSeq srhs;

    if(JavaServerDescriptorPtr::dynamicCast(lhs))
    {
	JavaServerDescriptorPtr jlhs = JavaServerDescriptorPtr::dynamicCast(lhs);
	JavaServerDescriptorPtr jrhs = JavaServerDescriptorPtr::dynamicCast(rhs);
	
	if(jlhs->className != jrhs->className)
	{
	    return false;
	}

	if(set<string>(jlhs->jvmOptions.begin(), jlhs->jvmOptions.end()) != 
	   set<string>(jrhs->jvmOptions.begin(), jrhs->jvmOptions.end()))
	{
	    return false;
	}
	
	if(JavaIceBoxDescriptorPtr::dynamicCast(lhs))
	{
	    JavaIceBoxDescriptorPtr ilhs = JavaIceBoxDescriptorPtr::dynamicCast(lhs);
	    JavaIceBoxDescriptorPtr irhs = JavaIceBoxDescriptorPtr::dynamicCast(rhs);

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
    else if(CppIceBoxDescriptorPtr::dynamicCast(lhs))
    {
	CppIceBoxDescriptorPtr ilhs = CppIceBoxDescriptorPtr::dynamicCast(lhs);
	CppIceBoxDescriptorPtr irhs = CppIceBoxDescriptorPtr::dynamicCast(rhs);

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
		    if(!equal(*p, *q))
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

string
IceGrid::getVariable(const vector<map<string, string> >& variables, const string& name)
{
    static const string empty;
    vector<map<string, string> >::const_reverse_iterator p = variables.rbegin();
    while(p != variables.rend())
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
IceGrid::hasVariable(const vector<map<string, string> >& variables, const string& name)
{
    vector<map<string, string> >::const_reverse_iterator p = variables.rbegin();
    while(p != variables.rend())
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

string
IceGrid::substitute(const string& v, const vector<map<string, string> >& vars, bool ignore, vector<string>& missing)
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
	if(!hasVariable(vars, name))
	{
	    if(!ignore)
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
	    value.replace(beg, end - beg + 1, getVariable(vars, name));
	}
    }

    return value;
}

void
IceGrid::instantiateComponentTemplate(const ComponentDescriptorPtr& desc, const vector<map<string, string> >& vars, 
				      vector<string>& missing)
{
    Substitute substitute(vars, missing);
    substitute(desc->name);
    substitute(desc->comment);
    for(AdapterDescriptorSeq::iterator p = desc->adapters.begin(); p != desc->adapters.end(); ++p)
    {
	substitute(p->name);
	substitute(p->id);
	substitute(p->endpoints);
	for(ObjectDescriptorSeq::iterator q = p->objects.begin(); q != p->objects.end(); ++q)
	{
	    //q->proxy = ; TODO!
	    substitute(q->type);
	    substitute(q->adapterId);
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

ServerDescriptorPtr
IceGrid::instantiateTemplate(const ServerDescriptorPtr& descriptor,
			     const map<string, string>& vars, 
			     vector<string>& missing)
{
    ServerDescriptorPtr desc = ServerDescriptorPtr::dynamicCast(descriptor->ice_clone());
    vector<map<string, string> > variables;
    variables.push_back(vars);
    instantiateComponentTemplate(desc, variables, missing);
    Substitute substitute(variables, missing);
    substitute(desc->exe);
    substitute(desc->pwd);
    for_each(desc->options.begin(), desc->options.end(), substitute);
    for_each(desc->envs.begin(), desc->envs.end(), substitute);
//     if(JavaServerDescriptorPtr::dynamicCast(desc))
//     {
// 	desc->className = substitute(desc->className, variables, true, missing);
// 	for_each(desc->jvmOptions.begin(), desc->jvmOptions.end(), Substitute(variables, missing));
//     }
    return desc;
}

ServiceDescriptorPtr
IceGrid::instantiateTemplate(const ServiceDescriptorPtr& descriptor, const map<string, string>& vars, 
			     vector<string>& missing)
{
    return descriptor;
}
