// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.DefaultComboBoxModel;
import javax.swing.JComboBox;
import javax.swing.JComponent;
import javax.swing.JTextField;

import IceGridGUI.*;

class ServerInstancePropertySetEditor extends PropertySetEditor
{
    ServerInstancePropertySetEditor()
    {
        super("Service Name");
        _id.setToolTipText("The name of the service");

        JTextField idTextField = (JTextField)_id.getEditor().getEditorComponent();
        idTextField.getDocument().addDocumentListener(_updateListener);
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Instance Property Set");
    }

    @Override
    protected JComponent getIdComponent()
    {
        return _id;
    }

    @Override
    protected String getIdText()
    {
        if(_id.getSelectedItem() == null)
        {
            return "";
        }
        else
        {
            return _id.getSelectedItem().toString().trim();
        }
    }

    @Override
    @SuppressWarnings("unchecked")
    protected void showId(String unsubstitutedId, Utils.Resolver resolver)
    {
        ServerInstance s = (ServerInstance)_target.getParent();

        _id.setEnabled(true);
        _id.setEditable(true);
        _id.setModel(new DefaultComboBoxModel(s.getServiceNames()));
        _id.setSelectedItem(Utils.substitute(unsubstitutedId, resolver));
        _id.setEditable(resolver == null);
        _id.setEnabled(resolver == null);
    }

    private JComboBox _id = new JComboBox();
}
