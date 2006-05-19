// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class CommunicatorSubEditor
{
    CommunicatorSubEditor(Editor mainEditor)
    {
	_mainEditor = mainEditor;
	
	_description.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_description.setToolTipText("An optional description");

	_propertySets.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_properties = new PropertiesField(mainEditor);
	_description.setToolTipText("Property Set References");
    }

  
    void appendProperties(DefaultFormBuilder builder)
    {
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
	
	builder.append("Property Sets");
	builder.append(_propertySets, 3);
	builder.nextLine();

	builder.append("Properties");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");

	builder.nextLine();
	builder.append("");

	builder.nextRow(-6);
	scrollPane = new JScrollPane(_properties);
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
	builder.nextRow(6);
	builder.nextLine();
    }

    void writeDescriptor(CommunicatorDescriptor descriptor)
    {
	descriptor.propertySet.references = 
	    (String[])_propertySets.getList().toArray(new String[0]);
	descriptor.propertySet.properties = _properties.getProperties();
	descriptor.description = _description.getText();
    }

    void show(CommunicatorDescriptor descriptor, boolean isEditable)
    {
	Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	isEditable = isEditable && (detailResolver == null);

	_propertySets.setList(java.util.Arrays.asList(descriptor.propertySet.references),
			      detailResolver);
	_propertySets.setEditable(isEditable);
	
	_properties.setProperties(descriptor.propertySet.properties,
				  descriptor.adapters,
				  detailResolver, isEditable);

	_description.setText(
	    Utils.substitute(descriptor.description, detailResolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
    }

    protected Editor _mainEditor;
 
    private JTextArea _description = new JTextArea(3, 20);
    private ListTextField _propertySets = new ListTextField(20); 
    private PropertiesField _properties;
}
