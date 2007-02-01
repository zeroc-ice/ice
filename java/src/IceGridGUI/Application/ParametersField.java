// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import IceGrid.*;
import IceGridGUI.*;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.JTextField;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

//
// A special field used to show/edit parameters
//

public class ParametersField extends JTable
{
    public ParametersField(Editor editor)
    {
        _editor = editor;

        _columnNames = new java.util.Vector(2);
        _columnNames.add("Name");
        _columnNames.add("Default value");

        JComboBox comboBox = new JComboBox(
            new Object[]{_noDefault});
        comboBox.setEditable(true);
        _cellEditor = new DefaultCellEditor(comboBox);

        Action deleteRow = new AbstractAction("Delete selected row(s)")
            {
                public void actionPerformed(ActionEvent e) 
                {
                    if(isEditing()) 
                    {
                        getCellEditor().stopCellEditing();
                    }
                    
                    for(;;)
                    {
                        int selectedRow = getSelectedRow();
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
        getActionMap().put("delete", deleteRow);
        getInputMap().put(
            KeyStroke.getKeyStroke("DELETE"), "delete");

    }

    public void set(java.util.List names, java.util.Map values)
    {
        //
        // Transform map into vector of vectors
        //
        java.util.Vector vector = new java.util.Vector(names.size());
        java.util.Iterator p = names.iterator();
        while(p.hasNext())
        {
            java.util.Vector row = new java.util.Vector(2);
            String name = (String)p.next();
            
            row.add(name);
            
            Object val = values.get(name);
            if(val == null)
            {
                row.add(_noDefault);
            }
            else
            {
                row.add(val);
            }
            vector.add(row);
        }


        java.util.Vector newRow = new java.util.Vector(2);
        newRow.add("");
        newRow.add(_noDefault);
        vector.add(newRow);

        _model = new DefaultTableModel(vector, _columnNames);
        
        _model.addTableModelListener(new TableModelListener()
            {
                public void tableChanged(TableModelEvent e)
                {
                    Object lastKey = _model.getValueAt(
                        _model.getRowCount() - 1 , 0);
                    if(lastKey != null && !lastKey.equals(""))
                    {
                        _model.addRow(new Object[]{"", _noDefault});
                    }
                    
                    _editor.updated();
                }
            });
        setModel(_model);

        TableColumn valColumn = getColumnModel().getColumn(1);
        valColumn.setCellEditor(_cellEditor);

        setPreferredScrollableViewportSize(getPreferredSize()); 
    }


    public java.util.Map get(java.util.List names)
    {
        assert names != null;

        java.util.Map values = new java.util.HashMap();

        if(isEditing()) 
        {
            getCellEditor().stopCellEditing();
        }
        java.util.Vector vector = _model.getDataVector();
        
        java.util.Iterator p = vector.iterator();
        while(p.hasNext())
        {
            java.util.Vector row = (java.util.Vector)p.next();
            
            //
            // Eliminate rows with null or empty names
            //
            String name = (String)row.elementAt(0);
            if(name != null)
            {
                name = name.trim();
                
                if(!name.equals(""))
                {
                    names.add(name);
                     
                    Object val = row.elementAt(1);
                    
                    //
                    // Eliminate entries with "default" value
                    //
                    if(val != _noDefault)
                    {
                        assert val != null;
                        values.put(name, val);
                    }
                }
            }
        }
        return values;
    }

    private final Object _noDefault = new Object()
        {
            public String toString()
            {
                return "No default";
            }
        };

    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
    private Editor _editor;
    private TableCellEditor _cellEditor;
}


