// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.BorderLayout;
import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JButton;
import javax.swing.JComponent;
import javax.swing.JLabel;
import javax.swing.JPanel;
import javax.swing.JScrollPane;
import javax.swing.JToolBar;

import javax.swing.tree.TreePath;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

import com.jgoodies.forms.builder.DefaultFormBuilder;
import com.jgoodies.forms.factories.Borders;
import com.jgoodies.forms.factories.ButtonBarFactory;
import com.jgoodies.forms.factories.DefaultComponentFactory;
import com.jgoodies.forms.layout.FormLayout;
import com.jgoodies.forms.util.LayoutStyle;

import IceGrid.*;
import IceGridGUI.*;

public class Editor extends EditorBase
{      
    protected void applyUpdate()
    {
	assert false;
    }
    
    protected void detectUpdates(boolean val)
    {
	_detectUpdates = val;
    }

    protected void discardUpdate()
    {
	if(_target.isEphemeral())
	{
	    _target.delete();
	}
	else
	{
	    _target.getCoordinator().getCurrentTab().showNode(_target);
	}
    }

    protected void appendProperties(DefaultFormBuilder builder)
    {}

    protected void buildPropertiesPanel()
    {
	super.buildPropertiesPanel();
	JComponent buttonBar = 
	    ButtonBarFactory.buildRightAlignedBar(_applyButton, 
						  _discardButton);
	buttonBar.setBorder(Borders.DIALOG_BORDER);
	_propertiesPanel.add(buttonBar, BorderLayout.SOUTH);
    }


    Editor()
    {
	//
	// _applyButton
	//
	AbstractAction apply = new AbstractAction("Apply")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    applyUpdate();
		}
	    };
	_applyButton = new JButton(apply);
	_applyButton.setEnabled(false);
	
	//
	// _discardButton
	//
	AbstractAction discard = new AbstractAction("Discard")
	    {
		public void actionPerformed(ActionEvent e) 
		{
		    discardUpdate();
		}
	    };
	_discardButton = new JButton(discard);
	_discardButton.setEnabled(false);
	
	_updateListener = new DocumentListener() 
	    {
		public void changedUpdate(DocumentEvent e)
		{
		    updated();
		}
		
		public void insertUpdate(DocumentEvent e)
		{
		    updated();
		}
		
		public void removeUpdate(DocumentEvent e)
		{
		    updated();
		}
	    };
    }

    //
    // Used by the sub-editor (when there is one)
    //
    Object getSubDescriptor()
    {
	return null;
    }

    Utils.Resolver getDetailResolver()
    {
	return null;
    }

    TreeNode getTarget()
    {
	return _target;
    }
    
    void updated()
    {
	if(_detectUpdates)
	{
	    _target.getRoot().disableRegistryUpdates();
	    _applyButton.setEnabled(true);
	    _discardButton.setEnabled(true);
	}
    }

    DocumentListener getUpdateListener()
    {
	return _updateListener;
    }
    
    
    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;
   
    protected TreeNode _target;
    private boolean _detectUpdates = true;   
}
