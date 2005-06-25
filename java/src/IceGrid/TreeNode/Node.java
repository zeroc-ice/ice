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

class Node extends Parent
{
    //
    // Node creation/deletion/renaming is done by starting/restarting
    // an IceGridNode process. Not through admin calls.
    //
    
    //
    // TODO: consider showing per-application node variables
    //

    Node(String applicationName, NodeDescriptor descriptor)
    {
	super(descriptor.name);
	_applicationMap.put(applicationName, descriptor);
    }

    void addApplication(String applicationName, NodeDescriptor descriptor)
    {
	_applicationMap.put(applicationName, descriptor);
    }
    
    //
    // Returns true when this node should be destroyed
    //
    boolean removeApplication(String applicationName)
    {
	_applicationMap.remove(applicationName);
	return (_applicationMap.size() == 0);
    }

    private java.util.Map _applicationMap = new java.util.HashMap();
}
