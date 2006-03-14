// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JFrame;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.Utils;

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

    ServerEditor(Model model, JFrame parentFrame)
    {
	super(model);
	_subEditor = new ServerSubEditor(this, parentFrame);
    }
 
    //
    // From Editor:
    //
    Utils.Resolver getDetailResolver()
    {
	Server server = (Server)_target;
	if(server.getModel().substitute())
	{
	    return server.getResolver();
	}
	else
	{
	    return null;
	}
    }

    void appendProperties(DefaultFormBuilder builder)
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
	setTarget(server);

	_subEditor.show(true);

	_applyButton.setEnabled(server.isEphemeral());
	_discardButton.setEnabled(server.isEphemeral());	  
	detectUpdates(true);

	refreshCurrentStatus();
    }

    private ServerSubEditor _subEditor;
}
