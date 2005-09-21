// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.BorderLayout;
import java.awt.Component;
import java.awt.event.ActionEvent;
import java.awt.event.ActionListener;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JTree;

import javax.swing.event.DocumentListener;

import javax.swing.tree.TreePath;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Model;
import IceGrid.ObjectDescriptor;
import IceGrid.TableDialog;
import IceGrid.Utils;

class ServiceEditor extends CommunicatorEditor
{
    ServiceEditor(JFrame parentFrame)
    {
	super(parentFrame);

	_name.getDocument().addDocumentListener(_updateListener);
	_entry.getDocument().addDocumentListener(_updateListener);
    }
 
    protected Utils.Resolver getResolver()
    {
	if(_target instanceof Service)
	{
	    Service service = (Service)_target;

	    if(service.getModel().substitute())
	    {
		return service.getResolver();
	    }
	}
	return null;
    }

    ServiceDescriptor getServiceDescriptor()
    {
	if(_target instanceof Service)
	{
	    return ((Service)_target).getServiceDescriptor();
	}
	else
	{
	    return null;
	}
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
    
  
    //
    // From Editor:
    //
    void writeDescriptor()
    {
	ServiceDescriptor descriptor = getServiceDescriptor();
	descriptor.name = _name.getText();
	descriptor.entry = _entry.getText();
    }	    
    
    boolean isSimpleUpdate()
    {
	return getServiceDescriptor().name.equals(_name.getText()); 
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

	ServiceDescriptor descriptor = service.getServiceDescriptor();
	Model model = service.getModel();

	Utils.Resolver resolver = getResolver();
	
	boolean isEditable = service.isEditable() && resolver == null;
	
	_name.setText(
	    Utils.substitute(descriptor.name, resolver));
	_name.setEditable(isEditable);
	
	_entry.setText(
	    Utils.substitute(descriptor.entry, resolver));
	_entry.setEditable(isEditable);

	show(descriptor, isEditable);

	_applyButton.setEnabled(service.isEphemeral());
	_discardButton.setEnabled(service.isEphemeral());	  
	detectUpdates(true);
    }

    private JTextField _name = new JTextField(20);
    private JTextField _entry = new JTextField(20);
}
