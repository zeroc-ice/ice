// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Service extends Parent
{
    static public ServiceInstanceDescriptor
    copyDescriptor(ServiceInstanceDescriptor instanceDescriptor)
    {
	ServiceInstanceDescriptor copy = (ServiceInstanceDescriptor)
	    instanceDescriptor.clone();
	
	if(copy.descriptor != null)
	{
	    copy.descriptor = (ServiceDescriptor)copy.descriptor.clone();
	    copy.descriptor.adapters = Adapters.copyDescriptors(copy.descriptor.adapters);
	    copy.descriptor.dbEnvs = DbEnvs.copyDescriptors(copy.descriptor.dbEnvs);

	    //
	    // Update to properties is not atomic because of Adapter endpoints
	    // (and possibly other properties set through a PropertiesHolder)
	    //
	    copy.descriptor.properties = (java.util.LinkedList)copy.descriptor.properties.clone();
	}
	return copy;
    }

    public Object getDescriptor()
    {
	return _instanceDescriptor;
    }

    public Object saveDescriptor()
    {
	//
	// Must be a shallow copy
	//
	ServiceInstanceDescriptor saved = 
	    (ServiceInstanceDescriptor)_instanceDescriptor.clone();
	
	if(saved.descriptor != null)
	{
	    saved.descriptor = (ServiceDescriptor)saved.descriptor.clone();
	}
	return saved;
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	ServiceInstanceDescriptor sd = (ServiceInstanceDescriptor)savedDescriptor;
	_instanceDescriptor.template = sd.template;
	_instanceDescriptor.parameterValues = sd.parameterValues;
	if(_instanceDescriptor.descriptor != null)
	{
	    _instanceDescriptor.descriptor.adapters = sd.descriptor.adapters;
	    _instanceDescriptor.descriptor.properties = sd.descriptor.properties;
	    _instanceDescriptor.descriptor.dbEnvs = sd.descriptor.properties;
	    _instanceDescriptor.descriptor.description = sd.descriptor.description;
	    
	    _instanceDescriptor.descriptor.name = sd.descriptor.name;
	    _instanceDescriptor.descriptor.entry = sd.descriptor.entry;
	}
    }

    public Object copy()
    {
	return copyDescriptor(_instanceDescriptor);
    }

    public boolean destroy()
    {
	if(isEphemeral() || isEditable() && _model.canUpdate())
	{
	    Services services = (Services)_parent;
	  
	    if(isEphemeral())
	    {
		services.removeChild(this, true);
	    }
	    else
	    {
		services.removeDescriptor(_instanceDescriptor);
		getEditable().markModified();
		getApplication().applySafeUpdate();
	    }
	    return true;
	}
	else
	{
	    return false;
	}
    }

    public void displayProperties()
    {
	//
	// Temporary
	//
	if(_instanceDescriptor.descriptor == null)
	{
	    super.displayProperties();
	    return;
	}

	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties for " + _id);
	
	if(_instanceDescriptor.descriptor != null)
	{
	    if(_editor == null)
	    {
		_editor = new ServiceEditor(_model.getMainFrame());
	    }
	    _editor.show(this);
	    propertiesFrame.setContent(_editor.getComponent());
	}
	else
	{
	    //
	    // Use instance editor 
	    //
	}
	propertiesFrame.validate();
	propertiesFrame.repaint();
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
	if(_adapters != null)
	{
	    _adapters.unregister();
	}
    }

    Service(String name,
	    String displayString,
	    ServiceInstanceDescriptor instanceDescriptor, 
	    ServiceDescriptor serviceDescriptor,
	    boolean isEditable,
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
	_ephemeral = false;

	_resolver = resolver;

	boolean areChildrenEditable = _instanceDescriptor.template.length() == 0 
	    && isEditable;

	_adapters = new Adapters(serviceDescriptor.adapters, 
				 areChildrenEditable, false, resolver, 
				 application, 
				 _model);
	addChild(_adapters);
	
	_dbEnvs = new DbEnvs(serviceDescriptor.dbEnvs, 
			     areChildrenEditable, resolver, _model);
	addChild(_dbEnvs);
    }

    //
    // New temporary object
    //
    Service(String name,
	    ServiceInstanceDescriptor instanceDescriptor,
	    Model model)
    {
	super("*" + name, model);
	_instanceDescriptor = instanceDescriptor;
	_serviceDescriptor = instanceDescriptor.descriptor;
	_ephemeral = true;
    }


    ServiceDescriptor getServiceDescriptor()
    {
	return _serviceDescriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    boolean isEditable()
    {
	return ((Services)_parent).isEditable();
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;
    private String _displayString;
    private boolean _ephemeral;
    private Utils.Resolver _resolver;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;

    static private ServiceEditor _editor;
    // static private ServiceInstanceEditor _instanceEditor;
}
