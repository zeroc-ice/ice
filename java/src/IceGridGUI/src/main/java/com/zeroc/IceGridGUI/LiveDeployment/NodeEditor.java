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
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;

class NodeEditor extends CommunicatorEditor
{
    NodeEditor()
    {
        _hostname.setEditable(false);
        _os.setEditable(false);
        _machineType.setEditable(false);
        _loadAverage.setEditable(false);

        Action refreshLoad = new AbstractAction("Refresh")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _loadAverage.setText("");
                    _loadRetrieved = false;
                    ((Node)_target).showLoad();
                }
            };
        refreshLoad.putValue(Action.SHORT_DESCRIPTION,
                        "Fetch the latest values from this IceGrid Node");
        _refreshLoadButton = new JButton(refreshLoad);
    }

    void show(Node node)
    {
        Node previous = (Node)_target;
        _target = node;

        NodeInfo info = node.getStaticInfo();

        if(info == null)
        {
            _hostname.setText("Unknown");
            _os.setText("Unknown");
            _machineType.setText("Unknown");
            _loadAverageLabel.setText("Load Average");
            _loadAverage.setText("Unknown");
            _loadRetrieved = false;
            _refreshLoadButton.setEnabled(false);
            clearRuntimeProperties("Unknown");
        }
        else
        {
            _hostname.setText(info.hostname);
            _os.setText(info.os + " " + info.release + " " + info.version);
            _os.setCaretPosition(0);
            _machineType.setText(info.machine + " with " +
                                 info.nProcessors
                                 + " CPU thread"
                                 + (info.nProcessors >= 2 ? "s" : ""));

            if(node.isRunningWindows())
            {
                _loadAverageLabel.setText("CPU Usage");
                _loadAverage.setToolTipText("CPU usage in the past 1 min, 5 min and 15 min period");
            }
            else
            {
                _loadAverageLabel.setText("Load Average");
                _loadAverage.setToolTipText("Load average in the past 1 min, 5 min and 15 min period");
            }

            if(_target != previous || !_loadRetrieved)
            {
                _loadAverage.setText("Refreshing load...");
                _loadRetrieved = true;
                node.showLoad();
                _refreshLoadButton.setEnabled(true);
            }

            showRuntimeProperties(previous);
        }

        _loadFactor.setSortedMap(node.getLoadFactors());
    }

    void setLoad(String load, Node node)
    {
        if(node == _target)
        {
            _loadAverage.setText(load);
            _loadRetrieved = true;
        }
        //
        // Otherwise, we've already moved to another node
        //
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.appendSeparator("System Information");

        builder.append("Hostname");
        builder.append(_hostname, 3);
        builder.nextLine();
        builder.append("Operating System");
        builder.append(_os, 3);

        builder.nextLine();
        builder.append("Machine Type");
        builder.append(_machineType, 3);
        builder.append(_loadAverageLabel, _loadAverage);
        builder.append(_refreshLoadButton);
        builder.nextLine();
        appendRuntimeProperties(builder);

        builder.appendSeparator("Configuration");
        builder.append("Load Factor");
        builder.nextLine();

        builder.append("");
        builder.nextLine();

        builder.append("");
        builder.nextLine();

        builder.append("");
        builder.nextRow(-6);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_loadFactor);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Node Properties");
    }

    private JTextField _hostname = new JTextField(20);
    private JTextField _os = new JTextField(20);
    private JTextField _machineType = new JTextField(20);
    private JLabel _loadAverageLabel = new JLabel();
    private JTextField _loadAverage = new JTextField(20);
    private JButton _refreshLoadButton;
    private boolean _loadRetrieved = false;

    private TableField _loadFactor = new TableField("Application", "Value");
}
