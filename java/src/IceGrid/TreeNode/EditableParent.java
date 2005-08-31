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
// Corresponds to "root" editable descriptors such as Server,
// Service, ServerTemplate, ServiceTemplate, Node etc.
//
class EditableParent extends Parent implements Editable
{
    EditableParent(boolean brandNew, String id, Model model, boolean root)
    {
	super(id, model, root);
	_modified = brandNew;
    }

    EditableParent(boolean brandNew, String id, Model model)
    {
	this(brandNew, id, model, false);
    }

    EditableParent(EditableParent o, boolean copyChildren)
    {
	super(o, copyChildren);
	_modified = o._modified;
	_removedElements = (java.util.TreeSet)o._removedElements.clone();
    }


    EditableParent(EditableParent o)
    {
	this(o, false);
    }

    public boolean isModified()
    {
	return _modified;
    }

    public void markModified()
    {
	_modified = true;
    }

    void removeElement(CommonBase child)
    {
	_removedElements.add(child.getId());
	removeChild(child);
    }
    
    void purgeChildren(java.util.Set keepSet)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    if(!keepSet.contains(child.getId()))
	    {
		removeElement(child);
	    }
	}
    }

    java.util.Set removedElements()
    {
	return _removedElements;
    }

    public Editable getEditable()
    {
	return this;
    }
    
    private boolean _modified;
    private java.util.TreeSet _removedElements = new java.util.TreeSet();
}


