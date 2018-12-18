// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;

class ServiceTemplateEditor extends TemplateEditor
{
    ServiceTemplateEditor()
    {
        _subEditor = new ServiceSubEditor(this);
    }

    @Override
    void writeDescriptor()
    {
        super.writeDescriptor();
        _subEditor.writeDescriptor();
    }

    @Override
    boolean isSimpleUpdate()
    {
        return super.isSimpleUpdate() && _subEditor.isSimpleUpdate();
    }

    @Override
    protected void appendProperties(DefaultFormBuilder builder)
    {
        super.appendProperties(builder);
        builder.appendSeparator();
        builder.nextLine();
        _subEditor.appendProperties(builder);
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Template Properties");
    }

    @Override
    protected boolean validate()
    {
        return super.validate() && _subEditor.validate();
    }

    void show(ServiceTemplate t)
    {
        detectUpdates(false);
        _target = t;

        super.show();
        _subEditor.show(true);

        _applyButton.setEnabled(t.isEphemeral());
        _discardButton.setEnabled(t.isEphemeral());
        detectUpdates(true);
        if(t.isEphemeral())
        {
            updated();
        }
    }

    private ServiceSubEditor _subEditor;
}
