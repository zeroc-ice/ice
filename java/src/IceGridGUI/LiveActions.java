// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.event.ActionEvent;
import javax.swing.AbstractAction;
import javax.swing.Action;

import IceGrid.*;
import IceGridGUI.LiveDeployment.*;

//
// Holds all actions for the "Live Deployment" view
//
public class LiveActions
{
    public Action get(int index)
    {
	return _array[index];
    }
    
    public boolean[] setTarget(TreeNode target)
    {
	_target = target;

	boolean[] availableActions;
	if(_target == null)
	{
	    availableActions = new boolean[TreeNode.ACTION_COUNT];
	}
	else
	{   
	    availableActions = _target.getAvailableActions();
	}

	for(int i = 0; i < _array.length; ++i)
	{
	    _array[i].setEnabled(availableActions[i]);
	}

	return availableActions;
    }

    LiveActions()
    {
	_array[TreeNode.START] = new AbstractAction(
	    "Start", Utils.getIcon("/icons/16x16/start.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.start();
		}
	    };
	_array[TreeNode.START].putValue(Action.SHORT_DESCRIPTION, 
					"Start this server");

	_array[TreeNode.STOP] = new AbstractAction(
	    "Stop", Utils.getIcon("/icons/16x16/stop.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.stop();
		}
	    };
	_array[TreeNode.STOP].putValue(Action.SHORT_DESCRIPTION, 
					    "Stop this server");
	

	_array[TreeNode.ENABLE] = new AbstractAction(
	    "Enable", Utils.getIcon("/icons/16x16/enable.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.enable();
		}
	    };
	_array[TreeNode.ENABLE].putValue(Action.SHORT_DESCRIPTION, 
					     "Enable this server");
	
	_array[TreeNode.DISABLE] = new AbstractAction(
	    "Disable", Utils.getIcon("/icons/16x16/disable.png"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.disable();
		}
	    };
	_array[TreeNode.DISABLE].putValue(Action.SHORT_DESCRIPTION, 
					      "Disable this server");


	_array[TreeNode.SHUTDOWN_NODE] = new AbstractAction("Shutdown")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.shutdownNode();
		}
	    };
	
	_array[TreeNode.SERVER_INSTALL_DISTRIBUTION] = 
	    new AbstractAction("Patch")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.serverInstallDistribution();
		}
	    };
    }

    private TreeNode _target;
    private Action[] _array = new Action[TreeNode.ACTION_COUNT];
}
