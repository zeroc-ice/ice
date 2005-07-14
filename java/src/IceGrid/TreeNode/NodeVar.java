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
import IceGrid.Model;

class NodeVar extends Leaf
{
    NodeVar(NodeDescriptor descriptor, Model model)
    {
	super(descriptor.name, model);
	rebuild(descriptor);
    }

    void rebuild(NodeDescriptor descriptor)
    {
	_descriptor = descriptor;
    }

    java.util.Map getVariables()
    {
	return _descriptor.variables;
    }

    private NodeDescriptor _descriptor;

}
