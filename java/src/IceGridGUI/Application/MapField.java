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
// A special field used to show/edit a map 
//

public class MapField extends JTable
{
    public MapField(Editor editor, String headKey, String[] headValues, boolean substituteKey)
    {
        _editor = editor;
        _substituteKey = substituteKey;
        _vectorSize = headValues.length + 1;

        _columnNames = new java.util.Vector(_vectorSize);
        _columnNames.add(headKey);
        for(int i = 0; i < headValues.length; ++i)
        {
            _columnNames.add(headValues[i]);
        }

        assert _vectorSize >= 2;
        
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

    public MapField(Editor editor, String headKey, String headValue, boolean substituteKey)
    {
        this(editor, headKey, new String[]{headValue}, substituteKey);
    }

    public void set(java.util.Map map, Utils.Resolver resolver, 
                    boolean editable)
    {
        _editable = editable;

        //
        // Transform map into vector of vectors
        //
        java.util.Vector vector = new java.util.Vector(map.size());
        java.util.Iterator p = map.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Vector row = new java.util.Vector(_vectorSize);
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            
            if(_substituteKey)
            {
                row.add(Utils.substitute((String)entry.getKey(), resolver));
            }
            else
            {
                row.add((String)entry.getKey());
            }

            if(_vectorSize == 2)
            {
                row.add(Utils.substitute((String)entry.getValue(), resolver));
            }
            else
            {
                String[] val = (String[])entry.getValue();

                for(int i = 0; i < val.length; ++i)
                {
                    row.add(Utils.substitute(val[i], resolver));
                }
            }
            vector.add(row);
        }

        if(_editable)
        {
            java.util.Vector newRow = new java.util.Vector(_vectorSize);
            for(int i = 0; i < _vectorSize; ++i)
            {
                newRow.add("");
            }
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
                            Object[] emptyRow = new Object[_vectorSize];
                            for(int i = 0; i < _vectorSize; ++i)
                            {
                                emptyRow[i] = "";
                            }
                            _model.addRow(emptyRow);
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


    public java.util.TreeMap get()
    {
        assert _editable;

        if(isEditing()) 
        {
            getCellEditor().stopCellEditing();
        }
        java.util.Vector vector = _model.getDataVector();
        
        java.util.TreeMap result = new java.util.TreeMap();

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
                     if(_vectorSize == 2)
                     {
                         String val = (String)row.elementAt(1);
                         if(val == null)
                         {
                             val = "";
                         }
                         result.put(key, val);
                     }
                     else
                     {
                         String[] val = new String[_vectorSize - 1];
                         for(int i = 1; i < _vectorSize; ++i)
                         {
                             val[i - 1] = (String)row.elementAt(i);
                             if(val[i - 1] == null)
                             {
                                 val[i - 1] = "";
                             }
                         }
                         result.put(key, val);
                     }
                 }
             }
        }
        return result;
    }

    private final int _vectorSize;

    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
    private boolean _editable = false;

    private boolean _substituteKey;

    private Editor _editor;
}


