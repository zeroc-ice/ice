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

class ServerTemplates extends EditableParent
{
    ServerTemplates(java.util.Map descriptors, Application application)
	throws DuplicateIdException
    {
	super(false, "Server templates", application.getModel());

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServerTemplate(false, (String)entry.getKey(),
					(TemplateDescriptor)entry.getValue(),
					application));
	}
    }

    ServerTemplates(ServerTemplates o)
    {
	super(o);
	_descriptors = o._descriptors;

	//
	// Deep-copy children
	//
	java.util.Iterator p = o._children.iterator();
	while(p.hasNext())
	{
	    try
	    {
		addChild(new ServerTemplate((ServerTemplate)p.next()));
	    }
	    catch(DuplicateIdException e)
	    {
		assert false;
	    }
	}
    }

    void getUpdates(java.util.Map updates)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerTemplate t = (ServerTemplate)p.next();
	    if(t.isNew() || t.isModified())
	    {
		updates.put(t.getId(), t.getDescriptor());
	    }
	}
    }

    void update() throws DuplicateIdException
    {
	//
	// The only template-descriptor update going through the
	// update() calls is the update or removal of one template;
	// template addition does not require a complex validation.
	//

	Application application = getApplication();

	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String templateId = (String)entry.getKey();
	    TemplateDescriptor d = (TemplateDescriptor)entry.getValue();

	    ServerTemplate t = (ServerTemplate)findChild(templateId);
	    if(t != null)
	    {
		t.rebuild(d, application);
	    }
	}
	purgeChildren(_descriptors.keySet());
	fireStructureChangedEvent(this);
    }

    void cascadeDeleteServiceInstance(String templateId)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerTemplate t = (ServerTemplate)p.next();
	    t.cascadeDeleteServiceInstance(templateId);
	}
    }
    
    void update(java.util.Map updates, String[] removeTemplates)
	throws DuplicateIdException
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	Application application = getApplication();

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
	    TemplateDescriptor templateDescriptor 
		= (TemplateDescriptor)entry.getValue();
	    ServerTemplate child = (ServerTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServerTemplate(false, name, templateDescriptor,
						   application));
	    }
	    else
	    {
		child.rebuild(templateDescriptor, application);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray
		       (new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));

	p = newChildren.iterator();
	while(p.hasNext())
	{
	    ServerTemplate serverTemplate = (ServerTemplate)p.next();
	    serverTemplate.setParent(this);
	}
    }
    
    private java.util.Map _descriptors;
}
