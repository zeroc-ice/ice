// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.SwingUtilities;
import javax.swing.JOptionPane;

import IceGrid.*;
import IceGridGUI.*;

public abstract class TreeNode extends TreeNodeBase
{
    public abstract Editor getEditor();

    TreeNode(TreeNode parent, String id)
    {
	super(parent, id);
    }

    Root getRoot()
    {
	assert _parent != null;
	return ((TreeNode)_parent).getRoot();
    }

    //
    // Actions
    //
    public static final int START = 0;
    public static final int STOP = 1;
    public static final int ENABLE = 2;
    public static final int DISABLE = 3;

    public static final int SHUTDOWN_NODE = 4;
    public static final int PATCH_SERVER = 5;

    public static final int ADD_OBJECT = 6;

    static public final int ACTION_COUNT = 7;
    
    public boolean[] getAvailableActions()
    {
	return new boolean[ACTION_COUNT];
    }

    public void start()
    {
	assert false;
    }
    public void stop()
    {
	assert false;
    }
    public void enable()
    {
	assert false;
    }
    public void disable()
    {
	assert false;
    }
    public void shutdownNode()
    {
	assert false;
    }
    public void patchServer()
    {
	assert false;
    }
    public void addObject()
    {
	assert false;
    }

    //
    // Helpers
    //
    protected void amiSuccess(final String prefix)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    getCoordinator().getStatusBar().setText(prefix + "done.");
		}
	    });
    }

    protected void amiFailure(String prefix, String title, Ice.UserException e)
    {
	if(e instanceof IceGrid.ServerNotExistException)
	{
	    IceGrid.ServerNotExistException sne =
		(IceGrid.ServerNotExistException)e;

	    amiFailure(prefix, title, 
		       "Server '" + sne.id + 
		       "' was not registered with the IceGrid Registry");
	}
	else if(e instanceof IceGrid.ServerStartException)
	{
	    IceGrid.ServerStartException ste = (IceGrid.ServerStartException)e;
	    amiFailure(prefix, title, "Server '" +
		       ste.id + "' did not start: " + ste.reason);
	}
	else if(e instanceof IceGrid.ApplicationNotExistException)
	{
	    amiFailure(prefix, title, 
		       "This application was not registered with the IceGrid Registry");
	}
	else if(e instanceof IceGrid.PatchException)
	{
	    IceGrid.PatchException pe = (IceGrid.PatchException)e;
	    //
	    // TODO: XXX: FIXME PatchException now contains a list of reasons
	    //
	    amiFailure(prefix, title, "");
	}
	else if(e instanceof IceGrid.NodeNotExistException)
	{
	    IceGrid.NodeNotExistException nnee = 
		(IceGrid.NodeNotExistException)e;

	    amiFailure(prefix, title, 
		       "Node '" + nnee.name + 
		       " 'was not registered with the IceGrid Registry.");
	}
	else if(e instanceof IceGrid.NodeUnreachableException)
	{
	    IceGrid.NodeUnreachableException nue =
		(IceGrid.NodeUnreachableException)e;
	    amiFailure(prefix, title, "Node '" +
		       nue.name + "' is unreachable: " + nue.reason);
	}
	else if(e instanceof IceGrid.DeploymentException)
	{
	    IceGrid.DeploymentException de = (IceGrid.DeploymentException)e;
	    amiFailure(prefix, title, "Deployment exception: " + de.reason);
	}
	else
	{
	    amiFailure(prefix, title, e.toString());
	}
    }
    
    protected void amiFailure(final String prefix, 
			      final String title, final String message)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {	
		public void run() 
		{
		    failure(prefix, title, message);
		}
	    });
    }

    protected void failure(String prefix, String title, String message)
    {
	getCoordinator().getStatusBar().setText(prefix + "failed!");

	JOptionPane.showMessageDialog(
	    getCoordinator().getMainFrame(),
	    message,
	    title,
	    JOptionPane.ERROR_MESSAGE);
    }  
}
