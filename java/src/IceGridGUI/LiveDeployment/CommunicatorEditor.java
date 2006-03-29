// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.JTextArea;

import javax.swing.table.DefaultTableModel;
import javax.swing.table.DefaultTableCellRenderer;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class CommunicatorEditor extends Editor
{
    protected CommunicatorEditor()
    {
	_description.setEditable(false);
	_description.setOpaque(false);

	_propertiesModel = new DefaultTableModel()
	    {
		public boolean isCellEditable(int row, int column)
		{
		    return false;
		}
	    };

	_properties = new JTable(_propertiesModel);
	_properties.setCellSelectionEnabled(false);
	_properties.setOpaque(false);
	_properties.setPreferredScrollableViewportSize(_properties.getPreferredSize());
	
	_columnNames.add("Name");
	_columnNames.add("Value");
    }

    protected void show(CommunicatorDescriptor descriptor, Utils.Resolver resolver)
    {
	_description.setText(resolver.substitute(descriptor.description));
	_propertiesModel.setDataVector(
	    mapToVector(propertiesToMap(descriptor.properties, resolver)),
	    _columnNames);

	DefaultTableCellRenderer cr = (DefaultTableCellRenderer)
	    _properties.getDefaultRenderer(String.class);
	cr.setOpaque(false);
	
	// _properties.setAutoResizeMode(JTable.AUTO_RESIZE_OFF);
	// _properties.doLayout();
    }

    protected void appendProperties(DefaultFormBuilder builder)
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

    private JTextArea _description = new JTextArea(3, 20);
    private JTable _properties;
    private DefaultTableModel _propertiesModel;
    private java.util.Vector _columnNames = new java.util.Vector(2);
}

