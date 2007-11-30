// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.JCheckBox;
import javax.swing.JTextField;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import javax.swing.JToolBar;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;


import IceGrid.*;
import IceGridGUI.*;

class ServiceEditor extends CommunicatorEditor
{
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
    }

    void show(Service service)
    {
        ServiceDescriptor descriptor = service.getServiceDescriptor();
        Utils.Resolver resolver = service.getResolver();

        show(descriptor, service.getProperties(), resolver);
        _entry.setText(resolver.substitute(descriptor.entry));
        _started.setSelected(service.isStarted());
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.appendSeparator("Runtime Status");

        builder.append("", _started);
        builder.nextLine();

        builder.appendSeparator("Configuration");

        super.appendProperties(builder);

        builder.append("Entry Point");
        builder.append(_entry, 3);
        builder.nextLine();
    }

    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
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
    private JTextField _entry = new JTextField(20);
    private JCheckBox _started = new JCheckBox("Started");
    private JToolBar _toolBar;
}

