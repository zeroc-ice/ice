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

class NodeVar extends Leaf
{
    NodeVar(NodeDescriptor descriptor)
    {
	_descriptor = descriptor;
    }

    public String toString()
    {
	return _descriptor.name;
    }

    private NodeDescriptor _descriptor;

}
