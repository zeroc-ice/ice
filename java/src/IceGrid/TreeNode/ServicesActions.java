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
import IceGrid.ServiceDescriptor;
import IceGrid.ServiceInstanceDescriptor;
import IceGrid.Model;

class ServicesActions extends Actions
{
    private class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    add(_newService);
	    add(_newInstance);
	    addPopupMenuListener(_popupListener);
	}
    }

    public JPopupMenu getPopupMenu()
    {
	if(_newService.isEnabled())
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
	_services.paste();
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu servicesMenu = new JMenu("Services");
	servicesMenu.setMnemonic(java.awt.event.KeyEvent.VK_S);
	menuBar.add(servicesMenu);

	servicesMenu.add(_newService);
	servicesMenu.add(_newInstance);
    }

    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_newService);
	toolBar.add(_newInstance);
    }

    ServicesActions(Model model)
    {
	super(model);

	_newService = new AbstractAction("New service")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    ServiceDescriptor sd = 
			new ServiceDescriptor(new java.util.LinkedList(),
					      new java.util.LinkedList(),
					      new java.util.LinkedList(),
					      "",
					      "NewService",
					      "");
		    
		    ServiceInstanceDescriptor descriptor = 
			new ServiceInstanceDescriptor("",
						      new java.util.TreeMap(),
						      sd);
		    _services.newService(descriptor);
		}
	    };
	_newService.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl N"));   
	 
	_newInstance = new AbstractAction("New template instance")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    ServiceInstanceDescriptor descriptor = 
			new ServiceInstanceDescriptor("",
						      new java.util.TreeMap(),
						      null);
		    _services.newService(descriptor);
		}
	    };

	_popup = new PopupMenu();
    }
    
    void reset(Services services)
    {
	_services = services;
	_copy.setEnabled(false);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof ServiceInstanceDescriptor)
	{
	    _paste.setEnabled(_services.isEditable());
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(false);

	_newService.setEnabled(_services.isEditable());
	_newInstance.setEnabled(_services.isEditable());
    }

    Services _services;
    Action _newService;
    Action _newInstance;
    PopupMenu _popup;
}
