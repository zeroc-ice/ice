// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class CommunicatorEditor extends Editor
{
    protected CommunicatorEditor()
    {
        _description.setEditable(false);
        _description.setOpaque(false);
    }

    protected void show(CommunicatorDescriptor descriptor, java.util.SortedMap<String, String> properties,
                        Utils.Resolver resolver)
    {
        _description.setText(resolver.substitute(descriptor.description));
        _properties.setSortedMap(properties);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
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
    }

    private JTextArea _description = new JTextArea(3, 20);
    private TableField _properties = new TableField("Name", "Value");
}
