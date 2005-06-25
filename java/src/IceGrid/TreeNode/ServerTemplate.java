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
    ServerTemplate(String name, TemplateDescriptor descriptor)
    {
	super(name);
	rebuild(descriptor);
    }

    void rebuild(TemplateDescriptor descriptor)
    {
	_descriptor = descriptor;
	clearChildren();

	//
	// Fix-up parameters order
	//
	java.util.Collections.sort(_descriptor.parameters);
	
	if(_descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_descriptor.descriptor;
	    
	    _serviceInstances = new ServiceInstances(_iceBoxDescriptor.services);
	    addChild(_serviceInstances);
	}
	else
	{
	    _serviceInstances = null;
	    _iceBoxDescriptor = null;
	}
	
	_adapters = new Adapters(_descriptor.descriptor.adapters, true);
	addChild(_adapters);

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, true);
	addChild(_dbEnvs);
    }


    public String toString()
    {
	return templateLabel(_id, _descriptor.parameters);
    }

    private TemplateDescriptor _descriptor;
    private IceBoxDescriptor _iceBoxDescriptor;

    private ServiceInstances _serviceInstances;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
