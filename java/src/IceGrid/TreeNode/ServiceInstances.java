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
import IceGrid.Model;

class ServiceInstances extends Parent
{
    ServiceInstances(java.util.List descriptors, Model model, Node node)
    {
	super("Service instances", model);
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    ServiceInstanceDescriptor descriptor = 
		(ServiceInstanceDescriptor)p.next();
	    addChild(new ServiceInstance(descriptor, _model, node));
	}
    }

    void unregisterAdapters()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServiceInstance instance = (ServiceInstance)p.next();
	    instance.unregisterAdapters();
	}
    }

    private java.util.List _descriptors;
}
