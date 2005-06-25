// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.NodeDescriptor;

class NodeVars extends Parent
{
    NodeVars(java.util.List descriptors)
    {
	super("Node settings");
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    NodeDescriptor descriptor = (NodeDescriptor)p.next();
	    addChild(new NodeVar(descriptor));
	}
    }
    
    void update(java.util.List descriptors, String[] removeNodes)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	//
	// One big set of removes
	//
	removeChildren(removeNodes);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    NodeDescriptor nodeDescriptor = (NodeDescriptor)p.next();
	    NodeVar child = (NodeVar)findChild(nodeDescriptor.name);
	    if(child == null)
	    {
		newChildren.add(new NodeVar(nodeDescriptor));
	    }
	    else
	    {
		child.rebuild(nodeDescriptor);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }
}
