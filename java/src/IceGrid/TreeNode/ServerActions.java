// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JCheckBox;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPopupMenu;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;
import javax.swing.tree.TreePath;

import IceGrid.Actions;
import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;


class ServerActions extends Actions
{
    private class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    add(_start);
	    add(_stop);
	    add(_enable);
	    add(_disable);
	    addPopupMenuListener(_popupListener);
	}
    }

    public JPopupMenu getPopupMenu()
    {
	return _popup;
    }

    ServerActions(Model model)
    {
	super(model);
	_substituteTool = new JToggleButton(_model.getSubstituteVarAction());
	_substituteMenuItem = new JCheckBox(_model.getSubstituteVarAction());

	_start = new AbstractAction("Start")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.start();
		}
	    };
	
	_stop = new AbstractAction("Stop")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.stop();
		}
	    };
	
	_enable = new AbstractAction("Enable")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.enable();
		}
	    };
	
	_disable = new AbstractAction("Disable")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _server.disable();
		}
	    };

	_popup = new PopupMenu();
    }
    
    void reset(Server server)
    {
	_server = server;
	_copy.setEnabled(true);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && (clipboard instanceof ServerDescriptor
				 || clipboard instanceof ServerInstanceDescriptor))
	{
	    _paste.setEnabled(true);
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(true);

	if(!_server.isEphemeral())
	{
	    _model.getSubstituteVarAction().setEnabled(true);
	    _substituteTool.setSelected(_model.substitute());
	    _substituteMenuItem.setSelected(_model.substitute());
	}
	else
	{
	    _model.getSubstituteVarAction().setEnabled(false);
	}
		
	_start.setEnabled(_server.getState() == ServerState.Inactive 
			  && _server.isEnabled());
	_stop.setEnabled(_server.getState() != ServerState.Inactive);

	_enable.setEnabled(!_server.isEnabled());
	_disable.setEnabled(server.isEnabled());
    }
    
    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_substituteTool);
	toolBar.addSeparator();
	toolBar.add(_start);
	toolBar.add(_stop);
	toolBar.addSeparator();
	toolBar.add(_enable);
	toolBar.add(_disable);
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu serverMenu = new JMenu("Server");
	serverMenu.setMnemonic(java.awt.event.KeyEvent.VK_S);
	menuBar.add(serverMenu);

	serverMenu.add(_substituteMenuItem);
	serverMenu.addSeparator();
	serverMenu.add(_start);
	serverMenu.add(_stop);
	serverMenu.addSeparator();
	serverMenu.add(_enable);
	serverMenu.add(_disable);
    }

    protected void copy()
    {
	_model.setClipboard(_server.copy());
	_paste.setEnabled(true);
    }

    protected void paste()
    {
	((Node)_server.getParent()).paste();
    }

    protected void delete()
    {
	TreePath parentPath = _server.getParent().getPath();
	_server.destroy();
	_model.setSelectionPath(parentPath);
    }

    Action _start;
    Action _stop;
    Action _enable;
    Action _disable;

    Server _server;
    JCheckBox _substituteMenuItem;
    JToggleButton _substituteTool;
    
    PopupMenu _popup;
}
