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

class Services extends Parent
{
    Services(java.util.List descriptors,
	     boolean editable,
	     Utils.Resolver resolver, // Null within template
	     Application application)
    {
	super("Services", application.getModel());    
	_descriptors = descriptors;

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
		
		assert templateDescriptor != null;
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
				 editable, 
				 serviceResolver,
				 _model));
	}
    }

    public void cleanup()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Service service = (Service)p.next();
	    service.cleanup();
	}
    }

    private java.util.List _descriptors;
}
