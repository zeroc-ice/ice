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

import IceGrid.ReplicatedAdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class ReplicatedAdapters extends Parent
{
    ReplicatedAdapters(java.util.List descriptors, Model model)
    {
	super("Replicated Adapters", model);
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ReplicatedAdapterDescriptor descriptor 
		= (ReplicatedAdapterDescriptor)p.next();
	    
	    addChild(new ReplicatedAdapter(descriptor, _model));
	}
    }

    void update(java.util.List descriptors, String[] removeAdapters)
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
	    ReplicatedAdapterDescriptor descriptor =
		(ReplicatedAdapterDescriptor)p.next();
	    
	    ReplicatedAdapter child 
		= (ReplicatedAdapter)findChild(descriptor.id);
	    if(child == null)
	    {
		newChildren.add(new ReplicatedAdapter(descriptor,
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

	p = newChildren.iterator();
	while(p.hasNext())
	{
	    ReplicatedAdapter replicatedAdapter = (ReplicatedAdapter)p.next();
	    replicatedAdapter.setParent(this);
	}
    }

    private java.util.List _descriptors;
}
