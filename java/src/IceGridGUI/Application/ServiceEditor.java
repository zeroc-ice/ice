// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JFrame;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class ServiceEditor extends CommunicatorChildEditor
{
    ServiceEditor(JFrame parentFrame)
    {
	_subEditor = new ServiceSubEditor(this, parentFrame);
    }
 
    //
    // From Editor:
    //
    Utils.Resolver getDetailResolver()
    {
	Service service = (Service)_target;
	if(service.getCoordinator().substitute())
	{
	    return service.getResolver();
	}
	else
	{
	    return null;
	}
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {    
	_subEditor.appendProperties(builder);
    }
    
    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Service Properties");
    }


    void writeDescriptor()
    {
	_subEditor.writeDescriptor();
    }	    
    
    boolean isSimpleUpdate()
    {
	return _subEditor.isSimpleUpdate(); 
    }
    
    Communicator.ChildList getChildList()
    {
	return ((Communicator)_target.getParent()).getServices();
    }

    Object getSubDescriptor()
    {
	return ((Service)_target).getServiceDescriptor();
    }

    void show(Service service)
    {
	//
	// If it's a template instance, it's shown using
	// ServiceInstanceEditor.show()
	//
	assert ((ServiceInstanceDescriptor)service.getDescriptor()).descriptor 
	    != null;

	detectUpdates(false);
	_target = service;

	_subEditor.show(service.isEditable());

	_applyButton.setEnabled(service.isEphemeral());
	_discardButton.setEnabled(service.isEphemeral());	  
	detectUpdates(true);
    }

    private ServiceSubEditor _subEditor;
}
