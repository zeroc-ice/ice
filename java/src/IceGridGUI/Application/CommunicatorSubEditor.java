// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class CommunicatorSubEditor
{
    CommunicatorSubEditor(Editor mainEditor)
    {
        _mainEditor = mainEditor;
        
        _description.getDocument().addDocumentListener(
            _mainEditor.getUpdateListener());
        _description.setToolTipText("An optional description");

        _propertySets.getDocument().addDocumentListener(
            _mainEditor.getUpdateListener());
        _propertySets.setToolTipText("Property Set References");

        _properties = new PropertiesField(mainEditor);
        _logFiles = new MapField(mainEditor, "Path", "Property",
                                 true);
        _logFiles.setToolTipText("Log files used by this server or service");
    }

  
    void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Description");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_description);
        builder.add(scrollPane, 
                    cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
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
        builder.add(scrollPane, 
                    cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.append("Log files");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-4);
        scrollPane = new JScrollPane(_logFiles);
        builder.add(scrollPane, 
                    cc.xywh(builder.getColumn(), builder.getRow(), 3, 5));
        builder.nextRow(4);
        builder.nextLine();
    }

    void writeDescriptor(CommunicatorDescriptor descriptor)
    {
        descriptor.propertySet.references = 
            (String[])_propertySets.getList().toArray(new String[0]);
        descriptor.propertySet.properties = _properties.getProperties();
        descriptor.description = _description.getText();

        java.util.TreeMap tm = _logFiles.get();
        java.util.Iterator p = tm.entrySet().iterator();
        descriptor.logs = new String[tm.size()];
        int i = 0;

        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String path = (String)entry.getKey();
            String prop = ((String)entry.getValue()).trim();

            descriptor.logs[i++] = path;
            if(!prop.equals(""))
            {
                setProperty((java.util.LinkedList)descriptor.propertySet.properties, prop, path);
            }
        }
    }

    void show(CommunicatorDescriptor descriptor, boolean isEditable)
    {
        Utils.Resolver detailResolver = _mainEditor.getDetailResolver();
        isEditable = isEditable && (detailResolver == null);

        //
        // Note that we don't substitute in the lookup
        //
        java.util.Map map = new java.util.TreeMap();
        for(int i = 0; i < descriptor.logs.length; ++i)
        {
            String prop = lookupKey(descriptor.propertySet.properties,
                                    descriptor.logs[i]);
            map.put(descriptor.logs[i], prop);
        }
        _logFiles.set(map, detailResolver, isEditable);

        _propertySets.setList(java.util.Arrays.asList(descriptor.propertySet.references),
                              detailResolver);
        _propertySets.setEditable(isEditable);
        _properties.setProperties(descriptor.propertySet.properties,
                                  descriptor.adapters,
                                  descriptor.logs,
                                  detailResolver, isEditable);

        _description.setText(
            Utils.substitute(descriptor.description, detailResolver));
        _description.setEditable(isEditable);
        _description.setOpaque(isEditable);
    }


    //
    // Returns first key matching this value, if there is one
    //
    private String lookupKey(java.util.List props, String value)
    {
        java.util.Iterator p = props.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.value.equals(value))
            {
                return pd.name;
            }
        }
        return "";
    }

    private void setProperty(java.util.LinkedList props, String key, String newValue)
    {
        removeProperty(props, key);
        props.addFirst(new PropertyDescriptor(key, newValue));
    }

    private void removeProperty(java.util.List props, String key)
    {
        java.util.Iterator p = props.iterator();
        while(p.hasNext())
        {
            PropertyDescriptor pd = (PropertyDescriptor)p.next();
            if(pd.name.equals(key))
            {
                p.remove();
            }
        }
    }
    

    protected Editor _mainEditor;
 
    private JTextArea _description = new JTextArea(3, 20);
    private ListTextField _propertySets = new ListTextField(20); 
    private PropertiesField _properties;
    private MapField _logFiles;
}
