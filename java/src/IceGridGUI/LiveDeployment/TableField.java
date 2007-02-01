// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

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
// A special field used to show a map or list
//

public class TableField extends JTable
{
    public TableField(String headKey, String headValue)
    {   
        _columnNames = new java.util.Vector(2);
        _columnNames.add(headKey);
        _columnNames.add(headValue);
        init();
    }

    public TableField(String headKey, String headValue1, String headValue2)
    {   
        _columnNames = new java.util.Vector(3);
        _columnNames.add(headKey);
        _columnNames.add(headValue1);
        _columnNames.add(headValue2);
        init();
    }

    private void init()
    {
        _model = new DefaultTableModel()
            {
                public boolean isCellEditable(int row, int column)
                {
                    return false;
                }
            };
        setModel(_model);

        setCellSelectionEnabled(false);
        setOpaque(false);
        setPreferredScrollableViewportSize(getPreferredSize());
    }

    public void setProperties(java.util.List properties, Utils.Resolver resolver)
    {
        java.util.SortedMap map = new java.util.TreeMap();
        java.util.Iterator p = properties.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            map.put(resolver.substitute(pd.name), resolver.substitute(pd.value));
        }
        setSortedMap(map);
    }

    public void setObjects(java.util.List objects, Utils.Resolver resolver)
    {
        java.util.SortedMap map = new java.util.TreeMap();
        java.util.Iterator p = objects.iterator();
        while(p.hasNext())
        {
            ObjectDescriptor od = (ObjectDescriptor)p.next();
            Ice.Identity id = new Ice.Identity(
                resolver.substitute(od.id.name),
                resolver.substitute(od.id.category));

            map.put(Ice.Util.identityToString(id), resolver.substitute(od.type));
        }
        setSortedMap(map);
    }

    public void setObjects(java.util.SortedMap objects)
    {
        java.util.SortedMap map = new java.util.TreeMap();
        java.util.Iterator p = objects.values().iterator();
        while(p.hasNext())
        {
            ObjectInfo oi = (ObjectInfo)p.next();
            map.put(oi.proxy.toString(), oi.type);
        }
        setSortedMap(map);
    }

    public void setEnvs(java.util.List envs, Utils.Resolver resolver)
    {
        java.util.SortedMap map = new java.util.TreeMap();

        java.util.Iterator p = envs.iterator();
        while(p.hasNext())
        {
            String env = resolver.substitute((String)p.next());
            
            int equal = env.indexOf('=');
            if(equal == -1 || equal == env.length() - 1)
            {
                map.put(env, "");
            }
            else
            {
                map.put(env.substring(0, equal),
                        env.substring(equal + 1));
            }
        }
        setSortedMap(map);
    }

    public void setAdapters(java.util.SortedMap adapters)
    {
        java.util.Vector vector = new java.util.Vector(adapters.size());
        java.util.Iterator p = adapters.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Vector row = new java.util.Vector(3);
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            row.add((String)entry.getKey());

            AdapterInfo ai = (AdapterInfo)entry.getValue();
            
            if(ai.proxy == null)
            {
                row.add("");
            }
            else
            {
                String str = ai.proxy.toString();
                int index = str.indexOf(':');
                if(index == -1 || index == str.length() - 1)
                {
                    row.add("");
                }
                else
                {
                    row.add(str.substring(index + 1));
                }
            }

            row.add(ai.replicaGroupId);
            vector.add(row);
        }

        _model.setDataVector(vector, _columnNames);

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)
            getDefaultRenderer(String.class);
        cr.setOpaque(false);    
    }


    public void setSortedMap(java.util.SortedMap map)
    {
        java.util.Vector vector = new java.util.Vector(map.size());
        java.util.Iterator p = map.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Vector row = new java.util.Vector(2);
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            row.add((String)entry.getKey());
            row.add((String)entry.getValue());
            vector.add(row);
        }

        _model.setDataVector(vector, _columnNames);

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)
            getDefaultRenderer(String.class);
        cr.setOpaque(false);    
    }

    private DefaultTableModel _model;
    private java.util.Vector _columnNames;
}


