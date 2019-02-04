//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.IceGridGUI.Application;

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

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

@SuppressWarnings("unchecked")
class ServiceInstanceEditor extends CommunicatorChildEditor
{
    ServiceInstanceEditor()
    {
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
        gotoTemplate.putValue(Action.SHORT_DESCRIPTION, "Goto this service template");
        _templateButton = new JButton(gotoTemplate);

        _parameters = new ParameterValuesField(this);

        _propertySets.getDocument().addDocumentListener(_updateListener);
        _propertySets.setToolTipText("Property Set References");
        _properties = new PropertiesField(this);
    }

    ServiceInstanceDescriptor getDescriptor()
    {
        return (ServiceInstanceDescriptor)_target.getDescriptor();
    }

    //
    // From Editor:
    //
    @Override
    Utils.Resolver getDetailResolver()
    {
        ServiceInstance service = (ServiceInstance)_target;
        if(service.getCoordinator().substitute())
        {
            return service.getResolver();
        }
        else
        {
            return null;
        }
    }

    @Override
    void writeDescriptor()
    {
        ServiceInstanceDescriptor descriptor = getDescriptor();
        descriptor.template = ((ServiceTemplate)_template.getSelectedItem()).getId();
        descriptor.parameterValues = _parameters.getValues();

        descriptor.propertySet.references = _propertySets.getList().toArray(new String[0]);
        descriptor.propertySet.properties = _properties.getProperties();
    }

    @Override
    boolean isSimpleUpdate()
    {
        ServiceInstanceDescriptor descriptor = getDescriptor();
        ServiceTemplate t = (ServiceTemplate)_template.getSelectedItem();

        return descriptor.template.equals(t.getId()) && descriptor.parameterValues.equals(_parameters.getValues());
    }

    @Override
    Communicator.ChildList getChildList()
    {
        return ((Communicator)_target.getParent()).getServices();
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

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
    }

    void show(ServiceInstance service)
    {
        detectUpdates(false);
        _target = service;

        //
        // If it's not a template instance, it's shown using
        // ServiceEditor.show()
        //
        assert getDescriptor().template.length() > 0;

        ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)service.getDescriptor();
        Coordinator coordinator = service.getCoordinator();
        boolean isEditable = service.isEphemeral() || !coordinator.substitute();

        Utils.Resolver resolver = isEditable ? null : ((TreeNode)service.getParent()).getResolver();

        //
        // Need to make control enabled before changing it
        //
        _template.setEnabled(true);

        ServiceTemplates serviceTemplates = service.getRoot().getServiceTemplates();
        _template.setModel(serviceTemplates.createComboBoxModel());

        ServiceTemplate t = (ServiceTemplate)serviceTemplates.findChild(descriptor.template);
        assert t != null;
        _template.setSelectedItem(t);

        ListDataListener templateListener = new ListDataListener()
            {
                @Override
                public void contentsChanged(ListDataEvent e)
                {
                    updated();

                    ServiceTemplate t = (ServiceTemplate)_template.getModel().getSelectedItem();

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

        _applyButton.setEnabled(service.isEphemeral());
        _discardButton.setEnabled(service.isEphemeral());
        detectUpdates(true);
        if(service.isEphemeral())
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
