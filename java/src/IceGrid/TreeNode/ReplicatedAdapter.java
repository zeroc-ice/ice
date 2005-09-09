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

class ReplicatedAdapter extends EditableLeaf
{
    ReplicatedAdapter(boolean brandNew, 
		      ReplicatedAdapterDescriptor descriptor,
		      Model model)
    {
	//
	// Make the assumption that app variables can't be used in
	// the descriptor.id
	//

	super(brandNew, descriptor.id, model);
	rebuild(descriptor);
    }

    ReplicatedAdapter(ReplicatedAdapter o)
    {
	super(o);
	_descriptor = o._descriptor;
    }

    void rebuild(ReplicatedAdapterDescriptor descriptor)
    {
	_descriptor = descriptor;
	//
	// And that's it since there is no children
	//
    }

    ReplicatedAdapterDescriptor getDescriptor()
    {
	return _descriptor;
    }

    private ReplicatedAdapterDescriptor _descriptor;
}
