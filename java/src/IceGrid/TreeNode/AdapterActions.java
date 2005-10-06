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
import IceGrid.AdapterDescriptor;
import IceGrid.Model;

class AdapterActions extends Actions
{
    AdapterActions(Model model)
    {
	super(model);
	_substituteTool = new JToggleButton(_model.getSubstituteVarAction());
	_substituteMenuItem = new JCheckBox(_model.getSubstituteVarAction());
    }
    
    void reset(Adapter adapter)
    {
	_adapter = adapter;
	_copy.setEnabled(true);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof AdapterDescriptor)
	{
	    Adapters adapters = (Adapters)_adapter.getParent();
	    _paste.setEnabled(adapters.canHaveNewChild());
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(_adapter.isEditable());

	if(_adapter.getResolver() != null && !_adapter.isEphemeral())
	{
	    _model.getSubstituteVarAction().setEnabled(true);
	    _substituteTool.setSelected(_model.substitute());
	    _substituteMenuItem.setSelected(_model.substitute());
	}
	else
	{
	    _model.getSubstituteVarAction().setEnabled(false);
	}
    }
    
    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_substituteTool);
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu adapterMenu = new JMenu("Adapter");
	adapterMenu.setMnemonic(java.awt.event.KeyEvent.VK_A);
	menuBar.add(adapterMenu);

	adapterMenu.add(_substituteMenuItem);
    }

    protected void copy()
    {
	_model.setClipboard(_adapter.copy());

	Adapters adapters = (Adapters)_adapter.getParent();
	_paste.setEnabled(adapters.canHaveNewChild());
    }

    protected void paste()
    {
	((Adapters)_adapter.getParent()).paste();
    }

    protected void delete()
    {
	TreePath parentPath = _adapter.getParent().getPath();
	_adapter.destroy();
	_model.setSelectionPath(parentPath);
    }

    Adapter _adapter;
    JCheckBox _substituteMenuItem;
    JToggleButton _substituteTool;
}
