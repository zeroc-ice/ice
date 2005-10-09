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
// A simple list of strings
//

public class ListDialog extends JDialog
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
		_popup.show(e.getX(), e.getY());
	    }
	}
    }
    
    class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    _addRow = new AbstractAction("Add a new element")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			java.util.Vector newRow = null;
			_model.addRow(newRow);
		    }
		};
	    
	    _deleteRow = new AbstractAction("Delete selected element(s)")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
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

	    add(_addRow);
	    add(_deleteRow);
	}
	
	void show(int x, int y)
	{
	    _deleteRow.setEnabled(_list.getSelectedRowCount() > 0);
	    show(_list, x, y);
	}

	private Action _addRow;
	private Action _deleteRow;
    }
    
    public ListDialog(Frame parentFrame, String title)
    {
	super(parentFrame, title, true);
	setDefaultCloseOperation(JDialog.HIDE_ON_CLOSE);
	
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
	
	JScrollPane scrollPane = new JScrollPane(_list);
	
	PopupListener popupListener = new PopupListener();
	_list.addMouseListener(popupListener);
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
	if(vector.size() == 0)
	{
	    vector.addElement(new java.util.Vector(1));
	}
	
	_model = new DefaultTableModel(vector, _columnNames);
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
		if(!_model.getValueAt(i, 0).equals(""))
		{
		    result.add(_model.getValueAt(i, 0));
		}
	    }
	    return result;
	}
    }

    private boolean _cancelled;
    private DefaultTableModel _model;
    private JTable _list;
    private java.util.Vector _columnNames;
    private PopupMenu _popup = new PopupMenu();
}



