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
    protected ListParent(String id, Model model, boolean root)
    {
	super(id, model, root);
    }

    protected ListParent(String id, Model model)
    {
	this(id, model, false);
    }
    
    protected void addDescriptor(Object descriptor)
    {
	_descriptors.add(descriptor);
    }

    protected void removeDescriptor(Object descriptor)
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
    // Try to add this new element
    //
    void tryAdd(Object descriptor)
	throws UpdateFailedException
    {
	_descriptors.add(descriptor);

	java.util.List newChildren = new java.util.LinkedList();
	java.util.List list = _parent.findAllInstances(this);
	System.err.println("Parent == " + _parent.getId());
	System.err.println("Found " + list.size() + " instances");
	java.util.Iterator p = list.iterator();
	while(p.hasNext())
	{
	    ListParent container = (ListParent)p.next();
	    
	    try
	    {
		newChildren.add(container.addNewChild(descriptor));
	    }
	    catch(UpdateFailedException e)
	    {
		//
		// Rollback
		//
		java.util.Iterator q = newChildren.iterator();
		p = list.iterator();
		while(q.hasNext())
		{
		    container = (ListParent)p.next();
		    container.removeChild((CommonBase)q.next());
		}
		removeDescriptor(descriptor);
		throw e;
	    }
	}
    }
        
    //
    // Try to update this child
    //
    void tryUpdate(CommonBase child)
	throws UpdateFailedException
    {
	java.util.List list = _parent.findAllInstances(this);
	Object descriptor = child.getDescriptor();
	
	java.util.List backupList = new java.util.Vector();
	java.util.List children = new java.util.Vector();
	java.util.List editables = new java.util.LinkedList();

	java.util.Iterator p = list.iterator();
	while(p.hasNext())
	{
	    ListParent parent = (ListParent)p.next();
	    try
	    {
		child = parent.findChildWithDescriptor(descriptor);
		backupList.add(parent.rebuildChild(child, editables));
		children.add(child);
	    }
	    catch(UpdateFailedException e)
	    {
		for(int i = backupList.size() - 1; i >= 0; --i)
		{
		    parent = (ListParent)list.get(i);
		    child = (CommonBase)children.get(i);
		    parent.restoreChild(child, backupList.get(i));
		}
		throw e;
	    }
	}

	//
	// Success
	//
	p = editables.iterator();
	while(p.hasNext())
	{
	    Editable editable = (Editable)p.next();
	    editable.markModified();
	}
    }


    boolean destroyChild(CommonBase child)
    {
	if(child.isEphemeral())
	{
	    removeChild(child, true);
	    return true;
	}
	else
	{
	    if(_model.canUpdate())
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
		return true;
	    }
	    else
	    {
		return false;
	    }
	}
    }


    protected java.util.List _descriptors;
}


