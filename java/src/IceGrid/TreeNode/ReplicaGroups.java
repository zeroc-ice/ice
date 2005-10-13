// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.AbstractListModel;

import IceGrid.ReplicaGroupDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class ReplicaGroups extends EditableParent
{
    static public java.util.LinkedList
    copyDescriptors(java.util.List descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(ReplicaGroup.copyDescriptor(
			 (ReplicaGroupDescriptor)p.next()));
	}
	return copy;
    }

    ReplicaGroups(java.util.List descriptors, Model model)
	throws UpdateFailedException
    {
	super(false, "Replica Groups", model);
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ReplicaGroupDescriptor descriptor 
		= (ReplicaGroupDescriptor)p.next();
	    
	    addChild(new ReplicaGroup(false, descriptor, _model));
	}
    }

    java.util.LinkedList getUpdates()
    {
	java.util.LinkedList updates = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ReplicaGroup ra = (ReplicaGroup)p.next();
	    if(ra.isNew() || ra.isModified())
	    {
		updates.add(ra.getDescriptor());
	    }
	}
	return updates;
    }

    void update(java.util.List descriptors, String[] removeAdapters)
	throws UpdateFailedException
    {
	_descriptors = descriptors;

	//
	// One big set of removes
	//
	removeChildren(removeAdapters);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ReplicaGroupDescriptor descriptor =
		(ReplicaGroupDescriptor)p.next();
	    
	    ReplicaGroup child 
		= (ReplicaGroup)findChild(descriptor.id);
	    if(child == null)
	    {
		newChildren.add(new ReplicaGroup(false, descriptor,
						      _model));
	    }
	    else
	    {
		child.rebuild(descriptor);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray
		       (new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    private java.util.List _descriptors;
}
