// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.JTextField;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class ServiceSubEditor extends CommunicatorSubEditor
{
    ServiceSubEditor(Editor mainEditor)
    {
        super(mainEditor);

        _name.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _name.setToolTipText("Identifies this service within an IceBox server");

        _entry.getDocument().addDocumentListener(_mainEditor.getUpdateListener());
        _entry.setToolTipText(
            "<html>The service entry point and optional arguments.<br>"
            + "C++: <i>shared object:function-name arg1 arg2 ...</i><br>"
            + "Java: <i>class-name arg1 arg2 ...</i><br>"
            + "C#, Visual Basic: <i>assembly:class-name arg1 arg2 ...</i>"
            + "</html>");
    }

    ServiceDescriptor getServiceDescriptor()
    {
        return (ServiceDescriptor)_mainEditor.getSubDescriptor();
    }

    @Override
    void appendProperties(DefaultFormBuilder builder)
    {
        builder.append("Service Name");
        builder.append(_name, 3);
        builder.nextLine();

        //
        // Add Communicator fields
        //
        super.appendProperties(builder);

        builder.append("Entry Point");
        builder.append(_entry, 3);
        builder.nextLine();
    }

    void writeDescriptor()
    {
        ServiceDescriptor descriptor = getServiceDescriptor();
        descriptor.name = _name.getText().trim();
        descriptor.entry = _entry.getText().trim();
        super.writeDescriptor(descriptor);
    }

    boolean isSimpleUpdate()
    {
        return getServiceDescriptor().name.equals(_name.getText().trim());
    }

    boolean validate()
    {
        return _mainEditor.check(new String[]{
            "Service Name", _name.getText().trim(),
            "Entry Point", _entry.getText().trim()});
    }

    void show(boolean isEditable)
    {
        ServiceDescriptor descriptor = getServiceDescriptor();
        Utils.Resolver detailResolver = _mainEditor.getDetailResolver();

        isEditable = isEditable && (detailResolver == null);

        if(detailResolver != null)
        {
            _name.setText(detailResolver.find("service"));
        }
        else
        {
            _name.setText(descriptor.name);
        }
        _name.setEditable(isEditable);

        _entry.setText(Utils.substitute(descriptor.entry, detailResolver));
        _entry.setEditable(isEditable);

        show(descriptor, isEditable);
    }

    private JTextField _name = new JTextField(20);
    private JTextField _entry = new JTextField(20);
}
