// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

#include <IcePack/DescriptorUtil.h>

using namespace std;

namespace IcePack
{

bool equal(const ServiceDescriptorPtr&, const ServiceDescriptorPtr&);

}

bool
IcePack::equal(const ServiceDescriptorPtr& lhs, const ServiceDescriptorPtr& rhs)
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
IcePack::equal(const ServerDescriptorPtr& lhs, const ServerDescriptorPtr& rhs)
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
