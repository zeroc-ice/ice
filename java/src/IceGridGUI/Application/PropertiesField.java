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
import javax.swing.JTable;
import javax.swing.JTextField;
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
        _columnNames = new java.util.Vector(2);
        _columnNames.add("Name");
        _columnNames.add("Value");

        _editor = editor;

        Action deleteRow = new AbstractAction("Delete selected row(s)")
            {
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
        getInputMap().put(
            KeyStroke.getKeyStroke("DELETE"), "delete");


        

    }


    public void setProperties(java.util.List properties,
                              java.util.List adapters,
                              String[] logs,
                              Utils.Resolver resolver, boolean editable)
    {
        _editable = editable;

        //
        // We don't show the .Endpoint and .PublishedEndpoints of adapters,
        // since they already appear in the Adapter pages 
        //
        java.util.Set hiddenPropertyNames = new java.util.HashSet();

        //
        // We also hide properties whose value match an object or allocatable
        //
        java.util.Set hiddenPropertyValues = new java.util.HashSet();

        _hiddenProperties.clear();

        if(adapters != null)
        {
            //
            // Note that we don't substitute *on purpose*, i.e. the names or values
            // must match before substitution.
            //
            java.util.Iterator p = adapters.iterator();
            while(p.hasNext())
            {
                AdapterDescriptor ad = (AdapterDescriptor)p.next();
                hiddenPropertyNames.add(ad.name + ".Endpoints");
                hiddenPropertyNames.add(ad.name + ".PublishedEndpoints");

                java.util.Iterator q = ad.objects.iterator();
                while(q.hasNext())
                {
                    ObjectDescriptor od = (ObjectDescriptor)q.next();
                    hiddenPropertyValues.add(Ice.Util.identityToString(od.id));
                }
                q = ad.allocatables.iterator();
                while(q.hasNext())
                {
                    ObjectDescriptor od = (ObjectDescriptor)q.next();
                    hiddenPropertyValues.add(Ice.Util.identityToString(od.id));
                }
            }
        }

        if(logs != null)
        {
            for(int i = 0; i < logs.length; ++i)
            {
                hiddenPropertyValues.add(logs[i]);
            }
        }
        
        //
        // Transform list into vector of vectors
        //
        java.util.Vector vector = new java.util.Vector(properties.size());
        java.util.Iterator p = properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(hiddenPropertyNames.contains(pd.name))
            {
                //
                // We keep them at the top of the list
                //
                if(_editable)
                {
                    _hiddenProperties.add(pd);
                }

                //
                // We hide only the first occurence
                //
                hiddenPropertyNames.remove(pd.name);
            }
            else if(hiddenPropertyValues.contains(pd.value))
            {
                //
                // We keep them at the top of the list
                //
                if(_editable)
                {
                    _hiddenProperties.add(pd);
                }

                //
                // We hide only the first occurence
                //
                hiddenPropertyValues.remove(pd.value);
            }
            else
            {
                java.util.Vector row = new java.util.Vector(2);
                row.add(Utils.substitute(pd.name, resolver));
                row.add(Utils.substitute(pd.value, resolver));
                vector.add(row);
            }
        }

        if(_editable)
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
                    return _editable;
                }
            };
        
        _model.addTableModelListener(new TableModelListener()
            {
                public void tableChanged(TableModelEvent e)
                {
                    if(_editable)
                    {
                        Object lastKey = _model.getValueAt(
                            _model.getRowCount() - 1 , 0);
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

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)
            getDefaultRenderer(String.class);
        cr.setOpaque(_editable);        
    }

    public java.util.LinkedList getProperties()
    {
        assert _editable;

        if(isEditing()) 
        {
            getCellEditor().stopCellEditing();
        }
        java.util.Vector vector = _model.getDataVector();
        
        java.util.LinkedList result = new java.util.LinkedList(_hiddenProperties);

        java.util.Iterator p = vector.iterator();
        while(p.hasNext())
        {
             java.util.Vector row = (java.util.Vector)p.next();

             //
             // Eliminate rows with null or empty keys
             //
             String key = (String)row.elementAt(0);
             if(key != null)
             {
                 key = key.trim(); 
                 if(!key.equals(""))
                 {
                     String val = (String) row.elementAt(1);
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
    private java.util.Vector _columnNames;
    private boolean _editable = false;

    private java.util.LinkedList _hiddenProperties = new java.util.LinkedList();

    private Editor _editor;
}


