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
    Adapters(AdapterDescriptor[] descriptors, boolean inTemplate)
    {
	_descriptors = descriptors;
	_inTemplate = inTemplate;

	for(int i = 0; i < _descriptors.length; ++i)
	{
	    Adapter child = new Adapter(_descriptors[i], _inTemplate);
	    addChild(child);
	}
    }

    public String toString()
    {
	return "Adapters";
    }

    private AdapterDescriptor[] _descriptors;
    private boolean _inTemplate;
}
