// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import com.zeroc.IceGridGUI.*;

class PlainServerEditor extends AbstractServerEditor
{
    @Override
    protected void writeDescriptor()
    {
        _subEditor.writeDescriptor();
    }

    @Override
    protected boolean isSimpleUpdate()
    {
        return _subEditor.isSimpleUpdate();
    }

    PlainServerEditor()
    {
        _subEditor = new ServerSubEditor(this);
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
        _subEditor.appendProperties(builder);
    }

    @Override
    Object getSubDescriptor()
    {
        return _target.getDescriptor();
    }

    @Override
    protected boolean validate()
    {
        return _subEditor.validate();
    }

    void show(PlainServer server)
    {
        detectUpdates(false);
        _target = server;

        _subEditor.show(true);

        _applyButton.setEnabled(server.isEphemeral());
        _discardButton.setEnabled(server.isEphemeral());
        detectUpdates(true);
        if(server.isEphemeral())
        {
            updated();
        }
    }

    private ServerSubEditor _subEditor;
}
