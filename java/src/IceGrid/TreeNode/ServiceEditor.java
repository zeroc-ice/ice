// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JFrame;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Utils;

class ServiceEditor extends ListElementEditor
{
    ServiceEditor(JFrame parentFrame)
    {
	super(false);
	_subEditor = new ServiceSubEditor(this, parentFrame);
    }
 
    //
    // From Editor:
    //
    Utils.Resolver getDetailResolver()
    {
	Service service = (Service)_target;
	if(service.getModel().substitute())
	{
	    return service.getResolver();
	}
	else
	{
	    return null;
	}
    }

    void appendProperties(DefaultFormBuilder builder)
    {    
	_subEditor.appendProperties(builder);
    }
    
    void writeDescriptor()
    {
	_subEditor.writeDescriptor();
    }	    
    
    boolean isSimpleUpdate()
    {
	return _subEditor.isSimpleUpdate(); 
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
	setTarget(service);

	_subEditor.show(service.isEditable());

	_applyButton.setEnabled(service.isEphemeral());
	_discardButton.setEnabled(service.isEphemeral());	  
	detectUpdates(true);
    }

    private ServiceSubEditor _subEditor;
}
