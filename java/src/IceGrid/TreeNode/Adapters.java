// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;

class Adapters extends Parent
{
    Adapters(java.util.List descriptors, Model model, Node node)
    {
	super("Adapters", model);

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    Adapter child = new Adapter(descriptor, _model, node);
	    addChild(child);
	}
    }

    void unregisterAll()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.unregister();
	}
    }

    private java.util.List _descriptors;
}
