// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;

class TemplateEditor extends Editor
{
    TemplateEditor()
    {
        _template.getDocument().addDocumentListener(_updateListener);
        _template.setToolTipText("Must be unique within the enclosing application");

        _parameters = new ParametersField(this);
    }

    TemplateDescriptor getDescriptor()
    {
        return (TemplateDescriptor)_target.getDescriptor();
    }

    @Override
    Object getSubDescriptor()
    {
        return getDescriptor().descriptor;
    }

    void writeDescriptor()
    {
        TemplateDescriptor descriptor = getDescriptor();
        java.util.LinkedList<String> parameters = new java.util.LinkedList<>();
        descriptor.parameterDefaults = _parameters.get(parameters);
        descriptor.parameters = parameters;
    }

    boolean isSimpleUpdate()
    {
        TemplateDescriptor descriptor = getDescriptor();
        java.util.List<String> parameters = new java.util.LinkedList<>();
        java.util.Map<String, String> defaultValues = _parameters.get(parameters);

        return descriptor.parameters.equals(parameters) && descriptor.parameterDefaults.equals(defaultValues);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Template ID");
        builder.append(_template, 3);
        builder.nextLine();

        builder.append("Parameters");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");

        builder.nextRow(-6);
        JScrollPane scrollPane = new JScrollPane(_parameters);
        CellConstraints cc = new CellConstraints();
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    @Override
    protected boolean validate()
    {
        return check(new String[]{"Template ID", _template.getText().trim()});
    }

    void show()
    {
        TemplateDescriptor descriptor = getDescriptor();
        _template.setText(_target.getId());
        _template.setEditable(_target.isEphemeral());

        _parameters.set(descriptor.parameters, descriptor.parameterDefaults);
    }

    @Override
    protected boolean applyUpdate(boolean refresh)
    {
        Root root = _target.getRoot();
        root.disableSelectionListener();

        try
        {
            if(_target.isEphemeral())
            {
                writeDescriptor();
                TemplateDescriptor descriptor = getDescriptor();
                Templates parent = (Templates)_target.getParent();
                _target.destroy(); // just removes the child

                try
                {
                    parent.tryAdd(_template.getText().trim(), descriptor);
                }
                catch(UpdateFailedException e)
                {
                    //
                    // Re-add ephemeral child
                    //
                    try
                    {
                        parent.insertChild(_target, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    root.setSelectedNode(_target);

                    JOptionPane.showMessageDialog(
                        _target.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                _target = parent.findChildWithDescriptor(descriptor);
                root.updated();
                _template.setEditable(false);
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }
            else if(isSimpleUpdate())
            {
                writeDescriptor();
                ((Communicator)_target).getEnclosingEditable().markModified();
                root.updated();
            }
            else
            {
                //
                // Save to be able to rollback
                //
                Object savedDescriptor = ((Communicator)_target).saveDescriptor();
                Templates parent = (Templates)_target.getParent();
                writeDescriptor();

                try
                {
                    parent.tryUpdate((Communicator)_target);
                }
                catch(UpdateFailedException e)
                {
                    ((Communicator)_target).restoreDescriptor(savedDescriptor);

                    JOptionPane.showMessageDialog(
                        _target.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    //
                    // Everything was restored, user must deal with error
                    //
                    return false;
                }

                //
                // Success
                //
                ((Communicator)_target).getEnclosingEditable().markModified();
                root.updated();

                _target = parent.findChildWithDescriptor(getDescriptor());
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }

            if(refresh)
            {
                root.getCoordinator().getCurrentTab().showNode(_target);
            }
            _applyButton.setEnabled(false);
            _discardButton.setEnabled(false);
            return true;
        }
        finally
        {
            root.enableSelectionListener();
        }
    }

    private JTextField _template = new JTextField(20);
    private ParametersField _parameters;
}
