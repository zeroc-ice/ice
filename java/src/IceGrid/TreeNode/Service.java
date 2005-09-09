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

class Service extends Parent
{
    Service(String name,
	    String displayString,
	    ServiceInstanceDescriptor instanceDescriptor, 
	    ServiceDescriptor serviceDescriptor,
	    Editable editable,
	    Utils.Resolver resolver,
	    Application application,
	    Model model)
	throws DuplicateIdException
    {
	super(name, model);
	_displayString = displayString;
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = serviceDescriptor;
	_propertiesHolder = new PropertiesHolder(serviceDescriptor);

	_isEditable = editable != null;
	_resolver = resolver;

	boolean areChildrenEditable = _instanceDescriptor.template.length() == 0 && editable != null;

	_adapters = new Adapters(serviceDescriptor.adapters, 
				 areChildrenEditable, false, resolver, application, _model);
	addChild(_adapters);
	
	_dbEnvs = new DbEnvs(serviceDescriptor.dbEnvs, 
			     areChildrenEditable, resolver, _model);
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

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public void unregister()
    {
	_adapters.unregister();
    }

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;
    private String _displayString;
    private boolean _isEditable;
    private Utils.Resolver _resolver;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;
}
