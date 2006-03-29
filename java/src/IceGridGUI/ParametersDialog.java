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
import javax.swing.DefaultCellEditor;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JComboBox;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;

//
// A simple two-column table dialog to edit parameter-with-defaults
// and parameter-values
//

public class ParametersDialog extends JDialog
{
    
    public ParametersDialog(Frame parentFrame, String title, 
			    String valHeading, boolean editNames,
			    final String nullLabel)
    {
	super(parentFrame, title + " - IceGrid Admin", true);
	_editNames = editNames;
	_nullObject = new Object()
	    {
		public String toString()
		{
		    return nullLabel;
		}
	    };

	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

	_columnNames = new java.util.Vector(2);
	_columnNames.add("Name");
	_columnNames.add(valHeading);

	_table = new JTable();

	JComboBox comboBox = new JComboBox();
	comboBox.setEditable(true);
	comboBox.addItem(_nullObject);
	_cellEditor = new DefaultCellEditor(comboBox);

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
	
	if(_editNames)
	{
	    Action deleteRow = new AbstractAction("Delete selected parameter(s)")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			for(;;)
			{
			    if(_table.isEditing()) 
			    {
				_table.getCellEditor().stopCellEditing();
			    }

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
	JPanel buttonBar = ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);

	getContentPane().add(buttonBar, BorderLayout.SOUTH);
	pack();
    }

    //
    // true=ok, false=cancel
    //
    public boolean show(java.util.List names, 
			java.util.Map values, 
			JComponent onComponent)
    {
	_cancelled = true;
      
	//
	// Transform into vector of vectors
	//
	java.util.Vector vector = new java.util.Vector(names.size());
	java.util.Iterator p = names.iterator();

	while(p.hasNext())
	{
	    java.util.Vector row = new java.util.Vector(2);
	    Object name = p.next();
	    row.add(name);
	    Object val = values.get(name);
	    if(val == null)
	    {
		row.add(_nullObject);
	    }
	    else
	    {
		row.add(val);
	    }
	    vector.add(row);
	}
	
	if(_editNames)
	{
	    java.util.Vector newRow = new java.util.Vector(2);
	    newRow.add("");
	    newRow.add(_nullObject);
	    vector.add(newRow);
	}

	_model = new DefaultTableModel(vector, _columnNames)
	    {
		public boolean isCellEditable(int row, int column)
		{
		    if(_editNames)
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
		    if(_editNames)
		    {
			Object lastKey = _model.getValueAt(
			    _model.getRowCount() - 1 , 0);
			if(lastKey != null && !lastKey.equals(""))
			{
			    _model.addRow(new Object[]{"", _nullObject});
			}
		    }
		}
	    });

	_table.setModel(_model);

	TableColumn valColumn = _table.getColumnModel().getColumn(1);
	valColumn.setCellEditor(_cellEditor);

	setLocationRelativeTo(onComponent);
	setVisible(true);	

	if(_cancelled)
	{
	    return false;
	}
	else
	{
	    if(_table.isEditing()) 
	    {
		_table.getCellEditor().stopCellEditing();
	    }
	    vector = _model.getDataVector();

	    //
	    // Transform vector into list & map
	    //
	    if(_editNames)
	    {
		names.clear();
	    }
	    values.clear();
	    
	    p = vector.iterator();
	    while(p.hasNext())
	    {
		java.util.Vector row = (java.util.Vector)p.next();
		
		//
		// Eliminate rows with null or empty names
		//
		String name = (String)row.elementAt(0);
		if(name != null)
		{
		    if(_editNames)
		    {
			name = name.trim();
		    }
 
		    if(!name.equals(""))
		    {
			if(_editNames)
			{
			    names.add(name);
			}
			
			Object val = row.elementAt(1);
			
			//
			// Eliminate entries with "default" value
			//
			if(val != _nullObject)
			{
			    assert val != null;
			    values.put(name, val);
			}
		    }
		}
	    }
	    return true;
	}
    }

    private final boolean _editNames;
    private final Object _nullObject;
    private boolean _cancelled;
    private JTable _table;
    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
    private TableCellEditor _cellEditor;
}



