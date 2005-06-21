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
import IceGrid.TemplateDescriptor;

class ServiceInstances extends Parent
{
    ServiceInstances(ServiceInstanceDescriptor[] descriptors,
		     java.util.Map serviceTemplates)
    {
	_descriptors = descriptors;

	for(int i = 0; i < _descriptors.length; ++i)
	{
	    TemplateDescriptor templateDescriptor =
		(TemplateDescriptor)serviceTemplates.
		get(_descriptors[i].template);

	    addChild(new ServiceInstance(_descriptors[i],
					 templateDescriptor));
	}
    }

    public String toString()
    {
	return "Service instances";
    }


    private ServiceInstanceDescriptor[] _descriptors;
}
