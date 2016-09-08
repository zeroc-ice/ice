// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComboBox;
import javax.swing.JScrollPane;
import javax.swing.event.ListDataEvent;
import javax.swing.event.ListDataListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.layout.CellConstraints;

import IceGrid.*;
import IceGridGUI.*;

@SuppressWarnings("unchecked")
class ServerInstanceEditor extends AbstractServerEditor
{
    @Override
    protected void writeDescriptor()
    {
        ServerInstanceDescriptor descriptor = getDescriptor();
        ServerTemplate t = (ServerTemplate)_template.getSelectedItem();

        descriptor.template = t.getId();
        descriptor.parameterValues = _parameters.getValues();

        descriptor.propertySet.references = _propertySets.getList().toArray(new String[0]);
        descriptor.propertySet.properties = _properties.getProperties();

        ((ServerInstance)_target).isIceBox(
            ((TemplateDescriptor)t.getDescriptor()).descriptor instanceof IceBoxDescriptor);
    }

    @Override
    protected boolean isSimpleUpdate()
    {
        ServerInstanceDescriptor descriptor = getDescriptor();
        ServerTemplate t = (ServerTemplate)_template.getSelectedItem();

        return descriptor.template.equals(t.getId()) && descriptor.parameterValues.equals(_parameters.getValues());
    }

    ServerInstanceEditor()
    {
        _template.setToolTipText("Server template");

        //
        // Template
        //
        Action gotoTemplate = new AbstractAction(
            "", Utils.getIcon("/icons/16x16/goto.png"))
            {
                @Override
                public void actionPerformed(ActionEvent e)
                {
                    TreeNode t = (TreeNode)_template.getSelectedItem();
                    if(t != null)
                    {
                        t.getRoot().setSelectedNode(t);
                    }
                }
            };
        gotoTemplate.putValue(Action.SHORT_DESCRIPTION, "Goto this server template");
        _templateButton = new JButton(gotoTemplate);

        _parameters = new ParameterValuesField(this);

        _propertySets.getDocument().addDocumentListener(_updateListener);
        _propertySets.setToolTipText("Property Set References");

        _properties = new PropertiesField(this);
    }

    ServerInstanceDescriptor getDescriptor()
    {
        return (ServerInstanceDescriptor)_target.getDescriptor();
    }

    //
    // From Editor:
    //

    @Override
    Utils.Resolver getDetailResolver()
    {
        if(_target.getCoordinator().substitute())
        {
            return _target.getResolver();
        }
        else
        {
            return null;
        }
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Template", _template);
        builder.append(_templateButton);
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
        scrollPane = new JScrollPane(_properties);
        builder.add(scrollPane, cc.xywh(builder.getColumn(), builder.getRow(), 3, 7));
        builder.nextRow(6);
        builder.nextLine();
    }

    void show(ServerInstance server)
    {
        detectUpdates(false);
        _target = server;

        ServerInstanceDescriptor descriptor = getDescriptor();
        Root root = server.getRoot();
        boolean isEditable = server.isEphemeral() || !root.getCoordinator().substitute();

        Utils.Resolver resolver = isEditable ? null : ((Node)_target.getParent()).getResolver();

        //
        // Need to make control enabled before changing it
        //
        _template.setEnabled(true);

        ServerTemplates serverTemplates = root.getServerTemplates();
        _template.setModel(serverTemplates.createComboBoxModel());

        ServerTemplate t = (ServerTemplate)serverTemplates.findChild(descriptor.template);
        assert t != null;
        _template.setSelectedItem(t);

        ListDataListener templateListener = new ListDataListener()
            {
                @Override
                public void contentsChanged(ListDataEvent e)
                {
                    updated();

                    ServerTemplate t = (ServerTemplate)_template.getModel().getSelectedItem();

                    TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();

                    //
                    // Replace parameters but keep existing values
                    //
                    _parameters.set(td.parameters,
                                    makeParameterValues(_parameters.getValues(), td.parameters),
                                    td.parameterDefaults, null);
                }

                @Override
                public void intervalAdded(ListDataEvent e)
                {
                }

                @Override
                public void intervalRemoved(ListDataEvent e)
                {
                }
            };

        _template.getModel().addListDataListener(templateListener);
        _template.setEnabled(isEditable);

        TemplateDescriptor td = (TemplateDescriptor)t.getDescriptor();
        _parameters.set(td.parameters, descriptor.parameterValues, td.parameterDefaults, resolver);

        _propertySets.setList(java.util.Arrays.asList(descriptor.propertySet.references), getDetailResolver());
        _propertySets.setEditable(isEditable);

        _properties.setProperties(descriptor.propertySet.properties, null, null, getDetailResolver(), isEditable);

        _applyButton.setEnabled(server.isEphemeral());
        _discardButton.setEnabled(server.isEphemeral());
        detectUpdates(true);
        if(server.isEphemeral())
        {
            updated();
        }
    }

    private JComboBox _template = new JComboBox();
    private JButton _templateButton;
    private ParameterValuesField _parameters;

    private ListTextField _propertySets = new ListTextField(20);
    private PropertiesField _properties;
}
