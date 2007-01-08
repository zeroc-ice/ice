// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class SlaveEditor extends Editor
{
    SlaveEditor()
    {
	_hostname.setEditable(false);
	_endpoints.setEditable(false);
    }

    void show(RegistryInfo info)
    {
	_hostname.setText(info.hostname);
	_endpoints.setText(info.endpoints);
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
	builder.append("Hostname" );
	builder.append(_hostname, 3);
	builder.nextLine();

	builder.append("Endpoints" );
	builder.append(_endpoints, 3);
	builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Slave Registry Properties");
    }

    private JTextField _hostname = new JTextField(20);
    private JTextField _endpoints = new JTextField(20);
}
