// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.JTextField;
import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class ServiceEditor extends CommunicatorEditor
{
    ServiceEditor()
    {
	_entry.setEditable(false);
    }

    void show(Service service)
    {
	ServiceDescriptor descriptor = service.getServiceDescriptor();
	Utils.Resolver resolver = service.getResolver();

	String application = ((Server)service.getParent()).getApplication().name;
	Node node = (Node)service.getParent().getParent();

	ExpandedPropertySet propertySet = 
	    node.expand(descriptor.propertySet, application);	

	ExpandedPropertySet instancePropertySet = null;
	ServiceInstanceDescriptor instanceDescriptor = 
	    service.getInstanceDescriptor();
	if(instanceDescriptor != null)
	{
	    instancePropertySet = node.expand(instanceDescriptor.propertySet,
					      application);	      
	}

	show(descriptor, propertySet, instancePropertySet, resolver);
	_entry.setText(resolver.substitute(descriptor.entry));
    }


    protected void appendProperties(DefaultFormBuilder builder)
    {
	super.appendProperties(builder);

	builder.append("Entry Point");
	builder.append(_entry, 3);
	builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Service Properties");
    }

    private JTextField _entry = new JTextField(20);
}

