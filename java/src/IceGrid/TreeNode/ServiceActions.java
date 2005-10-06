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
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Model;

class ServiceActions extends Actions
{
    ServiceActions(Model model)
    {
	super(model);
	_substituteTool = new JToggleButton(_model.getSubstituteVarAction());
	_substituteMenuItem = new JCheckBox(_model.getSubstituteVarAction());

	_moveUp = new AbstractAction("Up")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _service.moveUp();
		}
	    };
	
	_moveDown = new AbstractAction("Down")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _service.moveDown();
		}
	    };
    }
    
    void reset(Service service)
    {
	_service = service;
	_copy.setEnabled(true);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof ServiceInstanceDescriptor)
	{
	    Services services = (Services)_service.getParent();
	    _paste.setEnabled(services.isEditable());
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(_service.isEditable());

	if(_service.getResolver() != null && !_service.isEphemeral())
	{
	    _model.getSubstituteVarAction().setEnabled(true);
	    _substituteTool.setSelected(_model.substitute());
	    _substituteMenuItem.setSelected(_model.substitute());
	}
	else
	{
	    _model.getSubstituteVarAction().setEnabled(false);
	}
		
	_moveUp.setEnabled(_service.canMoveUp());
	_moveDown.setEnabled(_service.canMoveDown());
    }
    
    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_substituteTool);
	toolBar.addSeparator();
	toolBar.add(_moveUp);
	toolBar.add(_moveDown);
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu serviceMenu = new JMenu("Service");
	serviceMenu.setMnemonic(java.awt.event.KeyEvent.VK_S);
	menuBar.add(serviceMenu);

	serviceMenu.add(_substituteMenuItem);
	serviceMenu.addSeparator();
	serviceMenu.add(_moveUp);
	serviceMenu.add(_moveDown);
    }

    protected void copy()
    {
	_model.setClipboard(_service.copy());

	Services services = (Services)_service.getParent();
	_paste.setEnabled(services.isEditable());
    }

    protected void paste()
    {
	((Services)_service.getParent()).paste();
    }

    protected void delete()
    {
	TreePath parentPath = _service.getParent().getPath();
	_service.destroy();
	_model.setSelectionPath(parentPath);
    }

    
    Action _moveUp;
    Action _moveDown;

    Service _service;
    JCheckBox _substituteMenuItem;
    JToggleButton _substituteTool;
}
