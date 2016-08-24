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
import javax.swing.JCheckBox;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class ServiceEditor extends CommunicatorEditor
{
    @Override
    public JToolBar getToolBar()
    {
        if(_toolBar == null)
        {
            _toolBar = new ToolBar();
        }
        return _toolBar;
    }

    ServiceEditor(Coordinator coordinator)
    {
        _coordinator = coordinator;
        _entry.setEditable(false);
        _started.setEnabled(false);

        _buildId.setEditable(false);

        Action refresh = new AbstractAction("Refresh")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _buildId.setText("Retrieving...");
                    _properties.clear();
                    _target.showRuntimeProperties();
                }
            };
        refresh.putValue(Action.SHORT_DESCRIPTION, "Reread the properties from the service");
        _refreshButton = new JButton(refresh);
    }

    void show(Service service)
    {
        _target = service;

        ServiceDescriptor descriptor = service.getServiceDescriptor();
        Utils.Resolver resolver = service.getResolver();

        show(descriptor, service.getProperties(), resolver);
        _entry.setText(resolver.substitute(descriptor.entry));
        _started.setSelected(service.isStarted());

        Server server = (Server)service.getParent();

        int iceIntVersion = server.getIceVersion();

        if(server.getState() == ServerState.Active && (iceIntVersion == 0 || iceIntVersion >= 30300))
        {
            _buildId.setText("Retrieving...");
            _properties.clear();

            //
            // Retrieve all properties in background
            //
            _target.showRuntimeProperties();
            _refreshButton.setEnabled(true);
        }
        else
        {
            _buildId.setText("");
            _properties.clear();
            _refreshButton.setEnabled(false);
        }
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.appendSeparator("Runtime Status");

        builder.append("", _started);
        builder.nextLine();

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
        JScrollPane scrollPane = new JScrollPane(_properties);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.appendSeparator("Configuration");

        super.appendProperties(builder);

        builder.append("Entry Point");
        builder.append(_entry, 3);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
    }

    void setBuildId(String buildString, Service service)
    {
        //
        // That's to report error messages
        //

        if(service == _target)
        {
            _buildId.setText(buildString);
        }
        //
        // Otherwise we've already moved to another server
        //
    }

    void setRuntimeProperties(java.util.SortedMap<String, String> map, Service service)
    {
        if(service == _target )
        {
            _properties.setSortedMap(map);

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
        // Otherwise we've already moved to another server
        //
    }

    private class ToolBar extends JToolBar
    {
        private ToolBar()
        {
            putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.SINGLE);
            putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
            setFloatable(false);
            putClientProperty("JToolBar.isRollover", Boolean.TRUE);

            LiveActions la = _coordinator.getLiveActionsForMenu();

            add(la.get(TreeNode.START));
            add(la.get(TreeNode.STOP));
        }
    }

    private final Coordinator _coordinator;
    private Service _target;
    private JTextField _entry = new JTextField(20);
    private JCheckBox _started = new JCheckBox("Started");
    private JTextField _buildId = new JTextField(20);
    private JButton _refreshButton;
    private TableField _properties = new TableField("Name", "Value");
    private JToolBar _toolBar;
}
