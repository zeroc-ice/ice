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

//
// A parent that holds a list of children 
//
abstract class ListParent extends Parent
{
    abstract boolean isEditable();
    
    abstract protected boolean validate(Object descriptor);
    abstract protected void applyUpdate(Object descriptor);

    protected ListParent(String id, Model model, boolean root)
    {
	super(id, model, root);
    }

    protected ListParent(String id, Model model)
    {
	this(id, model, false);
    }
    
    void addDescriptor(Object descriptor)
    {
	_descriptors.add(descriptor);
    }

    void removeDescriptor(Object descriptor)
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

    //
    // Try to update, returns true upon success
    // Only the child corresponding to the given descriptor needs to
    // be created or recreated
    //
    boolean tryUpdate(Object descriptor)
    {
	java.util.List list = _parent.findAllInstances(this);
	
	//
	// First validate
	//
	java.util.Iterator p = list.iterator();
	while(p.hasNext())
	{
	    ListParent container = (ListParent)p.next();
	    if(!container.validate(descriptor))
	    {
		return false;
	    }
	}

	//
	// Then applyUpdate
	//
	p = list.iterator();
	while(p.hasNext())
	{
	    ListParent container = (ListParent)p.next();
	    container.applyUpdate(descriptor);
	}
	return true;
    }


    boolean destroyChild(CommonBase child)
    {
	if(isEditable() && _model.canUpdate())
	{
	    if(child.isEphemeral())
	    {
		removeChild(child, true);
	    }
	    else
	    {
		Object descriptor = child.getDescriptor();
		removeDescriptor(descriptor);
		getEditable().markModified();
		
		//
		// List of Services, Adapters etc
		//
		java.util.List list = _parent.findAllInstances(this);
		java.util.Iterator p = list.iterator();
		while(p.hasNext())
		{
		    ListParent parent = (ListParent)p.next();
		    parent.removeChild(
			parent.findChildWithDescriptor(descriptor), 
			true);
		}
	    }
	    return true;
	}
	else
	{
	    return false;
	}
    }

    protected java.util.List _descriptors;
}


