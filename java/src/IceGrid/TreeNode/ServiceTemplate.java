// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.SimpleInternalFrame;

import IceGrid.ServiceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Model;

class ServiceTemplate extends EditableParent
{
    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties for " + _id);
       
	if(_editor == null)
	{
	    _editor = new ServiceTemplateEditor(_model.getMainFrame());
	}
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }


    ServiceTemplate(boolean brandNew, String name, 
		    TemplateDescriptor descriptor, Model model)
	throws UpdateFailedException
    {
	super(brandNew, name, model);
	_ephemeral = false;
	rebuild(descriptor);
    }
    
    ServiceTemplate(String name, TemplateDescriptor descriptor, Model model)
    {
	super(true, name, model);
	_ephemeral = true;
	try
	{
	    rebuild(descriptor);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

    ServiceTemplate(ServiceTemplate o)
    {
	super(o, true);
	assert o._ephemeral == false;

	_templateDescriptor = o._templateDescriptor;
	_adapters = o._adapters;
	_dbEnvs = o._dbEnvs;
	_propertiesHolder = o._propertiesHolder;
    }


    void rebuild(TemplateDescriptor descriptor)
	throws UpdateFailedException
    {
	_templateDescriptor = descriptor;
	_propertiesHolder = new PropertiesHolder(_templateDescriptor.descriptor);

	//
	// Fix-up parameters order
	//
	java.util.Collections.sort(_templateDescriptor.parameters);
	
	clearChildren();
	if(_ephemeral)
	{
	    _adapters = null;
	    _dbEnvs = null;
	}
	else
	{
	    _adapters = new Adapters(_templateDescriptor.descriptor.adapters, true, 
				     false, null, _model);
	    addChild(_adapters);
	    
	    _dbEnvs = new DbEnvs(_templateDescriptor.descriptor.dbEnvs, true,
				 null, _model);
	    addChild(_dbEnvs);
	}
    }

    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public String toString()
    {
	if(_ephemeral)
	{
	    return "*" + _id;
	}
	else
	{
	    return templateLabel(_id, _templateDescriptor.parameters);
	}
    }

    public Object getDescriptor()
    {
	return _templateDescriptor;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }
    
    public boolean destroy()
    {
	if(_parent != null && _model.canUpdate())
	{
	    ServiceTemplates serviceTemplates = (ServiceTemplates)_parent;
	 
	    if(_ephemeral)
	    {
		serviceTemplates.removeChild(this, true);
	    }
	    else
	    {
		serviceTemplates.removeDescriptor(_id);
		getApplication().removeServiceInstances(_id);
		serviceTemplates.removeElement(this, true);
	    }
	    return true;
	}
	else
	{
	    return false;
	}
    }

    public java.util.List findAllInstances(CommonBase child)
    {
	java.util.List result = super.findAllInstances(child);
	
	java.util.List serviceInstances = 
	    getApplication().findServiceInstances(_id);

	java.util.Iterator p = serviceInstances.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    result.addAll(service.findChildrenWithType(child.getClass()));
	}

	return result;
    }

   
    public Object saveDescriptor()
    {
	//
	// Shallow copy
	//
	TemplateDescriptor clone = (TemplateDescriptor)_templateDescriptor.clone();
	clone.descriptor = (ServiceDescriptor)_templateDescriptor.descriptor.clone();
	return clone;
    }
    
    public void restoreDescriptor(Object savedDescriptor)
    {
	TemplateDescriptor clone = (TemplateDescriptor)savedDescriptor;
	//
	// Keep the same object
	//
	_templateDescriptor.parameters = clone.parameters;

	ServiceDescriptor sd = (ServiceDescriptor)_templateDescriptor.descriptor;
	ServiceDescriptor csd = (ServiceDescriptor)clone.descriptor;

	sd.properties = csd.properties;
	sd.description = csd.description;
	sd.name = csd.name;
	sd.entry = csd.entry;
    }

    private TemplateDescriptor _templateDescriptor;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    private PropertiesHolder _propertiesHolder;
    private boolean _ephemeral;
    

    static private ServiceTemplateEditor _editor;
}
