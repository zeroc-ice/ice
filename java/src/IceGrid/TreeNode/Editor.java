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
import javax.swing.JButton;

import javax.swing.event.DocumentEvent;
import javax.swing.event.DocumentListener;

//
// Base class for all editors
//
abstract class Editor
{
    protected Editor()
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
    
    protected void updated()
    {
	if(_detectUpdates)
	{
	    _applyButton.setEnabled(true);
	    _discardButton.setEnabled(true);
	}
    }
    
    protected void detectUpdates(boolean val)
    {
	_detectUpdates = val;
    }

    abstract protected CommonBase getTarget();

    abstract protected void applyUpdate();

    protected void discardUpdate()
    {
	getTarget().getModel().refreshDisplay();
    }

    protected JButton _applyButton;
    protected JButton _discardButton;
    protected DocumentListener _updateListener;

    private boolean _detectUpdates = true;   
}
