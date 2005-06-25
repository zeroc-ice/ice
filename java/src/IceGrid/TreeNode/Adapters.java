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

class Adapters extends Parent
{
    Adapters(java.util.List descriptors, boolean inTemplate)
    {
	super("Adapters");

	_descriptors = descriptors;
	_inTemplate = inTemplate;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    Adapter child = new Adapter(descriptor, _inTemplate);
	    addChild(child);
	}
    }

    private java.util.List _descriptors;
    private boolean _inTemplate;
}
