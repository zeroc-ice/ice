// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.CommunicatorDescriptor;
import com.zeroc.IceGrid.PropertyDescriptor;
import com.zeroc.IceGridGUI.Utils;

import java.util.Arrays;
import java.util.Iterator;
import java.util.LinkedList;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;

class CommunicatorSubEditor {
    CommunicatorSubEditor(Editor mainEditor) {
        _mainEditor = mainEditor;

        _description.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _description.setToolTipText("An optional description");

        _propertySets.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _propertySets.setToolTipText("Property Set References");

        _properties = new PropertiesField(mainEditor);
        _logFiles = new SimpleMapField(mainEditor, true, "Path", "Property");
        _logFiles.setToolTipText("Log files used by this server or service");
    }

    @SuppressWarnings("deprecation")
    void appendProperties(DefaultFormBuilder builder) {
        builder.append("Description");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_description);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
        builder.nextRow(2);
        builder.nextLine();

        builder.append("Property Sets");
        builder.append(_propertySets, 3);
        builder.nextLine();

        builder.append("Properties");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");

        builder.nextRow(-6);
        scrollPane = new JScrollPane(_properties);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.append("Log files");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-4);
        scrollPane = new JScrollPane(_logFiles);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 5));
        builder.nextRow(4);
        builder.nextLine();
    }

    void writeDescriptor(CommunicatorDescriptor descriptor) {
        descriptor.propertySet.references = _propertySets.getList().toArray(new String[0]);
        descriptor.propertySet.properties = _properties.getProperties();
        descriptor.description = _description.getText();

        TreeMap<String, String> tm = _logFiles.get();
        descriptor.logs = new String[tm.size()];
        int i = 0;

        for (Map.Entry<String, String> p : tm.entrySet()) {
            String path = p.getKey();
            String prop = p.getValue().trim();

            descriptor.logs[i++] = path;
            if (!prop.isEmpty()) {
                setProperty(
                    (LinkedList<PropertyDescriptor>)
                        descriptor.propertySet.properties,
                    prop,
                    path);
            }
        }
    }

    void show(CommunicatorDescriptor descriptor, boolean isEditable) {
        Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
        isEditable = isEditable && (detailResolver == null);

        // Note that we don't substitute in the lookup
        Map<String, String> map = new TreeMap<>();
        for (String log : descriptor.logs) {
            String prop = lookupKey(descriptor.propertySet.properties, log);
            map.put(log, prop);
        }
        _logFiles.set(map, detailResolver, isEditable);

        _propertySets.setList(
            Arrays.asList(descriptor.propertySet.references), detailResolver);
        _propertySets.setEditable(isEditable);
        _properties.setProperties(
            descriptor.propertySet.properties,
            descriptor.adapters,
            descriptor.logs,
            detailResolver,
            isEditable);

        _description.setText(Utils.substitute(descriptor.description, detailResolver));
        _description.setEditable(isEditable);
        _description.setOpaque(isEditable);
    }

    // Returns first key matching this value, if there is one
    private String lookupKey(List<PropertyDescriptor> props, String value) {
        for (PropertyDescriptor p : props) {
            if (p.value.equals(value)) {
                return p.name;
            }
        }
        return "";
    }

    private void setProperty(
            LinkedList<PropertyDescriptor> props, String key, String newValue) {
        removeProperty(props, key);
        props.addFirst(new PropertyDescriptor(key, newValue));
    }

    private void removeProperty(List<PropertyDescriptor> props, String key) {
        Iterator<PropertyDescriptor> p = props.iterator();
        while (p.hasNext()) {
            PropertyDescriptor pd = p.next();
            if (pd.name.equals(key)) {
                p.remove();
            }
        }
    }

    protected Editor _mainEditor;

    private final JTextArea _description = new JTextArea(3, 20);
    private final ListTextField _propertySets = new ListTextField(20);
    private final PropertiesField _properties;
    private final SimpleMapField _logFiles;
}
