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

class ServiceInstances extends Parent
{
    //
    // In server template
    //
    ServiceInstances(java.util.List descriptors, Application application)
    {
	this(descriptors, true, application, null, null, null);
    }

    //
    // In server instance
    //
    ServiceInstances(java.util.List descriptors,
		     boolean editable,
		     Application application, 
		     java.util.Map nodeVariables,
		     java.util.Map parameters,
		     Node node)
    {
	super("Service instances", application.getModel());    
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor descriptor = 
		(ServiceInstanceDescriptor)p.next();

	    ServiceDescriptor serviceDescriptor = null;
	    String serviceId = null;
	    String displayString = null;
	    java.util.Map substitutedParameters = null;

	    if(descriptor.template.length() > 0)
	    {
		TemplateDescriptor templateDescriptor 
		    = application.findServiceTemplateDescriptor(descriptor.template);
		
		if(templateDescriptor != null)
		{
		    serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;
		}
		
		if(node != null) // in server instance
		{
		    //
		    // Substitute parameter values with variables + parameters,
		    // then use variables + these substituted parameter
		    // values on the serviceDescriptor
		    //

		    substitutedParameters =
			Utils.substituteVariables(descriptor.parameterValues,
						  parameters,
						  nodeVariables,
						  application.getVariables());

		    if(serviceDescriptor != null)
		    {
			serviceId = Utils.substituteVariables(serviceDescriptor.name,
							      substitutedParameters,
							      nodeVariables,
							      application.getVariables());
			
			displayString = serviceId + ": " + templateLabel(descriptor.template,
									 substitutedParameters.values());
		    }
		    else
		    {
			serviceId = "????: " + templateLabel(descriptor.template, 
							     substitutedParameters.values());
		    }
		}
		else
		{
		    //
		    // serviceId = TemplateName<unsubstituted param 1, ....>
		    //
		    serviceId = templateLabel(descriptor.template,
					      descriptor.parameterValues.values());
		}
	    }
	    else
	    {
		serviceDescriptor = descriptor.descriptor; // can't be null

		if(node != null) // in server instance
		{
		    //
		    // Use variables and parameters on this serviceDescriptor
		    // 
		    
		    serviceId = Utils.substituteVariables(serviceDescriptor.name,
							  parameters,
							  nodeVariables,
							  application.getVariables());

		}
		else
		{
		    serviceId = serviceDescriptor.name;
		}
	    }

	    addChild(new ServiceInstance(serviceId, 
					 displayString,
					 descriptor, 
					 serviceDescriptor,
					 editable, 
					 parameters,
					 substitutedParameters,
					 nodeVariables,
					 application.getVariables(),
					 _model, 
					 node));
	}
    }

    void unregisterAdapters()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServiceInstance instance = (ServiceInstance)p.next();
	    instance.unregisterAdapters();
	}
    }

    private java.util.List _descriptors;
}
