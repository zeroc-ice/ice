// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import IceGrid.*;
import IceGridGUI.*;

import javax.swing.JTable;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

//
// A special field used to show a map or list
//

public class TableField extends JTable
{
    public TableField(String... columns)
    {
        _columnNames = new java.util.Vector<String>(columns.length);
        for(String name : columns)
        {
            _columnNames.add(name);
        }
        
        //
        // Adjust row height for larger fonts
        //
        int fontSize = getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if(rowHeight < minRowHeight)
        {
            setRowHeight(minRowHeight);
        }

        init();
    }

    private void init()
    {
        _model = new DefaultTableModel()
            {
                @Override
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

    public void setProperties(java.util.List<PropertyDescriptor> properties, Utils.Resolver resolver)
    {
        java.util.SortedMap<String, String> map = new java.util.TreeMap<String, String>();
        for(PropertyDescriptor p : properties)
        {
            map.put(resolver.substitute(p.name), resolver.substitute(p.value));
        }
        setSortedMap(map);
    }

    public void setObjects(java.util.List<ObjectDescriptor> objects, Utils.Resolver resolver)
    {
        java.util.SortedMap<String, String> map = new java.util.TreeMap<String, String>();
        for(ObjectDescriptor p : objects)
        {
            Ice.Identity id = new Ice.Identity( resolver.substitute(p.id.name), resolver.substitute(p.id.category));
            map.put(Ice.Util.identityToString(id), resolver.substitute(p.type));
        }
        setSortedMap(map);
    }

    public void setObjects(java.util.SortedMap<String, ObjectInfo> objects)
    {
        java.util.SortedMap<String, String> map = new java.util.TreeMap<String, String>();
        for(ObjectInfo p : objects.values())
        {
            map.put(p.proxy.toString(), p.type);
        }
        setSortedMap(map);
    }

    public void setEnvs(java.util.List<String> envs, Utils.Resolver resolver)
    {
        java.util.SortedMap<String, String> map = new java.util.TreeMap<String, String>();

        for(String p : envs)
        {
            String env = resolver.substitute(p);

            int equal = env.indexOf('=');
            if(equal == -1 || equal == env.length() - 1)
            {
                map.put(env, "");
            }
            else
            {
                map.put(env.substring(0, equal), env.substring(equal + 1));
            }
        }
        setSortedMap(map);
    }

    public void setAdapters(java.util.SortedMap<String, AdapterInfo> adapters)
    {
        java.util.Vector<java.util.Vector<String>> vector =
            new java.util.Vector<java.util.Vector<String>>(adapters.size());
        for(java.util.Map.Entry<String, AdapterInfo> p : adapters.entrySet())
        {
            java.util.Vector<String> row = new java.util.Vector<String>(3);
            row.add(p.getKey());

            AdapterInfo ai = p.getValue();

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

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    public void setSortedMap(java.util.SortedMap<String, String> map)
    {
        java.util.Vector<java.util.Vector<String>> vector = new java.util.Vector<java.util.Vector<String>>(map.size());
        for(java.util.Map.Entry<String, String> p : map.entrySet())
        {
            java.util.Vector<String> row = new java.util.Vector<String>(2);
            row.add(p.getKey());
            row.add(p.getValue());
            vector.add(row);
        }

        _model.setDataVector(vector, _columnNames);

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    public void clear()
    {
        _model.setDataVector(new java.util.Vector<java.util.Vector<String>>(), _columnNames);
        DefaultTableCellRenderer cr = (DefaultTableCellRenderer)getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    private DefaultTableModel _model;
    private java.util.Vector<String> _columnNames;
}
