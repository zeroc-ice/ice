// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.Model;

//
// A simple container, for example: Services, Adapters, DbEnvs
//
class SimpleContainer extends Parent
{
    protected SimpleContainer(String id, Model model, boolean root)
    {
	super(id, model, root);
    }

    protected SimpleContainer(String id, Model model)
    {
	this(id, model, false);
    }
    
    void addDescriptor(Object descriptor)
    {
	_descriptors.add(descriptor);
    }

    void removeDescriptor(Object descriptor)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    if(descriptor == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }
    
    protected java.util.List _descriptors;
}


