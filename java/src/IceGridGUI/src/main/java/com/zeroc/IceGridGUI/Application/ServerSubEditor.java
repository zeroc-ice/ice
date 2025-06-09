// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.ServerDescriptor;
import com.zeroc.IceGridGUI.Utils;

import java.awt.event.ActionEvent;
import java.util.LinkedList;
import java.util.Map;
import java.util.TreeMap;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JCheckBox;
import javax.swing.JComboBox;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

@SuppressWarnings("unchecked")
class ServerSubEditor extends CommunicatorSubEditor {
    ServerSubEditor(Editor mainEditor) {
        super(mainEditor);

        _id.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _id.setToolTipText("Must be unique within this IceGrid deployment");

        _exe.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _exe.setToolTipText(
            "<html>Path to this server's executable, e.g.:<br>"
                + "icebox<br>"
                + "java<br>"
                + "myHelloServer<br>"
                + "C:\\testbed\\hello\\server</html>");

        _iceVersion.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _iceVersion.setToolTipText(
            "<html>The Ice version used by this server;<br>"
                + "blank means 'same version as the IceGrid registry'.</html>");

        _pwd.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _pwd.setToolTipText(
            "<html>If not set, the server will start in "
                + "<i>node data dir</i>/servers/<i>server-id</i>;<br>"
                + "relative directories are relative to the current directory"
                + " of the icegridnode process.</html>");

        _options.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _options.setToolTipText(
            "<html>Command-line arguments for this server.<br>"
                + "Use whitespace as separator; use double-quotes around arguments containing whitespace</html>");

        _user.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _user.setToolTipText(
            "<html>Run the server using this user account.<br>"
                + "This feature is only available on Unix/Linux, when the IceGrid node runs as root.</html>");

        _envs = new SimpleMapField(mainEditor, true, "Name", "Value");

        _activation = new JComboBox(new String[]{ALWAYS, MANUAL, ON_DEMAND, SESSION});
        _activation.setToolTipText(
            "<html>always: IceGrid starts and keeps the server up all the time<br>"
                + "manual: you start the server yourself<br>"
                + "on-demand: IceGrid starts the server when a client needs it<br>"
                + "session: IceGrid starts and shuts down the server for each session</html>");

        JTextField activationTextField = (JTextField) _activation.getEditor().getEditorComponent();
        activationTextField.getDocument().addDocumentListener(_mainEditor.getUpdateListener());

        _activationTimeout.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _activationTimeout.setToolTipText(
            "<html>Number of seconds; if not set or set to 0, "
                + "the IceGrid Node<br> uses the value of its "
                + "IceGrid.Node.WaitTime property</html>");
        _deactivationTimeout.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _deactivationTimeout.setToolTipText(
            "<html>Number of seconds; if not set or set to 0, "
                + "the IceGrid Node<br> uses the value of its "
                + "IceGrid.Node.WaitTime property</html>");

        Action allocatable =
            new AbstractAction("Allocatable") {
                @Override
                public void actionPerformed(ActionEvent e) {
                    _mainEditor.updated();
                }
            };
        allocatable.putValue(
            Action.SHORT_DESCRIPTION,
            "<html>Check this box to ensure that the well-known objects<br>"
                + "of this server can only be allocated by one session at a time.</html>");
        _allocatable = new JCheckBox(allocatable);
    }

    ServerDescriptor getServerDescriptor() {
        return (ServerDescriptor) _mainEditor.getSubDescriptor();
    }

    @Override
    void appendProperties(DefaultFormBuilder builder) {
        builder.append("Server ID");
        builder.append(_id, 3);
        builder.nextLine();

        // Add Communicator fields
        super.appendProperties(builder);

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

    void writeDescriptor() {
        ServerDescriptor descriptor = getServerDescriptor();
        descriptor.id = _id.getText().trim();
        descriptor.exe = _exe.getText().trim();
        descriptor.iceVersion = _iceVersion.getText().trim();
        descriptor.pwd = _pwd.getText().trim();

        descriptor.options = _options.getList();
        descriptor.user = _user.getText().trim();

        descriptor.envs = new LinkedList<>();
        for (Map.Entry<String, String> p : _envs.get().entrySet()) {
            descriptor.envs.add(p.getKey() + "=" + p.getValue());
        }

        descriptor.activation = _activation.getSelectedItem().toString().trim();
        descriptor.activationTimeout = _activationTimeout.getText().trim();
        descriptor.deactivationTimeout = _deactivationTimeout.getText().trim();

        descriptor.allocatable = _allocatable.isSelected();

        super.writeDescriptor(descriptor);
    }

    boolean isSimpleUpdate() {
        return getServerDescriptor().id.equals(_id.getText().trim());
    }

    boolean validate() {
        return _mainEditor.check(
            new String[]{
                "Server ID", _id.getText().trim(),
                "Path to Executable", _exe.getText().trim()
            });
    }

    void show(boolean isEditable) {
        ServerDescriptor descriptor = getServerDescriptor();
        Utils.Resolver detailResolver = _mainEditor.getDetailResolver();

        isEditable = isEditable && (detailResolver == null);

        if (detailResolver != null) {
            _id.setText(detailResolver.find("server"));
        } else {
            _id.setText(descriptor.id);
        }
        _id.setEditable(isEditable);

        _exe.setText(Utils.substitute(descriptor.exe, detailResolver));
        _exe.setEditable(isEditable);
        _iceVersion.setText(Utils.substitute(descriptor.iceVersion, detailResolver));
        _iceVersion.setEditable(isEditable);

        _pwd.setText(Utils.substitute(descriptor.pwd, detailResolver));
        _pwd.setEditable(isEditable);

        _options.setList(descriptor.options, detailResolver);
        _options.setEditable(isEditable);

        _user.setText(Utils.substitute(descriptor.user, detailResolver));
        _user.setEditable(isEditable);

        Map<String, String> envMap = new TreeMap<>();
        for (String p : descriptor.envs) {
            int equal = p.indexOf('=');
            if (equal == -1 || equal == p.length() - 1) {
                envMap.put(p, "");
            } else {
                envMap.put(p.substring(0, equal), p.substring(equal + 1));
            }
        }
        _envs.set(envMap, detailResolver, isEditable);

        String activation = Utils.substitute(descriptor.activation, detailResolver);

        _activation.setEnabled(true);
        _activation.setEditable(true);
        if (ALWAYS.equals(activation)) {
            _activation.setSelectedItem(ALWAYS);
        } else if (MANUAL.equals(activation)) {
            _activation.setSelectedItem(MANUAL);
        } else if (ON_DEMAND.equals(activation)) {
            _activation.setSelectedItem(ON_DEMAND);
        } else if (SESSION.equals(activation)) {
            _activation.setSelectedItem(SESSION);
        } else {
            _activation.setSelectedItem(activation);
        }
        _activation.setEnabled(isEditable);
        _activation.setEditable(isEditable);

        _activationTimeout.setText(Utils.substitute(descriptor.activationTimeout, detailResolver));
        _activationTimeout.setEditable(isEditable);

        _deactivationTimeout.setText(
            Utils.substitute(descriptor.deactivationTimeout, detailResolver));
        _deactivationTimeout.setEditable(isEditable);

        _allocatable.setSelected(descriptor.allocatable);
        _allocatable.setEnabled(isEditable);
        show(descriptor, isEditable);
    }

    private static final String ALWAYS = "always";
    private static final String MANUAL = "manual";
    private static final String ON_DEMAND = "on-demand";
    private static final String SESSION = "session";

    private JTextField _id = new JTextField(20);
    private JTextField _exe = new JTextField(20);
    private JTextField _iceVersion = new JTextField(20);
    private JTextField _pwd = new JTextField(20);
    private ListTextField _options = new ListTextField(20);
    private JTextField _user = new JTextField(20);
    private SimpleMapField _envs;

    private JComboBox _activation;
    private JTextField _activationTimeout = new JTextField(20);
    private JTextField _deactivationTimeout = new JTextField(20);
    private JCheckBox _allocatable;
}
