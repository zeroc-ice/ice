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
import javax.swing.JFrame;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class ServiceSubEditor extends CommunicatorSubEditor
{
    ServiceSubEditor(Editor mainEditor, JFrame parentFrame)
    {
	super(mainEditor, parentFrame);

	_name.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_entry.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
    }
 
    ServiceDescriptor getServiceDescriptor()
    {
	return (ServiceDescriptor)
	    _mainEditor.getSubDescriptor();
    }
    
    void append(DefaultFormBuilder builder)
    {    
	builder.append("Name");
	builder.append(_name, 3);
	builder.nextLine();
	
	//
	// Add Communicator fields
	//
	super.append(builder);

	builder.append("Entry");
	builder.append(_entry, 3);
	builder.nextLine();
    }
    
    void writeDescriptor()
    {
	ServiceDescriptor descriptor = getServiceDescriptor();
	descriptor.name = _name.getText();
	descriptor.entry = _entry.getText();
	super.writeDescriptor(descriptor);
    }	    
    
    boolean isSimpleUpdate()
    {
	return getServiceDescriptor().name.equals(_name.getText()); 
    }

    void show(boolean isEditable)
    {
	ServiceDescriptor descriptor = getServiceDescriptor();
	Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	
	isEditable = isEditable && (detailResolver == null);

	if(detailResolver != null)
	{
	    _name.setText(detailResolver.find("service"));
	}
	else
	{
	    _name.setText(descriptor.name);
	}
	_name.setEditable(isEditable);
	
	_entry.setText(
	    Utils.substitute(descriptor.entry, detailResolver));
	_entry.setEditable(isEditable);

	show(descriptor, isEditable);
    }

    private JTextField _name = new JTextField(20);
    private JTextField _entry = new JTextField(20);
}
