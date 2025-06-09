// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.CommunicatorDescriptor;
import com.zeroc.IceGridGUI.Utils;

import java.awt.event.ActionEvent;
import java.util.SortedMap;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

class CommunicatorEditor extends Editor {
    protected CommunicatorEditor() {
        _description.setEditable(false);
        _description.setOpaque(false);

        _buildId.setEditable(false);

        Action refresh =
            new AbstractAction("Refresh") {
                @Override
                public void actionPerformed(ActionEvent e) {
                    _buildId.setText("");
                    _runtimeProperties.clear();
                    _target.showRuntimeProperties();
                }
            };
        refresh.putValue(Action.SHORT_DESCRIPTION, "Retrieve again the properties");
        _refreshButton = new JButton(refresh);
    }

    protected void show(
            CommunicatorDescriptor descriptor,
            SortedMap<String, String> properties,
            Utils.Resolver resolver) {
        _description.setText(resolver.substitute(descriptor.description));
        _descriptorProperties.setSortedMap(properties);
    }

    protected void clearRuntimeProperties(String buildId) {
        _buildId.setText(buildId);
        _runtimeProperties.clear();
        _runtimePropertiesRetrieved = false;
        _refreshButton.setEnabled(false);
    }

    protected void showRuntimeProperties(Communicator previous) {
        if (_target != previous || !_runtimePropertiesRetrieved) {
            _buildId.setText("Retrieving...");
            _runtimeProperties.clear();

            // Retrieve all properties in background
            _target.showRuntimeProperties();
            _runtimePropertiesRetrieved = true; // set to true immediately to avoid 'spinning'
        }

        _refreshButton.setEnabled(true);
    }

    void setRuntimeProperties(SortedMap<String, String> map, Communicator communicator) {
        if (communicator == _target) {
            _runtimeProperties.setSortedMap(map);
            _runtimePropertiesRetrieved = true;

            String buildString = map.get("BuildId");
            if (buildString == null) {
                _buildId.setText("");
            } else {
                _buildId.setText(buildString);
            }
        }
        // Otherwise we've already moved on
    }

    protected void appendDescriptorProperties(DefaultFormBuilder builder) {
        builder.append("Description");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_description);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
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

    protected void appendRuntimeProperties(DefaultFormBuilder builder) {
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

    private final JTextField _buildId = new JTextField(20);
    private final JButton _refreshButton;
    private final TableField _runtimeProperties = new TableField("Name", "Value");
    private boolean _runtimePropertiesRetrieved;

    private final JTextArea _description = new JTextArea(3, 20);
    private final TableField _descriptorProperties = new TableField("Name", "Value");
}
