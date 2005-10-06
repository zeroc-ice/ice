// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JMenu;
import javax.swing.JMenuBar;
import javax.swing.JPopupMenu;
import javax.swing.JToggleButton;
import javax.swing.JToolBar;
import javax.swing.KeyStroke;

import javax.swing.event.PopupMenuEvent;
import javax.swing.event.PopupMenuListener;

import com.jgoodies.looks.Options;
import com.jgoodies.looks.HeaderStyle;
import com.jgoodies.looks.BorderStyle;
import com.jgoodies.looks.plastic.PlasticLookAndFeel;

import IceGrid.TreeNode.CommonBase;

//
// The base class for all actions; provides the corresponding menu bar, tool bar and popup-menu. 
//

public class Actions
{
    private class ToolBar extends JToolBar
    {
	private ToolBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);
	    setFloatable(false);
	    putClientProperty("JToolBar.isRollover", Boolean.TRUE);
	    
	    _model.addTools(this);

	    addSeparator();
	    add(_copy);
	    add(_paste);
	    addSeparator();
	    add(_delete);
	    addSeparator();

	    addTools(this);
	}
    }
    
    private class MenuBar extends JMenuBar
    {
	private MenuBar()
	{
	    putClientProperty(Options.HEADER_STYLE_KEY, HeaderStyle.BOTH);
	    putClientProperty(PlasticLookAndFeel.BORDER_STYLE_KEY, BorderStyle.SEPARATOR);

	    _model.addFileMenu(this);
	   
	    //
	    // Edit menu
	    //
	    JMenu editMenu = new JMenu("Edit");
	    editMenu.setMnemonic(java.awt.event.KeyEvent.VK_E);
	    add(editMenu);
	    editMenu.add(_copy);
	    editMenu.add(_paste);
	    editMenu.addSeparator();
	    editMenu.add(_delete);

	    addMenu(this);
	    _model.addHelpMenu(this);
	}
    }

    public Actions(Model model)
    {
	_model = model;

	_copy = new AbstractAction("Copy", Utils.getIcon("/icons/copy_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    copy();
		}
	    };
	_copy.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl C"));

	_paste = new AbstractAction("Paste", Utils.getIcon("/icons/paste_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		   paste();
		}
	    };
	_paste.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("ctrl V"));


	_delete = new AbstractAction("Delete", Utils.getIcon("/icons/delete_edit.gif"))
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    delete();
		}
	    };
	_delete.putValue(Action.ACCELERATOR_KEY, KeyStroke.getKeyStroke("DELETE"));
	_copy.setEnabled(false);
	_paste.setEnabled(false);
	_delete.setEnabled(false);

	_popupListener = new PopupMenuListener()
	    {
		public void popupMenuCanceled(PopupMenuEvent e)
		{
		}
		  
		public void popupMenuWillBecomeInvisible(PopupMenuEvent e)
		{
		    //
		    // Undo the getActions() that opened this menu without
		    // selecting the node
		    //
		    CommonBase currentNode = _model.getSelectedNode();
		    if(currentNode != null)
		    {
			currentNode.getActions();
		    }
		    else
		    {
			_model.setActions(_model.getDefaultActions());
		    }
		}
          
		public void popupMenuWillBecomeVisible(PopupMenuEvent e)
		{
		    
		}
	    };
    }

    public final JToolBar getToolBar()
    {
	if(_toolBar == null)
	{
	    _toolBar = new ToolBar();
	}
	return _toolBar;
    }

    public final JMenuBar getMenuBar()
    {
	if(_menuBar == null)
	{
	    _menuBar = new MenuBar();
	}
	return _menuBar;
    }

    public JPopupMenu getPopupMenu()
    {
	//
	// By default, no popup menu
	//
	return null;
    }

    //
    // Access to some actions
    //
    public Action getCopyAction()
    {
	return _copy;
    }

    public Action getPasteAction()
    {
	return _paste;
    }

    public Action getDeleteAction()
    {
	return _delete;
    }
    
    protected void addTools(JToolBar toolBar)
    {
	// That's it for the base tool bar.
    }

    protected void addMenu(JMenuBar menuBar)
    {
	// No private menu for the base menu bar
    }
    

    protected void copy()
    {
	//
	// Corresponding action must be disabled is copy is not overridden.
	//
	assert false;
    }
    
    protected void paste()
    {
	assert false;
    }
    
    protected void delete()
    {
	assert false;
    }
    
    protected Model _model;

    protected Action _copy;
    protected Action _paste;
    protected Action _delete;

    protected PopupMenuListener _popupListener;

    private MenuBar _menuBar;
    private ToolBar _toolBar;
 }
