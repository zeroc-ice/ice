// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import javax.swing.JComboBox;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class DbEnvEditor extends CommunicatorChildEditor
{
    DbEnvEditor()
    {
        _name.getDocument().addDocumentListener(_updateListener);
        _name.setToolTipText("Identifies this Freeze database environment within an Ice communicator");
        _description.getDocument().addDocumentListener(_updateListener);
        _description.setToolTipText("An optional description for this database environment");

        JTextField dbHomeTextField = (JTextField)_dbHome.getEditor().getEditorComponent();
        dbHomeTextField.getDocument().addDocumentListener(_updateListener);
        _dbHome.setToolTipText("<html><i>node data dir</i>/servers/<i>server id</i>"
                               + "/dbs/<i>db env name</i> if created by the IceGrid Node;<br>"
                               + "otherwise, IceGrid does not create this directory"
                               + "</html>");

        _properties = new PropertiesField(this);
    }

    @Override
    void writeDescriptor()
    {
        DbEnvDescriptor descriptor = (DbEnvDescriptor)getDbEnv().getDescriptor();
        descriptor.name = _name.getText().trim();
        descriptor.description = _description.getText();
        descriptor.dbHome = getDbHomeAsString();
        descriptor.properties = _properties.getProperties();
    }

    @Override
    boolean isSimpleUpdate()
    {
        DbEnvDescriptor descriptor = (DbEnvDescriptor)getDbEnv().getDescriptor();
        return descriptor.name.equals(_name.getText().trim());
    }

    @Override
    Communicator.ChildList getChildList()
    {
        return ((Communicator)_target.getParent()).getDbEnvs();
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Name" );
        builder.append(_name, 3);
        builder.nextLine();

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

    @Override
    protected boolean validate()
    {
        return check(new String[]{"Name", _name.getText().trim()});
    }

    void show(DbEnv dbEnv)
    {
        detectUpdates(false);
        _target = dbEnv;

        DbEnvDescriptor descriptor = (DbEnvDescriptor)dbEnv.getDescriptor();

        Utils.Resolver resolver = null;
        if(dbEnv.getCoordinator().substitute())
        {
            resolver = dbEnv.getResolver();
        }
        boolean isEditable = resolver == null;

        _name.setText(Utils.substitute(descriptor.name, resolver));
        _name.setEditable(isEditable);

        _description.setText(Utils.substitute(descriptor.description, resolver));
        _description.setEditable(isEditable);
        _description.setOpaque(isEditable);

        _dbHome.setEnabled(true);
        _dbHome.setEditable(true);
        setDbHome(Utils.substitute(descriptor.dbHome, resolver));
        _dbHome.setEnabled(isEditable);
        _dbHome.setEditable(isEditable);

        _properties.setProperties(descriptor.properties, null, null, resolver, isEditable);

        _applyButton.setEnabled(dbEnv.isEphemeral());
        _discardButton.setEnabled(dbEnv.isEphemeral());
        detectUpdates(true);

        if(dbEnv.isEphemeral())
        {
            updated();
        }
    }

    private DbEnv getDbEnv()
    {
        return (DbEnv)_target;
    }

    private void setDbHome(String dbHome)
    {
        if(dbHome.equals(""))
        {
            _dbHome.setSelectedItem(NO_DB_HOME);
        }
        else
        {
            _dbHome.setSelectedItem(dbHome);
        }
    }

    private String getDbHomeAsString()
    {
        Object obj = _dbHome.getSelectedItem();
        if(obj == NO_DB_HOME)
        {
            return "";
        }
        else
        {
            return obj.toString().trim();
        }
    }

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);

    @SuppressWarnings("unchecked")
    private JComboBox _dbHome = new JComboBox(new Object[]{NO_DB_HOME});
    private PropertiesField _properties;

    static private final Object NO_DB_HOME = new Object()
        {
            @Override
            public String toString()
            {
                return "Created by the IceGrid Node";
            }
        };
}
