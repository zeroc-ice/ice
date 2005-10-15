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
import IceGrid.ReplicaGroupDescriptor;
import IceGrid.SimpleInternalFrame;
import IceGrid.Utils;

class ReplicaGroup extends EditableLeaf
{
    static public ReplicaGroupDescriptor 
    copyDescriptor(ReplicaGroupDescriptor d)
    {
	return (ReplicaGroupDescriptor)d.clone();
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	if(_parent.getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}
	actions[DELETE] = true;
	actions[SUBSTITUTE_VARS] = true;
	return actions;
    }

    public void copy()
    {
	_model.setClipboard(copyDescriptor(_descriptor));
	_model.getActions()[PASTE].setEnabled(true);

    }
    public void paste()
    {
	_parent.paste();
    }

    public boolean destroy()
    {
	if(_parent == null)
	{
	    return false;
	}
	ReplicaGroups replicaGroups = (ReplicaGroups)_parent;
	
	if(_ephemeral)
	{
	    replicaGroups.removeChild(this, true);
	    return true;
	}
	else if(_model.canUpdate())
	{
	    replicaGroups.removeDescriptor(_descriptor);
	    replicaGroups.removeElement(this, true);
	    return true;
	}
	return false;
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	
	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new ReplicaGroupEditor(_model.getMainFrame());
	}
	propertiesFrame.setContent(_editor.getComponent());
	_editor.show(this);

	propertiesFrame.validate();
	propertiesFrame.repaint();
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }
    
    public Object saveDescriptor()
    {
	return _descriptor.clone();
    }
    public void restoreDescriptor(Object savedDescriptor)
    {
	ReplicaGroupDescriptor clone = (ReplicaGroupDescriptor)savedDescriptor;
	_descriptor.id = clone.id;
	_descriptor.description = clone.description;
	_descriptor.objects = clone.objects;
	_descriptor.loadBalancing = clone.loadBalancing;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    ReplicaGroup(boolean brandNew, 
		 ReplicaGroupDescriptor descriptor,
		 Model model)
    {
	super(brandNew, descriptor.id, model);
	_ephemeral = false;
	rebuild(descriptor);
    }
    
    ReplicaGroup(ReplicaGroupDescriptor descriptor,
		 Model model)
    {
	super(false, descriptor.id, model);
	_ephemeral = true;
	rebuild(descriptor);
    }
    
    void rebuild(ReplicaGroupDescriptor descriptor)
    {
	_descriptor = descriptor;
	//
	// And that's it since there is no children
	//
    }

    private ReplicaGroupDescriptor _descriptor;
    private final boolean _ephemeral;
    static private ReplicaGroupEditor _editor;
}
