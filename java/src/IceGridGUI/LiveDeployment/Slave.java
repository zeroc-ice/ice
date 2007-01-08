// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import IceGrid.*;
import IceGridGUI.*;


class Slave extends TreeNode
{
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[SHUTDOWN_REGISTRY] = true;
	return actions;
    }

    public void shutdownRegistry()
    {
	final String prefix = "Shutting down registry '" + _id + "'...";
	getCoordinator().getStatusBar().setText(prefix);

	AMI_Admin_shutdownRegistry cb = new AMI_Admin_shutdownRegistry()
	    {
		//
		// Called by another thread!
		//
		public void ice_response()
		{
		    amiSuccess(prefix);
		}
		
		public void ice_exception(Ice.UserException e)
		{
		    amiFailure(prefix, "Failed to shutdown " + _id, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to shutdown " + _id, 
			       e.toString());
		}
	    };

	try
	{   
	    getCoordinator().getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    
	    getCoordinator().getAdmin().shutdownRegistry_async(cb, _id);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to shutdown " + _id, e.toString());
	}
	finally
	{
	    getCoordinator().getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }

    public JPopupMenu getPopupMenu()
    {
	LiveActions la = getCoordinator().getLiveActionsForPopup();

	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(la.get(SHUTDOWN_REGISTRY));
	}
	
	la.setTarget(this);
	return _popup;
    }


    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new SlaveEditor();
	}
	_editor.show(_info);
	return _editor;
    }

    public Component getTreeCellRendererComponent(
	JTree tree,
	Object value,
	boolean sel,
	boolean expanded,
	boolean leaf,
	int row,
	boolean hasFocus) 
    {
	if(_cellRenderer == null)
	{
	    //
	    // TODO: separate icon for master
	    //

	    _cellRenderer = new DefaultTreeCellRenderer();
	    _cellRenderer.setLeafIcon(Utils.getIcon("/icons/16x16/registry.png"));
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    Slave(TreeNode parent, RegistryInfo info)
    {
	super(parent, info.name);
	_info = info;
    }   

    private RegistryInfo _info;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private SlaveEditor _editor;
    static private JPopupMenu _popup;
}
