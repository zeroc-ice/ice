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
    public void unregister()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.unregister();
	}
    }


    Adapters(java.util.List descriptors, boolean isEditable, 
	     boolean inIceBox, Utils.Resolver resolver, 
	     Application application, Model model)
	throws DuplicateIdException
    {
	super("Adapters", model);
	_descriptors = descriptors;
	_isEditable = isEditable;
	_inIceBox = inIceBox;
	_resolver = resolver;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    
	    String adapterName = Utils.substitute(descriptor.name,
						  _resolver);
	    
	    addChild(new Adapter(adapterName, descriptor, 
				 _resolver, application, _model));
	}
    }
    
    boolean canHaveNewChild()
    {
	return _isEditable && !_inIceBox;
    }

    boolean isEditable()
    {
	return _isEditable;
    }

    boolean inIceBox()
    {
	return _inIceBox;
    }

    void newChild()
    {
	newAdapter(null);
    }

    void newAdapter(AdapterDescriptor descriptor)
    {
	//
	// Generate a unique child name; ignore substitution for simplicity
	//
	String baseName = descriptor == null ? "NewAdapter" : descriptor.name;
	String name = baseName;

	int i = 0;
	while(findChild(name) != null || findChild("*" + name) != null)
	{
	    name = baseName + "-" + (++i);
	}

	if(descriptor == null)
	{
	    CommonBase parent = getParent();
	    String defaultId = (parent instanceof Service || parent instanceof ServiceTemplate) ? 
		"${server}.${service}." + name : "${server}." + name;

	    descriptor = new AdapterDescriptor(
		name,
		defaultId,
		false,
		true,
		new java.util.LinkedList());   
	}
	else
	{
	    descriptor.name = name;
	}

	Adapter adapter = new Adapter(name, descriptor, _resolver, _model);
	try
	{
	    addChild(adapter, true);
	}
	catch(DuplicateIdException e)
	{
	    assert false;
	}
	adapter.setParent(this);
	_model.setSelectionPath(adapter.getPath());
    }

    public void paste(Object descriptor)
    {
	if(canHaveNewChild() && descriptor instanceof AdapterDescriptor)
	{
	    AdapterDescriptor d = (AdapterDescriptor)descriptor;
	    newAdapter((AdapterDescriptor)d.clone());
	}
    }

    void addDescriptor(AdapterDescriptor descriptor)
    {
	_descriptors.add(descriptor);
    }

    void removeDescriptor(AdapterDescriptor descriptor)
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

    private java.util.List _descriptors;
    private Utils.Resolver _resolver;
    private boolean _isEditable;
    private boolean _inIceBox;
}
