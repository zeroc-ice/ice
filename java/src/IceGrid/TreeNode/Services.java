// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Services extends ListParent
{
    static public java.util.LinkedList
    copyDescriptors(java.util.LinkedList descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(Service.copyDescriptor(
			 (ServiceInstanceDescriptor)p.next()));
	}
	return copy;
    }

    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	
	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = isEditable() &&  
		descriptor instanceof ServiceInstanceDescriptor;
	}
	actions[NEW_SERVICE] = isEditable();
	actions[NEW_SERVICE_FROM_TEMPLATE] = isEditable();
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	    JMenuItem newServiceItem = new JMenuItem(_model.getActions()[NEW_SERVICE]);
	    newServiceItem.setText("New service");
	    _popup.add(newServiceItem);
	    JMenuItem newServiceFromTemplateItem = 
		new JMenuItem(_model.getActions()[NEW_SERVICE_FROM_TEMPLATE]);
	    newServiceFromTemplateItem.setText("New service from template");
	    _popup.add(newServiceFromTemplateItem); 
	}
	return _popup;
    }
    
    public void paste()
    {
	Object descriptor =  _model.getClipboard();
	ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)descriptor;
	newService(Service.copyDescriptor(d));
    }

    public void newService()
    {
	ServiceDescriptor sd = 
	    new ServiceDescriptor(new java.util.LinkedList(),
				  new java.util.LinkedList(),
				  new java.util.LinkedList(),
				  "",
				  "NewService",
				  "");
	
	ServiceInstanceDescriptor descriptor = 
	    new ServiceInstanceDescriptor("",
					  new java.util.TreeMap(),
					  sd);
	newService(descriptor);
    }

    public void newServiceFromTemplate()
    {
	ServiceInstanceDescriptor descriptor = 
	    new ServiceInstanceDescriptor("",
					  new java.util.HashMap(),
					  null);
	newService(descriptor);
    }

    
    private Service createService(ServiceInstanceDescriptor descriptor,
				  Application application) throws UpdateFailedException
    {
	ServiceDescriptor serviceDescriptor = null;
	String serviceName = null;
	String displayString = null;
	Utils.Resolver serviceResolver = null;
	
	if(descriptor.template.length() > 0)
	{
	    TemplateDescriptor templateDescriptor 
		= application.findServiceTemplateDescriptor(descriptor.template);
	    
	    assert templateDescriptor != null;
	    
	    serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;
	    assert serviceDescriptor != null;
	    
	    if(_resolver != null)
	    {
		serviceResolver = new Utils.Resolver(_resolver, 
						     descriptor.parameterValues,
						     templateDescriptor.parameterDefaults);
		serviceName = serviceResolver.substitute(serviceDescriptor.name);
		serviceResolver.put("service", serviceName);
		displayString = serviceName + ": " + descriptor.template + "<>";
	    }
	    else
	    {
		//
		// serviceName = TemplateName<unsubstituted param 1, ....>
		//
		serviceName = templateLabel(descriptor.template, 
					    templateDescriptor.parameters,
					    descriptor.parameterValues,
					    templateDescriptor.parameterDefaults);
					    
	    }
	}
	else
	{
	    serviceDescriptor = descriptor.descriptor;
	    assert serviceDescriptor != null;
	    
	    if(_resolver != null)
	    {
		serviceResolver = new Utils.Resolver(_resolver);
		serviceName = _resolver.substitute(serviceDescriptor.name);
		serviceResolver.put("service", serviceName);
	    }
	    else
	    {
		serviceName = serviceDescriptor.name;
	    }
	}
	   
	return new Service(serviceName,
			   displayString,
			   descriptor, 
			   serviceDescriptor,
			   _isEditable, 
			   serviceResolver,
			   _model);
    }

    Services(java.util.List descriptors,
	     boolean isEditable,
	     Utils.Resolver resolver, // Null within template
	     Application application)
	throws UpdateFailedException
    {
	super("Services", application.getModel());    
	_descriptors = descriptors;
	_isEditable = isEditable;
	_resolver = resolver;

	sortChildren(false);

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor descriptor = 
		(ServiceInstanceDescriptor)p.next();

	    try
	    {
		addChild(createService(descriptor, application));
	    }
	    catch(UpdateFailedException e)
	    {
		e.addParent(this);
		throw e;
	    }
	}
    }

    private void newService(ServiceInstanceDescriptor descriptor)
    {
	String baseName = descriptor.descriptor == null ? "NewService" :
	    descriptor.descriptor.name;
	String name = makeNewChildId(baseName);
	
	if(descriptor.descriptor != null)
	{
	    descriptor.descriptor.name = name;
	}
	else
	{
	    //
	    // Make sure descriptor.template points to a real template
	    //
	    ServiceTemplate t = getApplication().findServiceTemplate(descriptor.template);

	    if(t == null)
	    {
		t = (ServiceTemplate)getApplication().getServiceTemplates().getChildAt(0);

		if(t == null)
		{
		    JOptionPane.showMessageDialog(
			_model.getMainFrame(),
			"You need to create a service template before you can create a service from a template.",
			"No Service Template",
			JOptionPane.INFORMATION_MESSAGE);
		    return;
		}
		else
		{
		    descriptor.template = t.getId();
		    descriptor.parameterValues = new java.util.HashMap();
		}
	    }

	    //
	    // Validate/update parameterValues
	    //
	    TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
	    descriptor.parameterValues = Editor.makeParameterValues(descriptor.parameterValues,
								    td.parameters);

	}

	Service service = new Service(name, descriptor, _model);
	try
	{
	    addChild(service, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(service.getPath());
    }
    
    boolean isEditable()
    {
	return _isEditable;
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    ServiceInstanceDescriptor d = 
		(ServiceInstanceDescriptor)service.getDescriptor();
	    if(d.template.equals(template))
	    {
		result.add(service);
	    }
	}
	return result;
    }


    void removeServiceInstances(String template)
    {
	java.util.List toRemove = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    ServiceInstanceDescriptor d = 
		(ServiceInstanceDescriptor)service.getDescriptor();
	    if(d.template.equals(template))
	    {
		removeDescriptor(d);
		toRemove.add(service.getId());
	    }
	}

	if(toRemove.size() > 0)
	{
	    getEditable().markModified();
	    removeChildren((String[])toRemove.toArray(new String[0]));
	}
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }
    
    boolean canMove(Service service, boolean up)
    {
	if(_isEditable)
	{
	    int index = _descriptors.indexOf(service.getDescriptor());
	    assert index != -1;
	    if(up && index == 0 || !up && (index == _descriptors.size() - 1))
	    {
		return false;
	    } 
	    else
	    {
		return true;
	    }
	}
	return false;
    }

    void move(Service service, boolean up)
    {
	int index = _descriptors.indexOf(service.getDescriptor());
	
	if(_model.canUpdate())
	{   
	    getEditable().markModified();
	    
	    Object descriptor = _descriptors.remove(index);
	    if(up)
	    {
		_descriptors.add(index - 1, descriptor);
	    }
	    else
	    {
		_descriptors.add(index + 1, descriptor);
	    }
	    moveChild(index, up, true);

	    //
	    // Propagate to instances
	    //
	    if(_parent instanceof ServerTemplate)
	    {
		java.util.List instances = 
		    getApplication().findServerInstances(_parent.getId());
		java.util.Iterator p = instances.iterator();
		while(p.hasNext())
		{
		    Server server = (Server)p.next();

		    //
		    // The descriptors are the same as `this` descriptors
		    //
		    server.getServices().moveChild(index, up, true);
		}
	    }	
	}    
	//
	// Recompute actions
	// 
	_model.showActions(_model.getSelectedNode());
    }

    
    CommonBase addNewChild(Object d) throws UpdateFailedException
    {
	ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)d;
       
	try
	{
	    Service service = createService(descriptor, getApplication());
	    addChild(service, true);
	    return service;
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}
    }


    Object rebuildChild(CommonBase child, java.util.List editables) 
	throws UpdateFailedException
    {
	int index = getIndex(child);
	assert index != -1;
	removeChild(child, true);
	ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)child.getDescriptor();
	
	java.util.Map savedParameterValues = null;

	if(descriptor.template.length() > 0)
	{
	    TemplateDescriptor templateDescriptor 
		= getApplication().findServiceTemplateDescriptor(descriptor.template);

	    java.util.Set parameters = new java.util.HashSet(templateDescriptor.parameters);
	    if(!parameters.equals(descriptor.parameterValues.keySet()))
	    {
		savedParameterValues = descriptor.parameterValues;
		descriptor.parameterValues = Editor.makeParameterValues(
		    descriptor.parameterValues, templateDescriptor.parameters);
		editables.add(getEditable());
	    }
	}
	
	try
	{
	    Service newChild = createService(descriptor, getApplication());
	    addChild(index, newChild, true);
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    //
	    // Restore
	    //
	    if(savedParameterValues != null)
	    {
		descriptor.parameterValues = savedParameterValues;
	    }
	    addChild(index, child, true);
	
	    throw e;
	}

	return savedParameterValues;
    }

    void restoreChild(CommonBase child, Object backup)
    {
	java.util.Map savedParameterValues = (java.util.Map)backup;

	ServiceInstanceDescriptor descriptor = 
	    (ServiceInstanceDescriptor)child.getDescriptor();

	if(savedParameterValues != null)
	{
	    descriptor.parameterValues = savedParameterValues;
	}
	
	CommonBase badChild = findChildWithDescriptor(descriptor);
	int index = getIndex(badChild);
	assert index != -1;
	removeChild(badChild, true);

	try
	{
	    addChild(index, child, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
    }

    private final boolean _isEditable;
    private final Utils.Resolver _resolver;

    static private JPopupMenu _popup;
}
