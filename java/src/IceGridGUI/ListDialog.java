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
// A simple list of strings
//

public class ListDialog extends JDialog
{ 
    public ListDialog(Frame parentFrame, String title, boolean trim)
    {
	super(parentFrame, title + " - IceGrid Admin", true);
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

	_trim = trim;

	_columnNames = new java.util.Vector(1);
	_columnNames.add("item");

	_list = new JTable();
	_list.setTableHeader(null);

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

	Action deleteRow = new AbstractAction("Delete selected element(s)")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    if(_list.isEditing()) 
		    {
			_list.getCellEditor().stopCellEditing();
		    }
		    
		    for(;;)
		    {
			int selectedRow = _list.getSelectedRow();
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
	_list.getActionMap().put("delete", deleteRow);
	_list.getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
	    
	
	JScrollPane scrollPane = new JScrollPane(_list);
	scrollPane.setBorder(Borders.DIALOG_BORDER);

	getContentPane().add(scrollPane, BorderLayout.CENTER);
	JPanel buttonBar = ButtonBarFactory.buildOKCancelBar(okButton, cancelButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);

	getContentPane().add(buttonBar, BorderLayout.SOUTH);
	pack();
    }

    //
    // Returns null when cancel is pressed
    //
    public java.util.LinkedList show(java.util.List elts, JComponent onComponent)
    {
	_cancelled = true;

	java.util.Vector vector = new java.util.Vector();
	java.util.Iterator p = elts.iterator();
	while(p.hasNext())
	{
	    java.util.Vector elt = new java.util.Vector(1);
	    elt.add(p.next());
	    vector.add(elt);
	}

	java.util.Vector newRow = new java.util.Vector(1);
	newRow.add("");
	vector.addElement(newRow);

	_model = new DefaultTableModel(vector, _columnNames);

	_model.addTableModelListener(new TableModelListener()
	    {
		public void tableChanged(TableModelEvent e)
		{
		    Object lastKey = _model.getValueAt(
			_model.getRowCount() - 1 , 0);
		    if(lastKey != null && !lastKey.equals(""))
		    {
			_model.addRow(new Object[]{""});
		    }
		}
	    });

	_list.setModel(_model);

	setLocationRelativeTo(onComponent);
	setVisible(true);	

	if(_cancelled)
	{
	    return null;
	}
	else
	{ 
	    if(_list.isEditing()) 
	    {
		_list.getCellEditor().stopCellEditing();
	    }

	    //
	    // Create results without empty strings
	    //
	    java.util.LinkedList result = new java.util.LinkedList();
	    for(int i = 0; i < _model.getRowCount(); ++i)
	    {
		String value = (String)_model.getValueAt(i, 0);
		if(value != null)
		{
		    if(_trim)
		    {
			value = value.trim();
		    }
		    if(!value.equals(""))
		    {
			result.add(value);
		    }
		}
	    }
	    return result;
	}
    }

    private boolean _cancelled;
    private DefaultTableModel _model;
    private JTable _list;
    private java.util.Vector _columnNames;
    private boolean _trim;
}



