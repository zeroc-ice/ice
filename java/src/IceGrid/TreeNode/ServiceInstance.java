// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;

class ServiceInstance extends Parent
{
    ServiceInstance(ServiceInstanceDescriptor descriptor,
		    TemplateDescriptor templateDescriptor)
    {
	_descriptor = descriptor;
	_templateDescriptor = templateDescriptor;
	
	if(_descriptor.descriptor != null)
	{
	    _adapters = new Adapters(_descriptor.descriptor.adapters, false);
	    addChild(_adapters);
	    
	    _dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, false);
	    addChild(_dbEnvs);
	}
    }

    public String toString()
    {
	String result = "";

	//
	// The if Temporary
	//
	if(_descriptor.descriptor != null)
	{
	    result = _descriptor.descriptor.name;
	}

	if(_templateDescriptor != null)
	{
	    result += ": " + templateLabel(_descriptor.template, 
					   _templateDescriptor.parameters,
					   _descriptor.parameterValues);
	}
	return result;
    }

    private ServiceInstanceDescriptor _descriptor;
    private TemplateDescriptor _templateDescriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
