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
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;

class ServerTemplates extends Parent
{
    ServerTemplates(java.util.Map descriptors, Application application)
    {
	super("Server templates", application.getModel());

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServerTemplate((String)entry.getKey(),
					(TemplateDescriptor)entry.getValue(),
					application));
	}
    }
    
    void update(java.util.Map updates, String[] removeTemplates)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	Application application = (Application)getParent();

	//
	// One big set of removes
	//
	removeChildren(removeTemplates);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    TemplateDescriptor templateDescriptor = (TemplateDescriptor)entry.getValue();
	    ServerTemplate child = (ServerTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServerTemplate(name, templateDescriptor, application));
	    }
	    else
	    {
		child.rebuild(templateDescriptor, application);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }
    
    private java.util.Map _descriptors;
}
