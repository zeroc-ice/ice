// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import com.zeroc.IceGrid.*;

class SlaveEditor extends CommunicatorEditor
{
    SlaveEditor()
    {
        _hostname.setEditable(false);
    }

    void show(Slave slave)
    {
        Slave previous = (Slave)_target;
        _target = slave;

        _hostname.setText(slave.getInfo().hostname);
        showRuntimeProperties(previous);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Hostname" );
        builder.append(_hostname, 3);
        builder.nextLine();
        appendRuntimeProperties(builder);
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Slave Registry Properties");
    }

    private JTextField _hostname = new JTextField(20);
}
