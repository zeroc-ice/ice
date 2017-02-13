// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JLabel;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;

class NodeEditor extends Editor
{
    NodeEditor()
    {
        _hostname.setEditable(false);
        _os.setEditable(false);
        _machineType.setEditable(false);
        _loadAverage.setEditable(false);

        Action refresh = new AbstractAction("Refresh")
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    _target.showLoad();
                }
            };
        refresh.putValue(Action.SHORT_DESCRIPTION,
                        "Fetch the latest values from this IceGrid Node");
        _refreshButton = new JButton(refresh);
    }

    void show(Node node)
    {
        _target = node;

        NodeInfo info = node.getStaticInfo();

        if(info == null)
        {
            _hostname.setText("Unknown");
            _os.setText("Unknown");
            _machineType.setText("Unknown");
            _loadAverageLabel.setText("Load Average");
            _loadAverage.setText("Unknown");
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
            _loadAverage.setText("Refreshing...");
            node.showLoad();
        }

        _loadFactor.setSortedMap(node.getLoadFactors());
    }

    void setLoad(String load, Node node)
    {
        if(node == _target)
        {
            _loadAverage.setText(load);
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
        builder.append(_refreshButton);
        builder.nextLine();

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
    private JButton _refreshButton;

    private TableField _loadFactor = new TableField("Application", "Value");

    private Node _target;
}
