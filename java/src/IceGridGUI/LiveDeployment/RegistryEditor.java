// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.KeyStroke;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class RegistryEditor extends Editor
{
    RegistryEditor()
    {
	Action deleteObject = new AbstractAction("Remove selected object")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int selectedRow = _objects.getSelectedRow();
		    if(selectedRow != -1)
		    {
			_target.removeObject((String)_objects.getValueAt(selectedRow, 0));
		    }
		}
	    };
	_objects.getActionMap().put("delete", deleteObject);
	_objects.getInputMap().put(
	    KeyStroke.getKeyStroke("DELETE"), "delete");

	
	Action addObject = new AbstractAction("Add a new well-known object")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.addObject();
		}
	    };
	_objects.getActionMap().put("insert", addObject);
	_objects.getInputMap().put(
	    KeyStroke.getKeyStroke("INSERT"), "insert");

	_objects.setToolTipText("<html>Well-known objects registered through the Admin interface.<br>"
				+ "Well-known objects registered using Adapter or Replica Group<br>" 
				+ "definitions are not displayed here.</html>");
       
	Action deleteAdapter = new AbstractAction("Remove selected adapter")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    int selectedRow = _adapters.getSelectedRow();
		    if(selectedRow != -1)
		    {
			_target.removeAdapter((String)_adapters.getValueAt(selectedRow, 0));
		    }
		}
	    };
	_adapters.getActionMap().put("delete", deleteAdapter);
	_adapters.getInputMap().put(
	    KeyStroke.getKeyStroke("DELETE"), "delete");
	_adapters.setToolTipText("<html>Object adapters registered at run time.</html>");
    }
    
    protected void appendProperties(DefaultFormBuilder builder)
    {
	CellConstraints cc = new CellConstraints();

	builder.appendSeparator("Dynamic Well-Known Objects");
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-14);
	JScrollPane scrollPane = new JScrollPane(_objects);
	scrollPane.setToolTipText(_objects.getToolTipText());
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 14));
	builder.nextRow(14);
	builder.nextLine();

	builder.appendSeparator("Dynamic Object Adapters");
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextLine();
	builder.append("");
	builder.nextRow(-14);
	scrollPane = new JScrollPane(_adapters);
	scrollPane.setToolTipText(_adapters.getToolTipText());
	builder.add(scrollPane, 
		    cc.xywh(builder.getColumn(), builder.getRow(), 3, 14));
	builder.nextRow(14);
	builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Registry Properties");
    }

    void show(Root root)
    {
	_target = root;
	_objects.setObjects(root.getObjects());
	_adapters.setAdapters(root.getAdapters());
    }


    private TableField _objects = new TableField("Proxy", "Type");
    private TableField _adapters = new TableField("ID", "Endpoints", "Replica Group");
    private Root _target;
}
