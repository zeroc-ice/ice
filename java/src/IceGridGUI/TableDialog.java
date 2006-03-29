// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Frame;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;

import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;

//
// A simple two-column table dialog, to edit properties and properties-like
// maps.
//

public class TableDialog extends JDialog
{
    public TableDialog(Frame parentFrame, String title, 
		       String heading0, String heading1, boolean editKeys)
    {
	super(parentFrame, title + " - IceGrid Admin", true);
	_editKeys = editKeys;
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

	_columnNames = new java.util.Vector(2);
	_columnNames.add(heading0);
	_columnNames.add(heading1);
	
	_table = new JTable();

	Action ok = new AbstractAction("OK")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _cancelled = false;
		    setVisible(false);
		}
	    };
	JButton okButton = new JButton(ok);

	Action cancel = new AbstractAction("Cancel")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    setVisible(false);
		}
	    };
	JButton cancelButton = new JButton(cancel);
	
	if(_editKeys)
	{
	     Action deleteRow = new AbstractAction("Delete selected row(s)")
		 {
		     public void actionPerformed(ActionEvent e) 
		     {
			 if(_table.isEditing()) 
			 {
			     _table.getCellEditor().stopCellEditing();
			 }
			 
			 for(;;)
			 {
			     int selectedRow = _table.getSelectedRow();
			     if(selectedRow == -1)
			     {
				 break;
			     }
			     else
			     {
				 _model.removeRow(selectedRow);
			     }
			 }
		     }
		 };
	     _table.getActionMap().put("delete", deleteRow);
	     _table.getInputMap().put(
		 KeyStroke.getKeyStroke("DELETE"), "delete");
	}
	   
	JScrollPane scrollPane = new JScrollPane(_table);
	scrollPane.setBorder(Borders.DIALOG_BORDER);

	getContentPane().add(scrollPane, BorderLayout.CENTER);
	JPanel buttonBar = 
	    ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);

	getContentPane().add(buttonBar, BorderLayout.SOUTH);
	pack();
    }

    //
    // Returns null when cancel is pressed
    //
    public java.util.TreeMap show(java.util.Map map, JComponent onComponent)
    {
	_cancelled = true;
      
	//
	// Transform map into vector of vectors
	//
	java.util.Vector vector = new java.util.Vector(map.size());
	java.util.Iterator p = map.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    java.util.Vector row = new java.util.Vector(2);
	    row.add(entry.getKey());
	    row.add(entry.getValue());
	    vector.add(row);
	}
	if(_editKeys)
	{
	    java.util.Vector newRow = new java.util.Vector(2);
	    newRow.add("");
	    newRow.add("");
	    vector.add(newRow);
	}

	_model = new DefaultTableModel(vector, _columnNames)
	    {
		public boolean isCellEditable(int row, int column)
		{
		    if(_editKeys)
		    {
			return true;
		    }
		    else
		    {
			return column > 0;
		    }
		}
	    };
	
	_model.addTableModelListener(new TableModelListener()
	    {
		public void tableChanged(TableModelEvent e)
		{
		    if(_editKeys)
		    {
			Object lastKey = _model.getValueAt(
			    _model.getRowCount() - 1 , 0);
			if(lastKey != null && !lastKey.equals(""))
			{
			    _model.addRow(new Object[]{"", ""});
			}
		    }
		}
	    });


	_table.setModel(_model);

	setLocationRelativeTo(onComponent);
	setVisible(true);	

	if(_cancelled)
	{
	    return null;
	}
	else
	{
	    if(_table.isEditing()) 
	    {
		_table.getCellEditor().stopCellEditing();
	    }
	    vector = _model.getDataVector();

	    //
	    // Transform vector into new Map
	    //
	    java.util.TreeMap result = new java.util.TreeMap();
	    p = vector.iterator();
	    while(p.hasNext())
	    {
		java.util.Vector row = (java.util.Vector)p.next();
		
		//
		// Eliminate rows with null or empty keys
		//
		String key = (String)row.elementAt(0);
		if(key != null)
		{
		    if(_editKeys)
		    {
			key = key.trim();
		    }
		    
		    if(!key.equals(""))
		    {
			String val = (String) row.elementAt(1);
			if(val == null)
			{
			    val = "";
			}
			result.put(key, val);
		    }
		}
	    }
	    return result;
	}
    }

    private final boolean _editKeys;
    private boolean _cancelled;
    private JTable _table;
    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
}



