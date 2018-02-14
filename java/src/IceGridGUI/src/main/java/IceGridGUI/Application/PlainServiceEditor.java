// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class PlainServiceEditor extends CommunicatorChildEditor
{
    PlainServiceEditor()
    {
        _subEditor = new ServiceSubEditor(this);
    }

    //
    // From Editor:
    //
    @Override
    Utils.Resolver getDetailResolver()
    {
        PlainService service = (PlainService)_target;
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
    protected void appendProperties(DefaultFormBuilder builder)
    {
        _subEditor.appendProperties(builder);
    }

    @Override
    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
    }

    @Override
    protected boolean validate()
    {
        return _subEditor.validate();
    }

    @Override
    void writeDescriptor()
    {
        _subEditor.writeDescriptor();
    }

    @Override
    boolean isSimpleUpdate()
    {
        return _subEditor.isSimpleUpdate();
    }

    @Override
    Communicator.ChildList getChildList()
    {
        return ((Communicator)_target.getParent()).getServices();
    }

    @Override
    Object getSubDescriptor()
    {
        ServiceInstanceDescriptor sid = (ServiceInstanceDescriptor)_target.getDescriptor();
        return sid.descriptor;
    }

    void show(PlainService service)
    {
        detectUpdates(false);
        _target = service;

        _subEditor.show(true);

        _applyButton.setEnabled(service.isEphemeral());
        _discardButton.setEnabled(service.isEphemeral());
        detectUpdates(true);
        if(service.isEphemeral())
        {
            updated();
        }
    }

    private ServiceSubEditor _subEditor;
}
