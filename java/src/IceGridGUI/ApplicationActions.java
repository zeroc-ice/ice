// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.awt.Toolkit;
import java.awt.event.ActionEvent;
import java.awt.event.KeyEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.KeyStroke;

import IceGrid.*;
import IceGridGUI.Application.*;

//
// Holds all actions for the Application view
//
public class ApplicationActions
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

    ApplicationActions(boolean popup)
    {
	final int MENU_MASK = Toolkit.getDefaultToolkit().getMenuShortcutKeyMask();

	_array[TreeNode.NEW_ADAPTER] = new AbstractAction(
	    popup ? "New adapter" : "Adapter")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newAdapter();
		}
	    };

	_array[TreeNode.NEW_DBENV] = new AbstractAction(
	    popup ? "New database environment" : "Database environment")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newDbEnv();
		}
	    };
	
	_array[TreeNode.NEW_NODE] = new AbstractAction(
	    popup ? "New node" : "Node")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newNode();
		}
	    };

	_array[TreeNode.NEW_REPLICA_GROUP] = new AbstractAction(
	    popup ? "New replica group" : "Replica group" )
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newReplicaGroup();
		}
	    };

	_array[TreeNode.NEW_SERVER] = new AbstractAction(
	    popup ? "New server" : "Server")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newServer();
		}
	    };

	_array[TreeNode.NEW_SERVER_ICEBOX] = new AbstractAction(
	    popup ? "New IceBox server" : "IceBox server")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newServerIceBox();
		}
	    };

	_array[TreeNode.NEW_SERVER_FROM_TEMPLATE] = new AbstractAction(
	    popup ? "New server from template" : "Server from template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newServerFromTemplate();
		}
	    };


	_array[TreeNode.NEW_SERVICE] = new AbstractAction(
	    popup ? "New service" : "Service")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newService();
		}
	    };

	_array[TreeNode.NEW_SERVICE_FROM_TEMPLATE] = new AbstractAction(
	    popup ? "New service from template" : "Service from template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newServiceFromTemplate();
		}
	    };

	_array[TreeNode.NEW_TEMPLATE_SERVER] = new AbstractAction(
	    popup ? "New Server template" : "Server template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newTemplateServer();
		}
	    };
	
	_array[TreeNode.NEW_TEMPLATE_SERVER_ICEBOX] = new AbstractAction(
	    popup ? "New IceBox server template" : "IceBox server template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newTemplateServerIceBox();
		}
	    };


	_array[TreeNode.NEW_TEMPLATE_SERVICE] = new AbstractAction(
	    popup ? "New service template" : "Service template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.newTemplateService();
		}
	    };
	
	_array[TreeNode.COPY] = new AbstractAction("Copy")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.copy();
		}
	    };
	_array[TreeNode.COPY].putValue(Action.ACCELERATOR_KEY, 
				       KeyStroke.getKeyStroke(KeyEvent.VK_C, MENU_MASK));
	_array[TreeNode.COPY].putValue(Action.SHORT_DESCRIPTION, "Copy");
	
	_array[TreeNode.PASTE] = new AbstractAction("Paste")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.paste();
		}
	    };
	_array[TreeNode.PASTE].putValue(Action.ACCELERATOR_KEY, 
					KeyStroke.getKeyStroke(KeyEvent.VK_V, MENU_MASK));
	_array[TreeNode.PASTE].putValue(Action.SHORT_DESCRIPTION, "Paste");
	
	_array[TreeNode.DELETE] = new AbstractAction("Delete")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.delete();
		}
	    };
	_array[TreeNode.DELETE].putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));
	_array[TreeNode.DELETE].putValue(Action.SHORT_DESCRIPTION, "Delete");
   
	_array[TreeNode.SHOW_VARS] = new 
	    AbstractAction("Show variables")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.getCoordinator().showVars();
		}
	    };
	_array[TreeNode.SHOW_VARS].putValue(
	    Action.SHORT_DESCRIPTION, 
	    "Show variables and parameters in the Properties pane");


	_array[TreeNode.SUBSTITUTE_VARS] = new 
	    AbstractAction("Substitute variables")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.getCoordinator().substituteVars();
		}
	    };
	
	_array[TreeNode.SUBSTITUTE_VARS].putValue(
	    Action.SHORT_DESCRIPTION, 
	    "Substitute variables and parameters with their values in the Properties pane");

	
	_array[TreeNode.MOVE_UP] = new AbstractAction("Move up")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.moveUp();
		}
	    };
			   
	_array[TreeNode.MOVE_DOWN] = new AbstractAction("Move down")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _target.moveDown();
		}
	    };
    }

    private TreeNode _target;
    private Action[] _array = new Action[TreeNode.ACTION_COUNT];
}
