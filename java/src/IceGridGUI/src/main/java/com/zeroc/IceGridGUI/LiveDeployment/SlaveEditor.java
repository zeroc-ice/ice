// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import javax.swing.JTextField;

class SlaveEditor extends CommunicatorEditor {
    SlaveEditor() {
        _hostname.setEditable(false);
    }

    void show(Slave slave) {
        Slave previous = (Slave) _target;
        _target = slave;

        _hostname.setText(slave.getInfo().hostname);
        showRuntimeProperties(previous);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder) {
        builder.append("Hostname");
        builder.append(_hostname, 3);
        builder.nextLine();
        appendRuntimeProperties(builder);
    }

    @Override
    protected void buildPropertiesPanel() {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Slave Registry Properties");
    }

    private final JTextField _hostname = new JTextField(20);
}
