// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.ApplicationDescriptor;
import com.zeroc.IceGridGUI.MainPane;
import com.zeroc.IceGridGUI.Utils;

import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextArea;
import javax.swing.JTextField;

@SuppressWarnings("unchecked")
class ApplicationEditor extends Editor {
    @Override
    protected boolean applyUpdate(boolean refresh) {
        Root root = (Root) _target;
        MainPane mainPane = _target.getCoordinator().getMainPane();

        root.disableSelectionListener();
        try {
            if (isSimpleUpdate()) {
                writeDescriptor();
                root.updated();
                root.getEditable().markModified();
            } else {
                // Save to be able to rollback
                ApplicationDescriptor savedDescriptor = root.saveDescriptor();
                writeDescriptor();
                try {
                    root.rebuild();
                } catch (UpdateFailedException e) {
                    root.restoreDescriptor(savedDescriptor);
                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                // Success
                root.updated();
                root.getEditable().markModified();

                if (!savedDescriptor.name.equals(root.getId())) {
                    mainPane.resetTitle(root);
                    root.getTreeModel().nodeChanged(root);
                }
            }

            _applyButton.setEnabled(false);
            _discardButton.setEnabled(false);
            return true;
        } finally {
            root.enableSelectionListener();
        }
    }

    ApplicationEditor() {
        _name.getDocument().addDocumentListener(_updateListener);
        _description.getDocument().addDocumentListener(_updateListener);

        // Variables
        _variables = new SimpleMapField(this, false, "Name", "Value");
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder) {
        builder.append("Name");
        builder.append(_name, 3);

        builder.append("Description");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-2);
        CellConstraints cc = new CellConstraints();
        JScrollPane scrollPane = new JScrollPane(_description);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 3));
        builder.nextRow(2);
        builder.nextLine();

        builder.append("Variables");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextRow(-6);
        scrollPane = new JScrollPane(_variables);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel() {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Application Properties");
    }

    boolean isSimpleUpdate() {
        ApplicationDescriptor descriptor = (ApplicationDescriptor) _target.getDescriptor();
        return descriptor.name.equals(_name.getText().trim())
            && _variables.get().equals(descriptor.variables);
    }

    void writeDescriptor() {
        ApplicationDescriptor descriptor = (ApplicationDescriptor) _target.getDescriptor();
        descriptor.name = _name.getText().trim();
        descriptor.variables = _variables.get();
        descriptor.description = _description.getText();
    }

    @Override
    protected boolean validate() {
        return check(new String[]{"Name", _name.getText().trim()});
    }

    void show(Root root) {
        detectUpdates(false);
        _target = root;

        Utils.Resolver resolver = getDetailResolver();
        boolean isEditable = resolver == null;

        ApplicationDescriptor descriptor = (ApplicationDescriptor) root.getDescriptor();

        _name.setText(descriptor.name);
        _name.setEditable(!root.isLive() && isEditable);

        _description.setText(Utils.substitute(descriptor.description, resolver));
        _description.setEditable(isEditable);
        _description.setOpaque(isEditable);
        _description.setToolTipText("An optional description for this application");

        _variables.set(descriptor.variables, resolver, isEditable);

        _applyButton.setEnabled(false);
        _discardButton.setEnabled(false);
        detectUpdates(true);
    }

    @Override
    Utils.Resolver getDetailResolver() {
        if (_target.getCoordinator().substitute()) {
            return _target.getResolver();
        } else {
            return null;
        }
    }

    private JTextField _name = new JTextField(20);
    private JTextArea _description = new JTextArea(3, 20);
    private SimpleMapField _variables;
}
