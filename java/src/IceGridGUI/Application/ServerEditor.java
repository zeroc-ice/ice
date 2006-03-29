// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JFrame;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

class ServerEditor extends AbstractServerEditor
{
    protected void writeDescriptor()
    {
	_subEditor.writeDescriptor();
    }	    
    
    protected boolean isSimpleUpdate()
    {
	return _subEditor.isSimpleUpdate(); 
    }

    ServerEditor(JFrame parentFrame)
    {
	_subEditor = new ServerSubEditor(this, parentFrame);
    }
 
    //
    // From Editor:
    //
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

    protected void appendProperties(DefaultFormBuilder builder)
    {    
	_subEditor.appendProperties(builder);
    }
    
    Object getSubDescriptor()
    {
	return ((Server)_target).getServerDescriptor();
    }

    void show(Server server)
    {
	detectUpdates(false);
	_target = server;

	_subEditor.show(true);

	_applyButton.setEnabled(server.isEphemeral());
	_discardButton.setEnabled(server.isEphemeral());	  
	detectUpdates(true);
    }

    private ServerSubEditor _subEditor;
}
