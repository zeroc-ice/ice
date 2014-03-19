// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
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

    protected void appendProperties(DefaultFormBuilder builder)
    {
        _subEditor.appendProperties(builder);
    }

    protected void buildPropertiesPanel()
    {
        super.buildPropertiesPanel();
        _propertiesPanel.setName("Service Properties");
    }

    protected boolean validate()
    {
        return _subEditor.validate();
    }

    void writeDescriptor()
    {
        _subEditor.writeDescriptor();
    }

    boolean isSimpleUpdate()
    {
        return _subEditor.isSimpleUpdate();
    }

    Communicator.ChildList getChildList()
    {
        return ((Communicator)_target.getParent()).getServices();
    }

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
