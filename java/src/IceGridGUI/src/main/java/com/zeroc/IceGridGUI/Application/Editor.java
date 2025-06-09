// Copyright (c) ZeroC, Inc.

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.ButtonBarBuilder;
import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;

import com.zeroc.IceGridGUI.EditorBase;
import com.zeroc.IceGridGUI.Utils;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;
import java.util.HashMap;
import java.util.List;
import java.util.Map;

import javax.swing.AbstractAction;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JOptionPane;
import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

public class Editor extends EditorBase {
    public static Map<String, String> makeParameterValues(
            Map<String, String> oldParameterValues,
            List<String> newParameters) {
        Map<String, String> result = new HashMap<>();

        for (String name : newParameters) {
            String value = oldParameterValues.get(name);
            if (value != null) {
                result.put(name, value);
            }
        }
        return result;
    }

    public boolean save(boolean refresh) {
        if (_applyButton.isEnabled()) {
            return validate() && applyUpdate(refresh);
        } else {
            return true;
        }
    }

    protected boolean validate() {
        return true;
    }

    protected boolean applyUpdate(boolean refresh) {
        assert false;
        return false;
    }

    protected void detectUpdates(boolean val) {
        _detectUpdates = val;
    }

    protected void discardUpdate() {
        Root root = _target.getRoot();

        if (_target.isEphemeral()) {
            _target.delete();
        } else {
            _target.getCoordinator().getCurrentTab().showNode(_target);
        }
        root.cancelEdit();
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder) {}

    @Override
    protected void buildPropertiesPanel() {
        super.buildPropertiesPanel();
        JComponent buttonBar =
            new ButtonBarBuilder().addGlue().addButton(_applyButton, _discardButton).build();
        buttonBar.setBorder(Borders.DIALOG);
        _propertiesPanel.add(buttonBar, BorderLayout.SOUTH);
    }

    Editor() {
        // _applyButton
        AbstractAction apply =
            new AbstractAction("Apply") {
                @Override
                public void actionPerformed(ActionEvent e) {
                    if (validate()) {
                        if (applyUpdate(true)) {
                            _target.getRoot().getTree().grabFocus();
                        }
                    }
                }
            };
        _applyButton = new JButton(apply);
        _applyButton.setEnabled(false);

        // _discardButton
        AbstractAction discard =
            new AbstractAction("Discard") {
                @Override
                public void actionPerformed(ActionEvent e) {
                    discardUpdate();
                    _target.getRoot().getTree().grabFocus();
                }
            };
        _discardButton = new JButton(discard);
        _discardButton.setEnabled(false);

        _updateListener =
            new DocumentListener() {
                @Override
                public void changedUpdate(DocumentEvent e) {
                    updated();
                }

                @Override
                public void insertUpdate(DocumentEvent e) {
                    updated();
                }

                @Override
                public void removeUpdate(DocumentEvent e) {
                    updated();
                }
            };
    }

    // Used by the sub-editor (when there is one)
    Object getSubDescriptor() {
        return null;
    }

    Utils.Resolver getDetailResolver() {
        return null;
    }

    public TreeNode getTarget() {
        return _target;
    }

    void updated() {
        if (_detectUpdates) {
            _target.getRoot().disableRegistryUpdates();
            _applyButton.setEnabled(true);
            _discardButton.setEnabled(true);
        }
    }

    DocumentListener getUpdateListener() {
        return _updateListener;
    }

    // Check that these 'val' are filled in
    boolean check(String[] nameValArray) {
        String emptyFields = "";
        int errorCount = 0;

        for (int i = 1; i < nameValArray.length; i += 2) {
            if (nameValArray[i] == null || nameValArray[i].isEmpty()) {
                errorCount++;
                if (!emptyFields.isEmpty()) {
                    emptyFields += "\n";
                }
                emptyFields += "'" + nameValArray[i - 1] + "'";
            }
        }

        if (errorCount > 0) {
            String message =
                errorCount == 1
                    ? emptyFields + " cannot be empty"
                    : "The following fields cannot be empty:\n" + emptyFields;

            JOptionPane.showMessageDialog(
                _target.getCoordinator().getMainFrame(),
                message,
                "Validation failed",
                JOptionPane.ERROR_MESSAGE);
        }

        return errorCount == 0;
    }

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;

    protected TreeNode _target;
    private boolean _detectUpdates = true;
}
