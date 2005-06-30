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
import IceGrid.Model;

class ServiceInstance extends Parent
{
    ServiceInstance(ServiceInstanceDescriptor descriptor, Model model, Node node)
    {
	super(descriptor.descriptor.name, model);
	_descriptor = descriptor;
	
	_adapters = new Adapters(_descriptor.descriptor.adapters, _model, node);
	addChild(_adapters);
	    
	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, _model, false);
	addChild(_dbEnvs);
    }

    void unregisterAdapters()
    {
	_adapters.unregisterAll();
    }

    public String toString()
    {
	String result = _descriptor.descriptor.name;

	if(!_descriptor.template.equals(""))
	{
	    result += ": " + templateLabel(_descriptor.template, 
					   _descriptor.parameterValues.values());
	}
	return result;
    }

    private ServiceInstanceDescriptor _descriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
