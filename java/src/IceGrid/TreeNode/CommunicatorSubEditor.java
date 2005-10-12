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
import javax.swing.JButton;

import javax.swing.JFrame;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.CommunicatorDescriptor;
import IceGrid.Model;
import IceGrid.TableDialog;
import IceGrid.Utils;

class CommunicatorSubEditor
{
    CommunicatorSubEditor(Editor mainEditor, JFrame parentFrame)
    {
	_mainEditor = mainEditor;
	_properties.setEditable(false);
	
	//
	// _propertiesButton
	//
	_propertiesDialog = new TableDialog(parentFrame, 
					    "Properties",
					    "Name", 
					    "Value", true);
	
	Action openPropertiesDialog = new AbstractAction("...")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    java.util.Map result = 
			_propertiesDialog.show(_propertiesMap, 
					       _mainEditor.getPanel());
		    if(result != null)
		    {
			_mainEditor.updated();
			_propertiesMap = result;
			setPropertiesField();
		    }
		}
	    };
	_propertiesButton = new JButton(openPropertiesDialog);

	_description.getDocument().addDocumentListener(
	    _mainEditor.getUpdateListener());
    }

  
    void append(DefaultFormBuilder builder)
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

	builder.append("Properties");
	builder.append(_properties, _propertiesButton);
	builder.nextLine();
    }

    void writeDescriptor(CommunicatorDescriptor descriptor)
    {
	descriptor.properties = Editor.mapToProperties(_propertiesMap);
	descriptor.description = _description.getText();
    }

    void show(CommunicatorDescriptor descriptor, boolean isEditable)
    {
	Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	isEditable = isEditable && (detailResolver == null);

	_propertiesMap = Editor.propertiesToMap(descriptor.properties);
	setPropertiesField();
	_propertiesButton.setEnabled(isEditable);
	
	_description.setText(
	    Utils.substitute(descriptor.description, detailResolver));
	_description.setEditable(isEditable);
	_description.setOpaque(isEditable);
    }

    private void setPropertiesField()
    {
	final Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
	
	Ice.StringHolder toolTipHolder = new Ice.StringHolder();
	Utils.Stringifier stringifier = new Utils.Stringifier()
	    {
		public String toString(Object obj)
		{
		    java.util.Map.Entry entry = (java.util.Map.Entry)obj;
		    
		    return Utils.substitute((String)entry.getKey(), detailResolver) 
			+ "="
			+ Utils.substitute((String)entry.getValue(), detailResolver);
		}
	    };
	
	_properties.setText(
	    Utils.stringify(_propertiesMap.entrySet(), stringifier,
			    ", ", toolTipHolder));
	_properties.setToolTipText(toolTipHolder.value);
    }


    protected Editor _mainEditor;
 
    private JTextArea _description = new JTextArea(3, 20);

    private JTextField _properties = new JTextField(20);
    private java.util.Map _propertiesMap;
    private TableDialog _propertiesDialog;
    private JButton _propertiesButton = new JButton("...");
}
