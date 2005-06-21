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

class Adapter extends Leaf
{
    Adapter(AdapterDescriptor descriptor, boolean inTemplate)
    {
	_descriptor = descriptor;
	_inTemplate = inTemplate;
    }

    public String toString()
    {
	return _descriptor.name;
    }

    private AdapterDescriptor _descriptor;
    private boolean _inTemplate;
}
