// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Service extends PropertiesHolder
{
    Service(String name,
	    String displayString,
	    ServiceInstanceDescriptor instanceDescriptor, 
	    ServiceDescriptor serviceDescriptor,
	    boolean editable,
	    Utils.Resolver resolver,
	    Model model)
    {
	super(name, model);
	_displayString = displayString;
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = serviceDescriptor;
	_descriptor = serviceDescriptor;

	_editable = editable;
	_resolver = resolver;

	boolean childrenEditable = _editable && 
	    (_instanceDescriptor.template.length() == 0);

	_adapters = new Adapters(serviceDescriptor.adapters, 
				 childrenEditable, resolver, _model);
	addChild(_adapters);
	
	_dbEnvs = new DbEnvs(serviceDescriptor.dbEnvs, 
			     childrenEditable, resolver, _model);
	addChild(_dbEnvs);
    }


    public String toString()
    {
	if(_displayString != null)
	{
	    return _displayString;
	}
	else
	{
	    return _id;
	}
    }

    public void cleanup()
    {
	_adapters.cleanup();
    }

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;
    private String _displayString;
    private boolean _editable;
    private Utils.Resolver _resolver;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
}
