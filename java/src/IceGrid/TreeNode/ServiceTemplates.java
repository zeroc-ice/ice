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
	super("Service templates");

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServiceTemplate((String)entry.getKey(),
					 (TemplateDescriptor)entry.getValue()));
	}
    }

    void update(java.util.Map descriptors, String[] removeTemplates)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	//
	// One big set of removes
	//
	removeChildren(removeTemplates);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    TemplateDescriptor templateDescriptor = (TemplateDescriptor)entry.getValue();
	    ServiceTemplate child = (ServiceTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServiceTemplate(name, templateDescriptor));
	    }
	    else
	    {
		child.rebuild(templateDescriptor);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    private java.util.Map _descriptors;
}
