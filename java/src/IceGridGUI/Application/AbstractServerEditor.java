// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import javax.swing.JOptionPane;
import javax.swing.JTextField;
import javax.swing.JToolBar;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import com.jgoodies.forms.builder.DefaultFormBuilder;

import IceGrid.*;
import IceGridGUI.*;

//
// Base class for ServerEditor and ServerInstanceEditor
//

abstract class AbstractServerEditor extends Editor
{
    abstract protected void writeDescriptor();
    abstract protected boolean isSimpleUpdate();

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	_propertiesPanel.setName("Server Properties");
    }

    protected void applyUpdate()
    {
	Root root = _target.getRoot();
	Server server = (Server)_target;

	root.disableSelectionListener();
	try
	{
	    if(_target.isEphemeral())
	    {
		Node node = (Node)_target.getParent();
		writeDescriptor();
		ServerInstanceDescriptor instanceDescriptor =
		    server.getInstanceDescriptor();
		ServerDescriptor serverDescriptor = 
		    server.getServerDescriptor();
		
		_target.destroy(); // just removes the child
		
		try
		{
		    node.tryAdd(instanceDescriptor, serverDescriptor, true);
		}
		catch(UpdateFailedException e)
		{
		    //
		    // Add back ephemeral child
		    //
		    try
		    {
			node.insertServer(_target, true);
		    }
		    catch(UpdateFailedException die)
		    {
			assert false;
		    }
		    root.setSelectedNode(_target);

		    JOptionPane.showMessageDialog(
			root.getCoordinator().getMainFrame(),
			e.toString(),
			"Apply failed",
			JOptionPane.ERROR_MESSAGE);
		    return;
		}

		//
		// Success
		//
		if(instanceDescriptor != null)
		{
		    _target = (Server)node.findChildWithDescriptor(instanceDescriptor);
		}
		else
		{
		    _target = (Server)node.findChildWithDescriptor(serverDescriptor);
		}
		root.updated();
		root.setSelectedNode(_target);
	    }
	    else if(isSimpleUpdate())
	    {
		writeDescriptor();
		root.updated();
		server.getEditable().markModified();
	    }
	    else
	    {
		//
		// Save to be able to rollback
		//
		Object savedDescriptor = ((Communicator)_target).saveDescriptor();
		Node node = (Node)_target.getParent();
		writeDescriptor();
		
		ServerInstanceDescriptor instanceDescriptor =
		    server.getInstanceDescriptor();
		ServerDescriptor serverDescriptor = 
		    server.getServerDescriptor();
		
		node.removeServer(_target);
		
		try
		{
		    node.tryAdd(instanceDescriptor, serverDescriptor, false);
		}
		catch(UpdateFailedException e)
		{
		    //
		    // Restore
		    //	
		    try
		    {
			node.insertServer(_target, true);
		    }
		    catch(UpdateFailedException die)
		    {
			assert false;
		    }
		    ((Communicator)_target).restoreDescriptor(savedDescriptor);
		    root.setSelectedNode(_target);
		    
		    JOptionPane.showMessageDialog(
			root.getCoordinator().getMainFrame(),
			e.toString(),
			"Apply failed",
			JOptionPane.ERROR_MESSAGE);
		    return;
		}
		
		//
		// Success
		//
		node.getEditable().removeElement(_target.getId(), 
						 Server.class); // replaced by brand new Server
		
		if(instanceDescriptor != null)
		{
		    _target = node.findChildWithDescriptor(instanceDescriptor);
		}
		else
		{
		    _target = node.findChildWithDescriptor(serverDescriptor);
		}
		root.updated();
		root.setSelectedNode(_target);
	    }
	    
	    root.getCoordinator().getCurrentTab().showNode(_target);
	    _applyButton.setEnabled(false);
	    _discardButton.setEnabled(false);
	}
	finally
	{
	    root.enableSelectionListener();
	}
    }
}
