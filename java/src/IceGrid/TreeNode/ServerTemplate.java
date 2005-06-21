// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.IceBoxDescriptor;
import IceGrid.TemplateDescriptor;

class ServerTemplate extends Parent
{
    ServerTemplate(String name, TemplateDescriptor descriptor,
		   java.util.Map serviceTemplates)
    {
	_name = name;
	_descriptor = descriptor;
	
	if(_descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_descriptor.descriptor;
	    
	    _serviceInstances = new ServiceInstances(_iceBoxDescriptor.services, 
						     serviceTemplates);
	    addChild(_serviceInstances);
	}
	else
	{
	    _iceBoxDescriptor = null;
	}
	

	_adapters = new Adapters(_descriptor.descriptor.adapters, true);
	addChild(_adapters);

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, true);
	addChild(_dbEnvs);
    }

    public String toString()
    {
	return templateLabel(_name, _descriptor.parameters);
    }

    private String _name;
    private TemplateDescriptor _descriptor;
    private IceBoxDescriptor _iceBoxDescriptor;

    private ServiceInstances _serviceInstances;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
