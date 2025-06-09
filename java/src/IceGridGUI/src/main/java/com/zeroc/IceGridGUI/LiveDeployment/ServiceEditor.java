// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.zeroc.IceGrid.ServerState;
import com.zeroc.IceGrid.ServiceDescriptor;
import com.zeroc.IceGridGUI.Coordinator;
import com.zeroc.IceGridGUI.LiveActions;
import com.zeroc.IceGridGUI.Utils;

import javax.swing.JCheckBox;
import javax.swing.JTextField;
import javax.swing.JToolBar;

class ServiceEditor extends CommunicatorEditor {
    @Override
    public JToolBar getToolBar() {
        if (_toolBar == null) {
            _toolBar = new ToolBar();
        }
        return _toolBar;
    }

    ServiceEditor(Coordinator coordinator) {
        _coordinator = coordinator;
        _entry.setEditable(false);
        _started.setEnabled(false);
    }

    void show(Service service) {
        Service previous = (Service) _target;
        _target = service;

        ServiceDescriptor descriptor = service.getServiceDescriptor();
        Utils.Resolver resolver = service.getResolver();

        show(descriptor, service.getProperties(), resolver);
        _entry.setText(resolver.substitute(descriptor.entry));
        _started.setSelected(service.isStarted());

        Server server = (Server) service.getParent();
        int iceIntVersion = server.getIceVersion();

        if (server.getState() == ServerState.Active
            && (iceIntVersion == 0 || iceIntVersion >= 30300)) {
            showRuntimeProperties(previous);
        } else {
            clearRuntimeProperties("");
        }
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder) {
        builder.appendSeparator("Runtime Status");

        builder.append("", _started);
        builder.nextLine();
        appendRuntimeProperties(builder);

        builder.appendSeparator("Configuration");
        appendDescriptorProperties(builder);
        builder.append("Entry Point");
        builder.append(_entry, 3);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel() {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
    }

    private class ToolBar extends JToolBar {
        private ToolBar() {
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
    private final JTextField _entry = new JTextField(20);
    private final JCheckBox _started = new JCheckBox("Started");
    private JToolBar _toolBar;
}
