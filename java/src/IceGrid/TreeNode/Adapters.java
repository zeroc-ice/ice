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
import IceGrid.Utils;

class Adapters extends Parent
{
    Adapters(java.util.List descriptors, boolean editable, 
	     Utils.Resolver resolver, Model model)
    {
	super("Adapters", model);
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    
	    String adapterName = Utils.substitute(descriptor.name,
						  resolver);
	    
	    addChild(new Adapter(adapterName, descriptor, editable, 
				 resolver, _model));
	}
    }
}
