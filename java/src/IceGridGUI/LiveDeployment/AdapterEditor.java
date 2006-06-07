// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultComboBoxModel;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class AdapterEditor extends Editor
{
    AdapterEditor()
    {
	_currentStatus.setEditable(false);
	_currentEndpoints.setEditable(false);
	_description.setEditable(false);
	_description.setOpaque(false);
	_id.setEditable(false);
	_replicaGroupId.setEditable(false);
	_endpoints.setEditable(false);
	_publishedEndpoints.setEditable(false);
	_registerProcess.setEnabled(false);
	_waitForActivation.setEnabled(false);
    }


    void show(Adapter adapter)
    {
	AdapterDescriptor descriptor = adapter.getDescriptor();
	Utils.Resolver resolver = adapter.getResolver();
	
	_id.setText(resolver.substitute(descriptor.id));

	String currentEndpoints = adapter.getCurrentEndpoints();
	    
	if(currentEndpoints == null)
	{
	    if(_id.getText().length() == 0)
	    {
		_currentStatus.setText("Direct adapter (does not register with IceGrid at runtime)");
	    }
	    else
	    {
		_currentStatus.setText("Inactive");
	    }
	    _currentEndpoints.setText("");
	}
	else
	{
	    _currentStatus.setText("Active");
	    _currentEndpoints.setText(currentEndpoints);
	}
	
	_description.setText(resolver.substitute(descriptor.description));
	_replicaGroupId.setText(resolver.substitute(descriptor.replicaGroupId));

	java.util.Map properties = adapter.getProperties();
	
	// getId() returns the name of the adapter!
	_endpoints.setText(resolver.substitute((String)properties.get(adapter.getId() + ".Endpoints")));
	_publishedEndpoints.setText(
	    resolver.substitute((String)properties.get(adapter.getId() + ".PublishedEndpoints")));
	
	_registerProcess.setSelected(descriptor.registerProcess);	
	_waitForActivation.setSelected(descriptor.waitForActivation);

	_objects.setObjects(descriptor.objects, resolver);
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
	builder.appendSeparator("Runtime Status");

	builder.append("Status" );
	builder.append(_currentStatus, 3);
	builder.nextLine();

	builder.append("Published Endpoints" );
	builder.append(_currentEndpoints, 3);
	builder.nextLine();

	builder.appendSeparator("Configuration");
	
	builder.append("Description");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-2);
	CellConstraints cc = new CellConstraints();
	JScrollPane scrollPane = new JScrollPane(_description);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
	builder.nextRow(2);
	builder.nextLine();

	builder.append("Adapter ID");
	builder.append(_id, 3);
	builder.nextLine();
	
	builder.append("Replica Group");
	builder.append(_replicaGroupId, 3);
	builder.nextLine();

	builder.append("Endpoints");
	builder.append(_endpoints, 3);
	builder.nextLine();

	builder.append("Published Endpoints");
	builder.append(_publishedEndpoints, 3);
	builder.nextLine();

	builder.append("", _registerProcess);
	builder.nextLine();
	builder.append("", _waitForActivation);
	builder.nextLine();

	builder.append("Registered Objects");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-6);
	scrollPane = new JScrollPane(_objects);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
	builder.nextRow(6);
	builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Adapter Properties");
    }

    private JTextField _currentStatus = new JTextField(20);
    private JTextField _currentEndpoints = new JTextField(20);

    private JTextArea _description = new JTextArea(3, 20);
    private JTextField _id = new JTextField(20);
    private JTextField _replicaGroupId = new JTextField(20);
    private JTextField _endpoints = new JTextField(20);
    private JTextField _publishedEndpoints  = new JTextField(20);

    private JCheckBox _registerProcess = new JCheckBox("Register Process");
    private JCheckBox _waitForActivation = new JCheckBox("Wait for Activation");
    
    private TableField _objects = new TableField("Identity", "Type");
}
