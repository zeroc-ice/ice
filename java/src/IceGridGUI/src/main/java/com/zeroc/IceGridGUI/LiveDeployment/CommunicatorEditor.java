// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

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

        _buildId.setEditable(false);

        Action refresh = new AbstractAction("Refresh")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _buildId.setText("");
                    _runtimeProperties.clear();
                    _target.showRuntimeProperties();
                }
            };
        refresh.putValue(Action.SHORT_DESCRIPTION, "Retrieve again the properties");
        _refreshButton = new JButton(refresh);
    }

    protected void show(CommunicatorDescriptor descriptor, java.util.SortedMap<String, String> properties,
                        Utils.Resolver resolver)
    {
        _description.setText(resolver.substitute(descriptor.description));
        _descriptorProperties.setSortedMap(properties);
    }

    protected void clearRuntimeProperties(String buildId)
    {
        _buildId.setText(buildId);
        _runtimeProperties.clear();
        _runtimePropertiesRetrieved = false;
        _refreshButton.setEnabled(false);
    }

    protected void showRuntimeProperties(Communicator previous)
    {
        if(_target != previous || !_runtimePropertiesRetrieved)
        {
            _buildId.setText("Retrieving...");
            _runtimeProperties.clear();

            //
            // Retrieve all properties in background
            //
            _target.showRuntimeProperties();
            _runtimePropertiesRetrieved = true; // set to true immediately to avoid 'spinning'
        }

        _refreshButton.setEnabled(true);
    }


    void setRuntimeProperties(java.util.SortedMap<String, String> map, Communicator communicator)
    {
        if(communicator == _target)
        {
            _runtimeProperties.setSortedMap(map);
            _runtimePropertiesRetrieved = true;

            String buildString = map.get("BuildId");
            if(buildString == null)
            {
                _buildId.setText("");
            }
            else
            {
                _buildId.setText(buildString);
            }
        }
        //
        // Otherwise we've already moved on
        //
    }

    protected void appendDescriptorProperties(DefaultFormBuilder builder)
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
        scrollPane = new JScrollPane(_descriptorProperties);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    protected void appendRuntimeProperties(DefaultFormBuilder builder)
    {
        builder.append("Build Id");
        builder.append(_buildId, _refreshButton);
        builder.nextLine();

        builder.append("Properties");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");

        builder.nextLine();
        builder.append("");

        builder.nextRow(-6);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_runtimeProperties);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    protected Communicator _target;

    private JTextField _buildId = new JTextField(20);
    private JButton _refreshButton;
    private TableField _runtimeProperties = new TableField("Name", "Value");
    private boolean _runtimePropertiesRetrieved = false;

    private JTextArea _description = new JTextArea(3, 20);
    private TableField _descriptorProperties = new TableField("Name", "Value");
}
