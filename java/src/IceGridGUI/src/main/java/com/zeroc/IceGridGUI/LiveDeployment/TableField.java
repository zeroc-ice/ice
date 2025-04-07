// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ToStringMode;
import com.zeroc.Ice.Util;
import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

import javax.swing.JTable;

import java.util.List;
import java.util.Map;
import java.util.SortedMap;
import java.util.TreeMap;
import java.util.Vector;
import javax.swing.table.DefaultTableCellRenderer;
import javax.swing.table.DefaultTableModel;

/** A special field used to show a map or list */
public class TableField extends JTable {
    public TableField(String... columns) {
        _columnNames = new Vector<>(columns.length);
        for (String name : columns) {
            _columnNames.add(name);
        }

        // Adjust row height for larger fonts
        int fontSize = getFont().getSize();
        int minRowHeight = fontSize + fontSize / 3;
        if (rowHeight < minRowHeight) {
            setRowHeight(minRowHeight);
        }

        init();
    }

    private void init() {
        _model =
                new DefaultTableModel() {
                    @Override
                    public boolean isCellEditable(int row, int column) {
                        return false;
                    }
                };
        setModel(_model);

        setCellSelectionEnabled(false);
        setOpaque(false);
        setPreferredScrollableViewportSize(getPreferredSize());
    }

    public void setProperties(
            List<PropertyDescriptor> properties, Utils.Resolver resolver) {
        SortedMap<String, String> map = new TreeMap<>();
        for (PropertyDescriptor p : properties) {
            map.put(resolver.substitute(p.name), resolver.substitute(p.value));
        }
        setSortedMap(map);
    }

    public void setObjects(List<ObjectDescriptor> objects, Utils.Resolver resolver) {
        SortedMap<String, String> map = new TreeMap<>();
        for (ObjectDescriptor p : objects) {
            Identity id =
                    new Identity(
                            resolver.substitute(p.id.name), resolver.substitute(p.id.category));
            map.put(
                    Util.identityToString(id, ToStringMode.Unicode),
                    resolver.substitute(p.type));
        }
        setSortedMap(map);
    }

    public void setObjects(SortedMap<String, ObjectInfo> objects) {
        SortedMap<String, String> map = new TreeMap<>();
        for (ObjectInfo p : objects.values()) {
            map.put(p.proxy.toString(), p.type);
        }
        setSortedMap(map);
    }

    public void setEnvs(List<String> envs, Utils.Resolver resolver) {
        SortedMap<String, String> map = new TreeMap<>();

        for (String p : envs) {
            String env = resolver.substitute(p);

            int equal = env.indexOf('=');
            if (equal == -1 || equal == env.length() - 1) {
                map.put(env, "");
            } else {
                map.put(env.substring(0, equal), env.substring(equal + 1));
            }
        }
        setSortedMap(map);
    }

    public void setAdapters(SortedMap<String, AdapterInfo> adapters) {
        Vector<Vector<String>> vector = new Vector<>(adapters.size());
        for (Map.Entry<String, AdapterInfo> p : adapters.entrySet()) {
            Vector<String> row = new Vector<>(3);
            row.add(p.getKey());

            AdapterInfo ai = p.getValue();

            if (ai.proxy == null) {
                row.add("");
            } else {
                String str = ai.proxy.toString();
                int index = str.indexOf(':');
                if (index == -1 || index == str.length() - 1) {
                    row.add("");
                } else {
                    row.add(str.substring(index + 1));
                }
            }

            row.add(ai.replicaGroupId);
            vector.add(row);
        }

        _model.setDataVector(vector, _columnNames);

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer) getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    public void setSortedMap(SortedMap<String, String> map) {
        Vector<Vector<String>> vector = new Vector<>(map.size());
        for (Map.Entry<String, String> p : map.entrySet()) {
            Vector<String> row = new Vector<>(2);
            row.add(p.getKey());
            row.add(p.getValue());
            vector.add(row);
        }

        _model.setDataVector(vector, _columnNames);

        DefaultTableCellRenderer cr = (DefaultTableCellRenderer) getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    public void clear() {
        _model.setDataVector(new Vector<>(), _columnNames);
        DefaultTableCellRenderer cr = (DefaultTableCellRenderer) getDefaultRenderer(String.class);
        cr.setOpaque(false);
    }

    private DefaultTableModel _model;
    private final Vector<String> _columnNames;
}
