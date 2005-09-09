// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.awt.BorderLayout;
import java.awt.Dimension;
import java.awt.Frame;

import java.awt.event.ActionEvent;
import java.awt.event.MouseAdapter;
import java.awt.event.MouseEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JDialog;
import javax.swing.JPanel;
import javax.swing.JPopupMenu;
import javax.swing.JScrollPane;
import javax.swing.JTable;
import javax.swing.table.DefaultTableModel;

import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;

//
// A simple two-column table dialog, to edit properties and properties-like
// maps.
//

public class TableDialog extends JDialog
{
    class PopupListener extends MouseAdapter
    {
	public void mousePressed(MouseEvent e) 
	{
	    maybeShowPopup(e);
	}

	public void mouseReleased(MouseEvent e) 
	{
	    maybeShowPopup(e);
	}

	private void maybeShowPopup(MouseEvent e) 
	{
	    if (e.isPopupTrigger()) 
	    {	
		_popup.show( e.getX(), e.getY());
	    }
	}
    }
    
    class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    _addRow = new AbstractAction("Add a new row")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			java.util.Vector newRow = null;
			_model.addRow(newRow);
		    }
		};
	    
	    _deleteRow = new AbstractAction("Delete selected row(s)")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
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

	    add(_addRow);
	    add(_deleteRow);
	}
	
	void show(int x, int y)
	{
	    _deleteRow.setEnabled(_table.getSelectedRowCount() > 0);
	    show(_table, x, y);
	}

	private Action _addRow;
	private Action _deleteRow;
    }
    
    public TableDialog(Frame parentFrame, String title, String heading0, String heading1)
    {
	super(parentFrame, title, true);
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);

	_columnNames = new java.util.Vector(2);
	_columnNames.add(heading0);
	_columnNames.add(heading1);
	
	_table = new JTable();
	PopupListener popupListener = new PopupListener();
	_table.addMouseListener(popupListener);

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
	
	JScrollPane scrollPane = new JScrollPane(_table);
	scrollPane.addMouseListener(popupListener);
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
    public java.util.Map show(java.util.Map map, JComponent onComponent)
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
	if(vector.size() == 0)
	{
	    vector.add(new java.util.Vector(2));
	}

	_model = new DefaultTableModel(vector, _columnNames);
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
	    java.util.Map result = new java.util.HashMap(vector.size());
	    p = vector.iterator();
	    while(p.hasNext())
	    {
		java.util.Vector row = (java.util.Vector)p.next();
		
		//
		// Eliminate rows with null or empty keys
		//
		String key = (String)row.elementAt(0);
		if(key != null && key.length() > 0)
		{
		    result.put(key, row.elementAt(1));
		}
	    }
	    return result;
	}
    }

    private boolean _cancelled;
    private JTable _table;
    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
    private PopupMenu _popup = new PopupMenu();
}



