// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JOptionPane;
import javax.swing.JScrollPane;
import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class PropertySetEditor extends Editor
{
    @Override
    protected boolean applyUpdate(boolean refresh)
    {
        PropertySet nps = (PropertySet)_target;
        Root root = nps.getRoot();

        root.disableSelectionListener();
        try
        {
            PropertySetParent parent = (PropertySetParent)nps.getParent();
            if(nps.isEphemeral())
            {
                writeDescriptor();
                PropertySetDescriptor descriptor = (PropertySetDescriptor)nps.getDescriptor();
                nps.destroy(); // just removes the child

                try
                {
                    parent.tryAdd(getIdText(), descriptor);
                }
                catch(UpdateFailedException e)
                {
                    //
                    // Add back ephemeral child
                    //
                    try
                    {
                        parent.insertPropertySet(nps, true);
                    }
                    catch(UpdateFailedException die)
                    {
                        assert false;
                    }
                    root.setSelectedNode(_target);

                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                _target = ((TreeNode)parent).findChildWithDescriptor(descriptor);
                root.updated();
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }
            else if(!isSimpleUpdate())
            {
                PropertySetDescriptor descriptor = (PropertySetDescriptor)nps.getDescriptor();

                try
                {
                    parent.tryRename(_target.getId(), _oldId, getIdText());
                }
                catch(UpdateFailedException e)
                {
                    JOptionPane.showMessageDialog(
                        root.getCoordinator().getMainFrame(),
                        e.toString(),
                        "Apply failed",
                        JOptionPane.ERROR_MESSAGE);
                    return false;
                }

                //
                // Success
                //
                _target = ((TreeNode)parent).findChildWithDescriptor(descriptor);
                writeDescriptor();
                root.updated();
                if(refresh)
                {
                    root.setSelectedNode(_target);
                }
            }
            else
            {
                writeDescriptor();
                root.updated();
                nps.getEditable().markModified();
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

    @Override
    Utils.Resolver getDetailResolver()
    {
        Root root = _target.getRoot();

        if(root.getCoordinator().substitute())
        {
            return _target.getResolver();
        }
        else
        {
            return null;
        }
    }

    PropertySetEditor()
    {
        this("ID");
        _id.setToolTipText("The id of this Property Set");
        _id.getDocument().addDocumentListener(_updateListener);
    }

    protected PropertySetEditor(String label)
    {
        _idLabel = new JLabel(label);

        _propertySets.getDocument().addDocumentListener(_updateListener);
        _propertySets.setToolTipText("Property Set References");
        _properties = new PropertiesField(this);
    }

    void writeDescriptor()
    {
        PropertySetDescriptor descriptor = (PropertySetDescriptor)getPropertySet().getDescriptor();

        descriptor.references = _propertySets.getList().toArray(new String[0]);
        descriptor.properties = _properties.getProperties();
    }

    boolean isSimpleUpdate()
    {
        return getIdText().equals(_oldId);
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append(_idLabel);
        builder.append(getIdComponent(), 3);
        builder.nextLine();

        builder.append("Property Sets");
        builder.append(_propertySets, 3);
        builder.nextLine();

        builder.append("Properties");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");
        builder.nextLine();
        builder.append("");

        builder.nextRow(-6);
        JScrollPane scrollPane = new JScrollPane(_properties);
        CellConstraints cc = new CellConstraints();
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Named Property Set");
    }

    @Override
    protected boolean validate()
    {
        return check(new String[]{_idLabel.getText(), getIdText()});
    }

    void show(String unsubstitutedId, PropertySet nps)
    {
        detectUpdates(false);
        _target = nps;

        Utils.Resolver resolver = getDetailResolver();
        boolean isEditable = (resolver == null);

        PropertySetDescriptor descriptor = (PropertySetDescriptor)nps.getDescriptor();

        showId(unsubstitutedId, resolver);
        _oldId = unsubstitutedId;

        _propertySets.setList(java.util.Arrays.asList(descriptor.references), resolver);
        _propertySets.setEditable(isEditable);

        _properties.setProperties(descriptor.properties, null, null, getDetailResolver(), isEditable);

        _applyButton.setEnabled(nps.isEphemeral());
        _discardButton.setEnabled(nps.isEphemeral());
        detectUpdates(true);
        if(nps.isEphemeral())
        {
            updated();
        }
    }

    protected JComponent getIdComponent()
    {
        return _id;
    }

    protected String getIdText()
    {
        return _id.getText().trim();
    }

    protected void showId(String unsubstitutedId, Utils.Resolver resolver)
    {
        //
        // This version does NOT substitute the ID
        //
        _id.setText(unsubstitutedId);
        _id.setEditable(resolver == null);
    }

    private PropertySet getPropertySet()
    {
        return (PropertySet)_target;
    }

    private String _oldId;

    private final JTextField _id = new JTextField(20);
    private final JLabel _idLabel;

    private ListTextField _propertySets = new ListTextField(20);
    private PropertiesField _properties;
}
