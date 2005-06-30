// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ServerInstanceDescriptor;
import IceGrid.Model;

class ServerInstances extends Parent
{
    ServerInstances(java.util.List descriptors, 
		    Model model,
		    boolean fireEvent)
    {
	super("Server instances", model);
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    //
	    // The ServerInstance constructor inserts the new object in the 
	    // node view model
	    //
	    ServerInstanceDescriptor descriptor = 
		(ServerInstanceDescriptor)p.next();
		
	    ServerInstance child = new ServerInstance(descriptor,
						      _model,
						      fireEvent);
	    addChild(child);
	}
    }

    void update(java.util.List descriptors, String[] removeServers)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	//
	// One big set of removes
	//
	for(int i = 0; i < removeServers.length; ++i)
	{
	    ServerInstance server = (ServerInstance)findChild(removeServers[i]);
	    server.removeFromNode();
	}
	removeChildren(removeServers);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor descriptor = (ServerInstanceDescriptor)p.next();
	    ServerInstance child = (ServerInstance)findChild(descriptor.descriptor.name);
	    if(child == null)
	    {
		newChildren.add(new ServerInstance(descriptor, _model, true));
	    }
	    else
	    {
		child.rebuild(descriptor, true);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    void removeFromNodes()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerInstance server = (ServerInstance)p.next();
	    server.removeFromNode();
	}
    }


    private java.util.List _descriptors;
}
