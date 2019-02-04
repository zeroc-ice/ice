//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.Application;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JTable;
import javax.swing.KeyStroke;
import javax.swing.event.TableModelEvent;
import javax.swing.event.TableModelListener;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

//
// A special field used to show/edit properties
//

public class PropertiesField extends JTable
{
    public PropertiesField(Editor editor)
    {
        _columnNames = new java.util.Vector<>(2);
        _columnNames.add("Name");
        _columnNames.add("Value");

        _editor = editor;

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
                    if(_editable)
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
                }
            };
        getActionMap().put("delete", deleteRow);
        getInputMap().put(KeyStroke.getKeyStroke("DELETE"), "delete");
    }

    public void setProperties(java.util.List<PropertyDescriptor> properties,
                              java.util.List<AdapterDescriptor> adapters, String[] logs, Utils.Resolver resolver,
                              boolean editable)
    {
        _editable = editable;

        //
        // We don't show the .Endpoint and .PublishedEndpoints of adapters,
        // since they already appear in the Adapter pages
        //
        java.util.Set<String> hiddenPropertyNames = new java.util.HashSet<>();

        //
        // We also hide properties whose value match an object or allocatable
        //
        java.util.Set<String> hiddenPropertyValues = new java.util.HashSet<>();

        _hiddenProperties.clear();

        if(adapters != null)
        {
            //
            // Note that we don't substitute *on purpose*, i.e. the names or values
            // must match before substitution.
            //
            for(AdapterDescriptor p : adapters)
            {
                hiddenPropertyNames.add(p.name + ".Endpoints");
                hiddenPropertyNames.add(p.name + ".PublishedEndpoints");
                hiddenPropertyNames.add(p.name + ".ProxyOptions");

                for(ObjectDescriptor q : p.objects)
                {
                    hiddenPropertyValues.add(com.zeroc.Ice.Util.identityToString(q.id, com.zeroc.Ice.ToStringMode.Unicode));
                }
                for(ObjectDescriptor q : p.allocatables)
                {
                    hiddenPropertyValues.add(com.zeroc.Ice.Util.identityToString(q.id, com.zeroc.Ice.ToStringMode.Unicode));
                }
            }
        }

        if(logs != null)
        {
            for(String log : logs)
            {
                hiddenPropertyValues.add(log);
            }
        }

        //
        // Transform list into vector of vectors
        //
        java.util.Vector<java.util.Vector<String>> vector = new java.util.Vector<>(properties.size());
        for(PropertyDescriptor p : properties)
        {
            if(hiddenPropertyNames.contains(p.name))
            {
                //
                // We keep them at the top of the list
                //
                if(_editable)
                {
                    _hiddenProperties.add(p);
                }

                //
                // We hide only the first occurence
                //
                hiddenPropertyNames.remove(p.name);
            }
            else if(hiddenPropertyValues.contains(p.value))
            {
                //
                // We keep them at the top of the list
                //
                if(_editable)
                {
                    _hiddenProperties.add(p);
                }

                //
                // We hide only the first occurence
                //
                hiddenPropertyValues.remove(p.value);
            }
            else
            {
                java.util.Vector<String> row = new java.util.Vector<>(2);
                row.add(Utils.substitute(p.name, resolver));
                row.add(Utils.substitute(p.value, resolver));
                vector.add(row);
            }
        }

        if(_editable)
        {
            java.util.Vector<String> newRow = new java.util.Vector<>(2);
            newRow.add("");
            newRow.add("");
            vector.add(newRow);
        }

        _model = new DefaultTableModel(vector, _columnNames)
            {
                @Override
                public boolean isCellEditable(int row, int column)
                {
                    return _editable;
                }
            };

        _model.addTableModelListener(new TableModelListener()
            {
                @Override
                public void tableChanged(TableModelEvent e)
                {
                    if(_editable)
                    {
                        Object lastKey = _model.getValueAt(_model.getRowCount() - 1 , 0);
                        if(lastKey != null && !lastKey.equals(""))
                        {
                            _model.addRow(new Object[]{"", ""});
                        }
                        _editor.updated();
                    }
                }
            });
        setModel(_model);

        setCellSelectionEnabled(_editable);
        setOpaque(_editable);
        setPreferredScrollableViewportSize(getPreferredSize());

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)getDefaultRenderer(String.class);
        cr.setOpaque(_editable);
    }

    public java.util.LinkedList<PropertyDescriptor> getProperties()
    {
        assert _editable;

        if(isEditing())
        {
            getCellEditor().stopCellEditing();
        }
        @SuppressWarnings("unchecked")
        java.util.Vector<java.util.Vector> vector = _model.getDataVector();

        java.util.LinkedList<PropertyDescriptor> result = new java.util.LinkedList<>(_hiddenProperties);

        for(java.util.Vector row : vector)
        {
            //
            // Eliminate rows with null or empty keys
            //
            String key = row.elementAt(0).toString();
            if(key != null)
            {
                key = key.trim();
                if(!key.equals(""))
                {
                    String val = row.elementAt(1).toString();
                    if(val == null)
                    {
                        val = "";
                    }

                    result.add(new PropertyDescriptor(key, val));
                }
            }
        }
        return result;
    }

    private DefaultTableModel _model;
    private java.util.Vector<String> _columnNames;
    private boolean _editable = false;

    private java.util.LinkedList<PropertyDescriptor> _hiddenProperties = new java.util.LinkedList<>();

    private Editor _editor;
}
