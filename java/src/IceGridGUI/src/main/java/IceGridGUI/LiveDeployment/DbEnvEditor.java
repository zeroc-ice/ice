// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

class DbEnvEditor extends Editor
{
    DbEnvEditor()
    {
        _description.setEditable(false);
        _description.setOpaque(false);
        _dbHome.setEditable(false);
    }

    void show(DbEnv dbEnv)
    {
        DbEnvDescriptor descriptor = dbEnv.getDescriptor();
        Utils.Resolver resolver = dbEnv.getResolver();
        _description.setText(resolver.substitute(descriptor.description));

        if(descriptor.dbHome.length() == 0)
        {
            _dbHome.setText("Created by the IceGrid Node");
        }
        else
        {
            _dbHome.setText(resolver.substitute(descriptor.dbHome));
        }

        _properties.setProperties(descriptor.properties, resolver);
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
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
        builder.nextRow(2);
        builder.nextLine();

        builder.append("DB Home" );
        builder.append(_dbHome, 3);
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

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Database Environment Properties");
    }

    private JTextArea _description = new JTextArea(3, 20);
    private JTextField _dbHome = new JTextField(20);
    private TableField _properties = new TableField("Name", "Value");
}
