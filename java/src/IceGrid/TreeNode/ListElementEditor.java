// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.Model;

abstract class ListElementEditor extends Editor
{  
    abstract void writeDescriptor();
    abstract boolean isSimpleUpdate();
    void postUpdate() {}

    //
    // Update when parent is a ListParent
    //
    protected void applyUpdate()
    {
	Model model = _target.getModel();

	if(model.canUpdate())
	{    
	    model.disableDisplay();

	    try
	    {
		if(_target.isEphemeral())
		{
		    ListParent parent = (ListParent)_target.getParent();
		    writeDescriptor();
		    Object descriptor = _target.getDescriptor();
		    parent.addDescriptor(descriptor);
		    _target.destroy(); // just removes the child
		    
		    if(!parent.tryUpdate(descriptor))
		    {
			//
			// Restores old display
			//
			parent.removeDescriptor(descriptor);
			try
			{
			    parent.addChild(_target, true);
			}
			catch(UpdateFailedException die)
			{
			    assert false;
			}
			return;
		    }
		    else
		    {
			_target = parent.findChildWithDescriptor(descriptor);
			model.setSelectionPath(_target.getPath());
			model.showActions(_target);
		    }
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
		    
		    if(!parent.tryUpdate(_target.getDescriptor()))
		    {
			_target.restoreDescriptor(savedDescriptor);
			
			//
			// Everything was restored, user must deal with error
			//
			return;
		    }
		    else
		    {
			_target = parent.findChildWithDescriptor(_target.getDescriptor());
			model.setSelectionPath(_target.getPath());
			model.showActions(_target);
		    }
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
	}
    }
}
