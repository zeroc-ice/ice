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

import IceGrid.Actions;
import IceGrid.AdapterDescriptor;
import IceGrid.Model;

class AdaptersActions extends Actions
{
    private class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    add(_newAdapter);
	    addPopupMenuListener(_popupListener);
	}
    }

    public JPopupMenu getPopupMenu()
    {
	if(_newAdapter.isEnabled())
	{
	    return _popup;
	}
	else
	{
	    return null;
	}
    }
    
    protected void paste()
    {
	_adapters.paste();
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu adaptersMenu = new JMenu("Adapters");
	adaptersMenu.setMnemonic(java.awt.event.KeyEvent.VK_A);
	menuBar.add(adaptersMenu);

	adaptersMenu.add(_newAdapter);
    }

    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_newAdapter);
    }

    AdaptersActions(Model model)
    {
	super(model);
	_newAdapter = new AbstractAction("New Adapter")
	    {
		public void actionPerformed(ActionEvent e)
		{
		    _adapters.newAdapter();
		}
	    };
	_newAdapter.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl N"));    

	_popup = new PopupMenu();
    }
    
    void reset(Adapters adapters)
    {
	_adapters = adapters;
	_copy.setEnabled(false);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof AdapterDescriptor)
	{
	    _paste.setEnabled(_adapters.canHaveNewChild());
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(false);

	_newAdapter.setEnabled(_adapters.canHaveNewChild());
    }

    Adapters _adapters;
    Action _newAdapter;
    PopupMenu _popup;
}
