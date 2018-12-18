// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.DefaultCellEditor;
import javax.swing.JComboBox;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableModel;
import javax.swing.table.TableCellEditor;
import javax.swing.table.TableColumn;

//
// A special field used to show/edit parameters
//

@SuppressWarnings("unchecked")
public class ParametersField extends JTable
{
    public ParametersField(Editor editor)
    {
        _editor = editor;

        _columnNames = new java.util.Vector<>(2);
        _columnNames.add("Name");
        _columnNames.add("Default value");

        JComboBox comboBox = new JComboBox(new String[]{_noDefault});
        comboBox.setEditable(true);
        _cellEditor = new DefaultCellEditor(comboBox);

        //
        // Adjust row height for larger fonts
        //
        int fontSize = getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if(rowHeight < minRowHeight)
        {
            setRowHeight(minRowHeight);
        }

        Action deleteRow = new AbstractAction("Delete selected row(s)")
            {
                @Override
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
        getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
    }

    public void set(java.util.List<String> names, java.util.Map<String, String> values)
    {
        //
        // Transform map into vector of vectors
        //
        java.util.Vector<java.util.Vector<String>> vector = new java.util.Vector<>(names.size());
        for(String name : names)
        {
            java.util.Vector<String> row = new java.util.Vector<>(2);

            row.add(name);

            String val = values.get(name);
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

        java.util.Vector<String> newRow = new java.util.Vector<>(2);
        newRow.add("");
        newRow.add(_noDefault);
        vector.add(newRow);

        _model = new DefaultTableModel(vector, _columnNames);

        _model.addTableModelListener(new TableModelListener()
            {
                @Override
                public void tableChanged(TableModelEvent e)
                {
                    Object lastKey = _model.getValueAt(_model.getRowCount() - 1 , 0);
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

    public java.util.Map<String, String> get(java.util.List<String> names)
    {
        assert names != null;

        java.util.Map<String, String> values = new java.util.HashMap<>();

        if(isEditing())
        {
            getCellEditor().stopCellEditing();
        }
        @SuppressWarnings("unchecked")
        java.util.Vector<java.util.Vector> vector = _model.getDataVector();

        for(java.util.Vector row : vector)
        {
            //
            // Eliminate rows with null or empty names
            //
            String name = row.elementAt(0).toString();
            if(name != null)
            {
                name = name.trim();

                if(!name.equals(""))
                {
                    names.add(name);

                    String val = row.elementAt(1).toString();

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

    private final String _noDefault = "No default";

    private DefaultTableModel _model;
    private java.util.Vector<String> _columnNames;
    private Editor _editor;
    private TableCellEditor _cellEditor;
}
