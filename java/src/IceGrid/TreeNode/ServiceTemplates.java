// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.TemplateDescriptor;

class ServiceTemplates extends Parent
{
    ServiceTemplates(java.util.Map descriptors)
    {
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServiceTemplate((String)entry.getKey(),
					 (TemplateDescriptor)entry.getValue()));
	}
    }

    public String toString()
    {
	return "Service templates";
    }


    private java.util.Map _descriptors;
}
