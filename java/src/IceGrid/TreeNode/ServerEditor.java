// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
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

    ServerEditor(JFrame parentFrame)
    {
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

    void append(DefaultFormBuilder builder)
    {    
	_subEditor.append(builder);
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
    }

    private ServerSubEditor _subEditor;
}
