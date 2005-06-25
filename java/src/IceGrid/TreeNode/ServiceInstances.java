// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ServiceInstanceDescriptor;

class ServiceInstances extends Parent
{
    ServiceInstances(java.util.List descriptors)
    {
	super("Service instances");
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor descriptor = 
		(ServiceInstanceDescriptor)p.next();
	    addChild(new ServiceInstance(descriptor));
	}
    }

    private java.util.List _descriptors;
}
