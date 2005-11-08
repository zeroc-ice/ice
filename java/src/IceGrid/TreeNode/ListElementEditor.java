// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;

import IceGrid.Model;

abstract class ListElementEditor extends Editor
{  
    abstract void writeDescriptor();
    abstract boolean isSimpleUpdate();
    void postUpdate() {}

    protected ListElementEditor(boolean hasCurrentStatus)
    {
	super(hasCurrentStatus, true);
    }
    
    //
    // Update when parent is a ListParent
    //
    protected void applyUpdate()
    {
	Model model = _target.getModel();

	if(model.canUpdate())
	{    
	    boolean refreshDisplay = false;
	    model.disableDisplay();

	    try
	    {
		if(_target.isEphemeral())
		{
		    ListParent parent = (ListParent)_target.getParent();
		    writeDescriptor();
		    Object descriptor = _target.getDescriptor();
		    _target.destroy(); // just removes the child
		    
		    try
		    {
			parent.tryAdd(descriptor);
		    }
		    catch(UpdateFailedException e)
		    {
			//
			// Restore ephemeral
			//
			try
			{
			    parent.addChild(_target, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}

			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);

			model.setSelectionPath(_target.getPath());
			return;
		    }

		    //
		    // Success
		    //
		    _target = parent.findChildWithDescriptor(descriptor);
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		    refreshDisplay = true;
		}
		else if(isSimpleUpdate())
		{
		    writeDescriptor();
		}
		else
		{
		    //
		    // Save to be able to rollback
		    //
		    Object savedDescriptor = _target.saveDescriptor();
		    ListParent parent = (ListParent)_target.getParent();
		    writeDescriptor();
		    
		    try
		    {
			parent.tryUpdate(_target);
		    }
		    catch(UpdateFailedException e)
		    {
			_target.restoreDescriptor(savedDescriptor);
			JOptionPane.showMessageDialog(
			    model.getMainFrame(),
			    e.toString(),
			    "Apply failed",
			    JOptionPane.ERROR_MESSAGE);
			return;
		    }
		   
		    //
		    // Success
		    //
		    _target = parent.findChildWithDescriptor(_target.getDescriptor());
		    model.setSelectionPath(_target.getPath());
		    model.showActions(_target);
		}
	    
		postUpdate();

		_target.getEditable().markModified();
		_applyButton.setEnabled(false);
		_discardButton.setEnabled(false);
	    }
	    finally
	    {
		model.enableDisplay();
	    }
	    if(refreshDisplay)
	    {
		model.refreshDisplay();
	    }
	}
    }
}
