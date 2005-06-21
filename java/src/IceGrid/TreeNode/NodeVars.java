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
    NodeVars(NodeDescriptor[] descriptors)
    {
	_descriptors = descriptors;

	for(int i = 0; i < _descriptors.length; ++i)
	{
	    addChild(new NodeVar(_descriptors[i]));
	}
    }

    public String toString()
    {
	return "Node settings";
    }

    private NodeDescriptor[] _descriptors;
}
