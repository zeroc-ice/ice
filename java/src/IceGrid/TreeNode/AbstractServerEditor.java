// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;

import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.ServerInstanceDescriptor;

//
// Base class for ServerEditor and ServerInstanceEditor
//

abstract class AbstractServerEditor extends Editor
{
    abstract protected void writeDescriptor();
    abstract protected boolean isSimpleUpdate();

    protected void applyUpdate()
    {
	Server server = (Server)_target;
	Model model = server.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(server.isEphemeral())
		{
		    Node node = (Node)server.getParent();
		    writeDescriptor();
		    ServerInstanceDescriptor instanceDescriptor =
			server.getInstanceDescriptor();
		    ServerDescriptor serverDescriptor = 
			server.getServerDescriptor();

		    server.destroy(); // just removes the child
		    
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
			    node.addChild(server, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(server.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
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
			_target = node.findChildWithDescriptor(instanceDescriptor);
		    }
		    else
		    {
			_target = node.findChildWithDescriptor(serverDescriptor);
		    }
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		    //
		    // If this new server has a known state, display it
		    //
		    ((Server)_target).fireNodeChangedEvent(this);

		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		    _target.getEditable().markModified();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = server.saveDescriptor();
		    Node node = (Node)server.getParent();
		    writeDescriptor();
		    
		    ServerInstanceDescriptor instanceDescriptor =
			server.getInstanceDescriptor();
		    ServerDescriptor serverDescriptor = 
			server.getServerDescriptor();

		    node.removeChild(server, true);
		    
		    try
		    {
			node.tryAdd(instanceDescriptor, serverDescriptor, false);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Restore all
			//
			server.restoreDescriptor(savedDescriptor);
			
			try
			{
			    node.addChild(server, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			model.setSelectionPath(server.getPath());

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }
		    
		    //
		    // Success
		    //
		    node.removeElement(server.getId()); // replaced by brand new Server
		    
		    if(instanceDescriptor != null)
		    {
			_target = node.findChildWithDescriptor(instanceDescriptor);
		    }
		    else
		    {
			_target = node.findChildWithDescriptor(serverDescriptor);
		    }
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);

		    //
		    // If this server has a known state, display it
		    //
		    ((Server)_target).fireNodeChangedEvent(this);

		}

		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	}
    }


    protected AbstractServerEditor()
    {}
}
