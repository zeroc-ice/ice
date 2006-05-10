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

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class CommunicatorSubEditor
{
    CommunicatorSubEditor(Editor mainEditor, JFrame parentFrame)
    {
	_mainEditor = mainEditor;
	
	_description.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
	_description.setToolTipText("An optional description");

	_propertySets.setEditable(false);
	_properties = new PropertiesField(mainEditor);

	_propertySetsDialog = new ListDialog(parentFrame, 
					     "Property Set References", true);

	Action openPropertySetsDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.LinkedList result = _propertySetsDialog.show(
			_propertySetsList, _mainEditor.getProperties());
		    if(result != null)
		    {
			_mainEditor.updated();
			_propertySetsList = result;
			setPropertySetsField();
		    }
		}
	    };
	openPropertySetsDialog.putValue(Action.SHORT_DESCRIPTION,
				       "Edit property set references");
	_propertySetsButton = new JButton(openPropertySetsDialog);
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
	builder.append(_propertySets, _propertySetsButton);
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
	    (String[])_propertySetsList.toArray(new String[0]);
	descriptor.propertySet.properties = _properties.getProperties();
	descriptor.description = _description.getText();
    }

    void show(CommunicatorDescriptor descriptor, boolean isEditable)
    {
	Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	isEditable = isEditable && (detailResolver == null);

	_propertySetsList = java.util.Arrays.asList(descriptor.propertySet.references);
	setPropertySetsField();
	_propertySetsButton.setEnabled(isEditable);

	_properties.setProperties(descriptor.propertySet.properties,
				  descriptor.adapters,
				  detailResolver, isEditable);
	
	_description.setText(
	    Utils.substitute(descriptor.description, detailResolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
    }

  
    private void setPropertySetsField()
    {
	final Utils.Resolver resolver = _mainEditor.getDetailResolver();

	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    return Utils.substitute((String)obj, resolver);
		}
	    };
	
	_propertySets.setText(
	    Utils.stringify(_propertySetsList, 
			    stringifier, ", ", toolTipHolder));

	String toolTip = "<html>Property Sets";

	if(toolTipHolder.value != null)
	{
	    toolTip += ":<br>" + toolTipHolder.value;
	}
	toolTip += "</html>";
	_propertySets.setToolTipText(toolTip);
    }

    protected Editor _mainEditor;
 
    private JTextArea _description = new JTextArea(3, 20);

    private JTextField _propertySets = new JTextField(20);
    private java.util.List _propertySetsList;
    private ListDialog _propertySetsDialog;
    private JButton _propertySetsButton;
 
    private PropertiesField _properties;
}
