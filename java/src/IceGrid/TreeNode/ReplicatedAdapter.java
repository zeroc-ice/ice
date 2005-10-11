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
    static public ReplicatedAdapterDescriptor 
    copyDescriptor(ReplicatedAdapterDescriptor d)
    {
	return (ReplicatedAdapterDescriptor)d.clone();
    }

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

    void rebuild(ReplicatedAdapterDescriptor descriptor)
    {
	_descriptor = descriptor;
	//
	// And that's it since there is no children
	//
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }

    private ReplicatedAdapterDescriptor _descriptor;
}
