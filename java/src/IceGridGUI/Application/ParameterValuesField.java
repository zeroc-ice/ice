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
// A special field used to show/edit parameter values
//

public class ParameterValuesField extends JTable
{
    public ParameterValuesField(Editor editor)
    {
        _editor = editor;

        _columnNames = new java.util.Vector(2);
        _columnNames.add("Name");
        _columnNames.add("Value");

        _useDefaultCombo.setEditable(true);
        _notSetCombo.setEditable(true);
    }

    public void set(java.util.List names,
                    java.util.Map values,
                    java.util.Map defaultValues,
                    final Utils.Resolver resolver)
    {
        //
        // Transform map into vector of vectors
        //
        java.util.Vector vector = new java.util.Vector(names.size());
        java.util.Iterator p = names.iterator();

        _hasDefault = new boolean[names.size()];
        int i = 0;

        while(p.hasNext())
        {
            java.util.Vector row = new java.util.Vector(2);
            String name = (String)p.next(); 
            row.add(name);

            _hasDefault[i] = (defaultValues.get(name) != null);
           
            Object val = values.get(name);
            if(val == null)
            {
                row.add(_hasDefault[i] ? _useDefault : _notSet);
            }
            else
            {
                row.add(Utils.substitute((String)val, resolver));
            }
            vector.add(row);
            i++;
        }

        _model = new DefaultTableModel(vector, _columnNames)
            {
                public boolean isCellEditable(int row, int column)
                {
                    if(resolver == null)
                    {
                        return column > 0;
                    }
                    else
                    {
                        return false;
                    }
                }
            };
        
        _model.addTableModelListener(new TableModelListener()
            {
                public void tableChanged(TableModelEvent e)
                {
                    _editor.updated();
                }
            });
        setModel(_model);

        setCellSelectionEnabled(resolver == null);
        setOpaque(resolver == null);
        setPreferredScrollableViewportSize(getPreferredSize());

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)
            getDefaultRenderer(String.class);
        cr.setOpaque(resolver == null); 
    }


    public java.util.Map getValues()
    {
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
            assert name != null;
               
            Object val = row.elementAt(1);
                    
            //
            // Eliminate entries with default or not set value
            //
            if(val != _useDefault && val != _notSet)
            {
                assert val != null;
                values.put(name, val);
            }
        }
        return values;
    }

    public TableCellEditor getCellEditor(int row,
                                         int column)
    {
        if(column == 1)
        {
            return _hasDefault[row] ? _useDefaultEditor : _notSetEditor;
        }
        else
        {
            return super.getCellEditor(row, column);
        }
    }


    private static final Object _useDefault = new Object()
        {
            public String toString()
            {
                return "Use default";
            }
        };

    private static final Object _notSet = new Object()
        {
            public String toString()
            {
                return "Not set";
            }
        };

    private JComboBox _useDefaultCombo = new JComboBox(
        new Object[]{_useDefault});
    
    private JComboBox _notSetCombo = new JComboBox(
        new Object[]{_notSet});

    private TableCellEditor _useDefaultEditor = new DefaultCellEditor(_useDefaultCombo);
    private TableCellEditor _notSetEditor = new DefaultCellEditor(_notSetCombo);

    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
    private Editor _editor;

    private boolean[] _hasDefault;
}


