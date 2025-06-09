// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.LiveDeployment;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.Options;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.zeroc.IceGrid.ServerDescriptor;
import com.zeroc.IceGrid.ServerState;
import com.zeroc.IceGridGUI.Coordinator;
import com.zeroc.IceGridGUI.LiveActions;
import com.zeroc.IceGridGUI.Utils;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JCheckBox;
import javax.swing.JScrollPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;

class ServerEditor extends CommunicatorEditor {
    @Override
    public JToolBar getToolBar() {
        if (_toolBar == null) {
            _toolBar = new ToolBar();
        }
        return _toolBar;
    }

    ServerEditor(Coordinator c) {
        _coordinator = c;
        _currentState.setEditable(false);
        _enabled.setEnabled(false);
        _currentPid.setEditable(false);
        _application.setEditable(false);
        _exe.setEditable(false);
        _iceVersion.setEditable(false);
        _pwd.setEditable(false);

        _activation.setEditable(false);
        _activationTimeout.setEditable(false);
        _deactivationTimeout.setEditable(false);

        _options.setEditable(false);
        _user.setEditable(false);
        _allocatable.setEnabled(false);

        Action gotoApplication =
            new AbstractAction("", Utils.getIcon("/icons/16x16/goto.png")) {
                @Override
                public void actionPerformed(ActionEvent e) {
                    ((Server) _target).openDefinition();
                }
            };
        gotoApplication.putValue(Action.SHORT_DESCRIPTION, "View/Edit this application");
        _gotoApplication = new JButton(gotoApplication);
    }

    void show(Server server) {
        Server previousServer = (Server) _target;
        _target = server;

        ServerState state = server.getState();
        ServerDescriptor descriptor = server.getServerDescriptor();
        final Utils.Resolver resolver = server.getResolver();

        if (state == null) {
            _currentState.setText("Unknown");
            _enabled.setSelected(false);
            _currentPid.setText("");
            clearRuntimeProperties("Unknown");
        } else {
            _currentState.setText(state.toString());
            _enabled.setSelected(server.isEnabled());

            int pid = server.getPid();
            if (pid == 0) {
                _currentPid.setText("");
            } else {
                _currentPid.setText(Integer.toString(pid));
            }

            int iceIntVersion = server.getIceVersion();
            if (state == ServerState.Active && (iceIntVersion == 0 || iceIntVersion >= 30300)) {
                showRuntimeProperties(previousServer);
            } else {
                clearRuntimeProperties("");
            }
        }

        _application.setText(resolver.find("application"));

        super.show(descriptor, server.getProperties(), resolver);

        _exe.setText(resolver.substitute(descriptor.exe));
        _iceVersion.setText(resolver.substitute(descriptor.iceVersion));
        _pwd.setText(resolver.substitute(descriptor.pwd));

        Utils.Stringifier stringifier =
            new Utils.Stringifier() {
                @Override
                public String toString(Object obj) {
                    return resolver.substitute((String) obj);
                }
            };

        Utils.StringifyResult r = Utils.stringify(descriptor.options, stringifier, " ");
        _options.setText(r.returnValue);
        _options.setToolTipText(r.toolTip);

        _envs.setEnvs(descriptor.envs, resolver);

        _user.setText(resolver.substitute(descriptor.user));

        _activation.setText(resolver.substitute(descriptor.activation));
        _activationTimeout.setText(resolver.substitute(descriptor.activationTimeout));
        _deactivationTimeout.setText(resolver.substitute(descriptor.deactivationTimeout));

        _allocatable.setSelected(descriptor.allocatable);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder) {
        builder.appendSeparator("Runtime Status");

        builder.append("State");
        builder.append(_currentState, 3);
        builder.nextLine();

        builder.append("", _enabled);
        builder.nextLine();

        builder.append("Process Id");
        builder.append(_currentPid, 3);
        builder.nextLine();
        appendRuntimeProperties(builder);

        builder.appendSeparator("Configuration");
        builder.append("Application");
        builder.append(_application);
        builder.append(_gotoApplication);
        builder.nextLine();
        appendDescriptorProperties(builder);

        builder.appendSeparator("Activation");
        builder.append("Path to Executable");
        builder.append(_exe, 3);
        builder.nextLine();
        builder.append("Ice Version");
        builder.append(_iceVersion, 3);
        builder.nextLine();
        builder.append("Working Directory");
        builder.append(_pwd, 3);
        builder.nextLine();
        builder.append("Command Arguments");
        builder.append(_options, 3);
        builder.nextLine();
        builder.append("Run as");
        builder.append(_user, 3);
        builder.nextLine();
        builder.append("Environment Variables");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-6);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_envs);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();

        builder.append("Activation Mode");
        builder.append(_activation, 3);
        builder.nextLine();
        builder.append("Activation Timeout");
        builder.append(_activationTimeout, 3);
        builder.nextLine();
        builder.append("Deactivation Timeout");
        builder.append(_deactivationTimeout, 3);
        builder.nextLine();
        builder.append("", _allocatable);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel() {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Server Properties");
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
            addSeparator();
            add(la.get(TreeNode.ENABLE));
            add(la.get(TreeNode.DISABLE));
        }
    }

    private final Coordinator _coordinator;

    private final JTextField _currentState = new JTextField(20);
    private final JCheckBox _enabled = new JCheckBox("Enabled");
    private final JTextField _currentPid = new JTextField(20);

    private final JTextField _application = new JTextField(20);
    private final JButton _gotoApplication;

    private final JTextField _exe = new JTextField(20);
    private final JTextField _iceVersion = new JTextField(20);
    private final JTextField _pwd = new JTextField(20);
    private final JTextField _user = new JTextField(20);

    private final JTextField _activation = new JTextField(20);
    private final JTextField _activationTimeout = new JTextField(20);
    private final JTextField _deactivationTimeout = new JTextField(20);
    private final JCheckBox _allocatable = new JCheckBox("Allocatable");

    private final TableField _envs = new TableField("Name", "Value");

    private final JTextField _options = new JTextField(20);
    private JToolBar _toolBar;
}
