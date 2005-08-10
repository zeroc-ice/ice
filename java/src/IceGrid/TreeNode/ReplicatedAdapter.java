// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ReplicatedAdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class ReplicatedAdapter extends Leaf
{
    ReplicatedAdapter(ReplicatedAdapterDescriptor descriptor,
	  Model model)
    {
	super(descriptor.id, model);
	rebuild(descriptor);
    }

    void rebuild(ReplicatedAdapterDescriptor descriptor)
    {
	_descriptor = descriptor;
    }

    private ReplicatedAdapterDescriptor _descriptor;
}
