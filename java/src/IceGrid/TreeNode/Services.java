// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JPopupMenu;

import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Services extends SimpleContainer
{
    static class NewPopupMenu extends JPopupMenu
    {
	NewPopupMenu()
	{
	    _newService = new AbstractAction("New service")
		{
		    public void actionPerformed(ActionEvent e) 
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
			_parent.newService(descriptor);
		    }
		};

	    add(_newService);

	    _newInstance = new AbstractAction("New template instance")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			ServiceInstanceDescriptor descriptor = 
			    new ServiceInstanceDescriptor("",
							  new java.util.TreeMap(),
							  null);
			_parent.newService(descriptor);
		    }
		};

	    add(_newInstance);
	}

	void setParent(Services parent)
	{
	    _parent = parent;
	}

	private Services _parent;
	private Action _newService;
	private Action _newInstance;
    }
    
    public JPopupMenu getPopupMenu()
    {
	if(_isEditable)
	{
	    if(_popup == null)
	    {
		_popup = new NewPopupMenu();
	    }
	    _popup.setParent(this);
	    return _popup;
	}
	else
	{
	    return null;
	}
    }

    static public java.util.LinkedList
    copyDescriptors(java.util.LinkedList descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(Service.copyDescriptor((ServiceInstanceDescriptor)p.next()));
	}
	return copy;
    }

    Services(java.util.List descriptors,
	     Editable editable,
	     Utils.Resolver resolver, // Null within template
	     Application application)
	throws DuplicateIdException
    {
	super("Services", application.getModel());    
	_descriptors = descriptors;
	_isEditable = (editable != null);

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor descriptor = 
		(ServiceInstanceDescriptor)p.next();

	    ServiceDescriptor serviceDescriptor = null;
	    String serviceName = null;
	    String displayString = null;
	    Utils.Resolver serviceResolver = null;
	
	    if(descriptor.template.length() > 0)
	    {
		TemplateDescriptor templateDescriptor 
		    = application.findServiceTemplateDescriptor(descriptor.template);
		
		if(templateDescriptor == null)
		{
		    //
		    // We've just removed this template instance;
		    // cascadeDeleteServiceInstance will later remove this descriptor
		    //
		    assert editable != null;
		    editable.markModified();
		    continue;
		}
		else
		{
		    serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;
		    assert serviceDescriptor != null;
		    
		    if(resolver != null)
		    {
			serviceResolver = new Utils.Resolver(resolver, descriptor.parameterValues);
			serviceName = serviceResolver.substitute(serviceDescriptor.name);
			serviceResolver.put("service", serviceName);
			displayString = serviceName + ": " 
			    + templateLabel(descriptor.template,
					    serviceResolver.getParameters().values());
		    }
		    else
		    {
			//
			// serviceName = TemplateName<unsubstituted param 1, ....>
			//
			serviceName = templateLabel(descriptor.template, 
						    descriptor.parameterValues.values());
			
		    }
		}
	    }
	    else
	    {
		serviceDescriptor = descriptor.descriptor;
		assert serviceDescriptor != null;
		
		if(resolver != null)
		{
		    serviceResolver = new Utils.Resolver(resolver);
		    serviceName = resolver.substitute(serviceDescriptor.name);
		    serviceResolver.put("service", serviceName);
		}
		else
		{
		    serviceName = serviceDescriptor.name;
		}
	    }
	    
	    addChild(new Service(serviceName, 
				 displayString,
				 descriptor, 
				 serviceDescriptor,
				 editable != null, 
				 serviceResolver,
				 application,
				 _model));
	}
    }

    public void unregister()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    service.unregister();
	}
    }

    void newService(ServiceInstanceDescriptor descriptor)
    {
	String baseName = descriptor.descriptor == null ? "NewService" :
	    descriptor.descriptor.name;
	String name = baseName;
	int i = 0;
	while(findChild(name) != null || findChild("*" + name) != null)
	{
	    name = baseName + "-" + (++i);
	}
	
	if(descriptor.descriptor != null)
	{
	    descriptor.descriptor.name = name;
	}

	Service service = new Service(name, descriptor, _model);
	try
	{
	    addChild(service, true);
	}
	catch(DuplicateIdException e)
	{
	    assert false;
	}
	service.setParent(this);
	_model.setSelectionPath(service.getPath());
    }
    
    public void paste(Object descriptor)
    {
	if(_isEditable && descriptor instanceof ServiceInstanceDescriptor)
	{
	    ServiceInstanceDescriptor d = (ServiceInstanceDescriptor)descriptor;
	    newService(Service.copyDescriptor(d));
	}
    }


    void cascadeDeleteServiceInstance(String templateId)
    {
	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor instanceDescriptor = 
		(ServiceInstanceDescriptor)p.next();

	    if(instanceDescriptor.template.equals(templateId))
	    {
		p.remove();
	    }
	}
    }

    boolean isEditable()
    {
	return _isEditable;
    }

    private final boolean _isEditable;
    static private NewPopupMenu _popup;
}
