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
// ServerTemplate, ServiceTemplate, Node etc.
//
class EditableParent extends Parent implements Editable
{
    public boolean isNew()
    {
	return _isNew;
    }

    public boolean isModified()
    {
	return _modified;
    }

    public void markModified()
    {
	_modified = true;
    }

    public Editable getEditable()
    {
	return this;
    }

    public void commit()
    {
	_isNew = false;
	_modified = false;
	_removedElements.clear();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Object child = p.next();
	    if(child instanceof Editable)
	    {
		Editable editable = (Editable)child;
		editable.commit();
	    }
	}
    }

    void removeElement(CommonBase child, boolean fireEvent)
    {
	_removedElements.add(child.getId());
	removeChild(child, fireEvent);
    }

    void removeElement(String id)
    {
	_removedElements.add(id);
    }
    
    void purgeChildren(java.util.Set keepSet)
    {
	java.util.List toRemove = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    if(!keepSet.contains(child.getId()))
	    {
		_removedElements.add(child.getId());
		toRemove.add(child.getId());
	    }
	}

	if(toRemove.size() > 0)
	{
	    removeChildren((String[])toRemove.toArray(new String[0]));
	}
    }

    String[] removedElements()
    {
	return (String[])_removedElements.toArray(new String[0]);
    }

    protected EditableParent(boolean brandNew, String id, Model model, boolean root)
    {
	super(id, model, root);
	_isNew = brandNew;
    }

    protected EditableParent(boolean brandNew, String id, Model model)
    {
	this(brandNew, id, model, false);
    }

    protected EditableParent(EditableParent o, boolean copyChildren)
    {
	super(o, copyChildren);
	_modified = o._modified;
	_isNew = o._isNew;
	_removedElements = (java.util.TreeSet)o._removedElements.clone();
    }

    protected EditableParent(EditableParent o)
    {
	this(o, false);
    }

    void clearNew()
    {
	_isNew = false;
    }
    
    void markNew()
    {
	_isNew = true;
    }

    private boolean _isNew = false;
    private boolean _modified = false;
    protected java.util.TreeSet _removedElements = new java.util.TreeSet();
}


