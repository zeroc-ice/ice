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

import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPopupMenu;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;

import IceGrid.Actions;
import IceGrid.Model;
import IceGrid.ServerDescriptor;
import IceGrid.TemplateDescriptor;


class ServerTemplatesActions extends Actions
{
    private class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    add(_newServerTemplate);
	    add(_newIceBoxTemplate);
	    addPopupMenuListener(_popupListener);
	}
    }

    public JPopupMenu getPopupMenu()
    {
	return _popup;
    }
    
    protected void paste()
    {
	_templates.paste();
    }

    protected void addMenu(JMenuBar menuBar)
    {
	JMenu templatesMenu = new JMenu("Templates");
	templatesMenu.setMnemonic(java.awt.event.KeyEvent.VK_T);
	menuBar.add(templatesMenu);

	templatesMenu.add(_newServerTemplate);
	templatesMenu.add(_newIceBoxTemplate);
    }

    protected void addTools(JToolBar toolBar)
    {
	toolBar.add(_newServerTemplate);
	toolBar.add(_newIceBoxTemplate);
    }

    ServerTemplatesActions(Model model)
    {
	super(model);

	_newServerTemplate = new AbstractAction("New server template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _templates.newServerTemplate();
		}
	    };
	_newServerTemplate.putValue(Action.ACCELERATOR_KEY, 
				    KeyStroke.getKeyStroke("ctrl N"));   

	_newIceBoxTemplate = new AbstractAction("New IceBox template")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    _templates.newIceBoxTemplate();
		}
	    };

	_popup = new PopupMenu();
    }
    
    void reset(ServerTemplates templates)
    {
	_templates = templates;
	_copy.setEnabled(false);
	Object clipboard = _model.getClipboard();
	if(clipboard != null && clipboard instanceof TemplateDescriptor)
	{
	    TemplateDescriptor d = (TemplateDescriptor)clipboard;
	    _paste.setEnabled(d.descriptor instanceof ServerDescriptor);
	}
	else
	{
	    _paste.setEnabled(false);
	}
	_delete.setEnabled(false);
    }

    ServerTemplates _templates;
    Action _newServerTemplate;
    Action _newIceBoxTemplate;
    PopupMenu _popup;
}
