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
import IceGrid.Model;

class ServiceTemplates extends EditableParent
{
    ServiceTemplates(java.util.Map descriptors, Model model)
	throws DuplicateIdException
    {
	super(false, "Service templates", model);

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServiceTemplate(false, (String)entry.getKey(),
					 (TemplateDescriptor)entry.getValue(),
					 _model));
	}
    }

    ServiceTemplates(ServiceTemplates o)
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
		addChild(new ServiceTemplate((ServiceTemplate)p.next()));
	    }
	    catch(DuplicateIdException e)
	    {
		assert false;
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
	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String templateId = (String)entry.getKey();
	    TemplateDescriptor d = (TemplateDescriptor)entry.getValue();

	    ServiceTemplate t = (ServiceTemplate)findChild(templateId);
	    if(t != null)
	    {
		t.rebuild(d);
	    }
	}
	purgeChildren(_descriptors.keySet());
    }


    void update(java.util.Map descriptors, String[] removeTemplates)
	throws DuplicateIdException
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
	    TemplateDescriptor templateDescriptor 
		= (TemplateDescriptor)entry.getValue();
	    ServiceTemplate child = (ServiceTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServiceTemplate(false, name, 
						    templateDescriptor, _model));
	    }
	    else
	    {
		child.rebuild(templateDescriptor);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray
		       (new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));

	p = newChildren.iterator();
	while(p.hasNext())
	{
	    ServiceTemplate serviceTemplate = (ServiceTemplate)p.next();
	    serviceTemplate.setParent(this);
	}

    }

    private java.util.Map _descriptors;
}
