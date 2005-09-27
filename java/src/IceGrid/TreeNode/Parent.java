// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.tree.TreePath;
import javax.swing.event.TreeModelEvent;
import IceGrid.Model;
import IceGrid.TreeModelI;

//
// Common implementation for non-leaf nodes
// 

class Parent extends CommonBaseI
{
   
    //
    // Adapts parent to a ComboBoxModel
    //
    class ComboBoxModel extends javax.swing.AbstractListModel 
	implements javax.swing.ComboBoxModel
    {
	public Object getElementAt(int index)
	{
	    return getChildAt(index);
	}

	public int getSize()
	{
	    return getChildCount();
	}
	
	public Object getSelectedItem()
	{
	    return _selectedItem;
	}

	public void setSelectedItem(Object obj)
	{
	    if(obj != _selectedItem)
	    {
		_selectedItem = obj;
		fireContentsChanged(this, -1, -1);
	    }
	}

	private Object _selectedItem;
    }

    static class ChildComparator implements java.util.Comparator
    {
	public int compare(Object o1, Object o2)
	{
	    CommonBase child1 = (CommonBase)o1;
	    CommonBase child2 = (CommonBase)o1;
	    return child1.getId().compareTo(child2.getId());
	}
    }

    public Object getChildAt(int childIndex)
    {
	try
	{
	    return _children.get(childIndex);
	}
	catch(IndexOutOfBoundsException e)
	{
	    return null;
	}
    }
    
    public int getChildCount()
    {
	return _children.size();
    }

    public int getIndex(Object child)
    {
	return _children.indexOf(child);
    }
    
    public boolean isLeaf()
    {
	return false;
    }
    

    public CommonBase findChild(String id)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase  child = (CommonBase)p.next();
	    if(id.equals(child.getId()))
	    {
		return child;
	    }
	}
	return null;
    }

    public CommonBase findChildWithDescriptor(Object descriptor)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    if(descriptor == child.getDescriptor())
	    {
		return child;
	    }
	}
	return null;
    }

    public void setParent(CommonBase parent)
    {
	_parent = (Parent)parent;
	
	TreePath parentPath = _parent.getPath();
	_path = parentPath.pathByAddingChild(this);
	
	//
	// Propagate to children
	//
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBaseI child = (CommonBaseI)p.next();
	    child.setParent(this);
	}
    }

    public void clearParent()
    {
	if(_parent != null)
	{
	    _parent = null;
	    _path = null;
	    
	    //
	    // Propagate to children
	    //
	    java.util.Iterator p = _children.iterator();
	    while(p.hasNext())
	    {
		CommonBaseI child = (CommonBaseI)p.next();
		child.clearParent();
	    }
	}
    }


    void addChild(CommonBase child) throws UpdateFailedException
    {
	addChild(child, false);
    }

    void addChild(CommonBase child, boolean fireEvent)
	throws UpdateFailedException
    {
	if(_sortChildren)
	{
	    //
	    // Sorted insert
	    //
	    String id = child.getId();
	    int i = 0;
	    java.util.Iterator p = _children.iterator();
	    
	    while(p.hasNext())
	    {
		CommonBase existingChild = (CommonBase)p.next();
		int cmp = id.compareTo(existingChild.getId());
		
		if(cmp == 0)
		{
		    throw new UpdateFailedException(this, id);
		}
		if(cmp < 0)
		{
		    break; // while
		}
		i++;
	    }
	    
	    if(i < _children.size())
	    {
		_children.add(i, child);
	    }
	    else
	    {
		_children.add(child);
	    }	
	    if(_path != null)
	    {
		child.setParent(this);
	    }

	    if(fireEvent)
	    {
		fireNodeInsertedEvent(this, child, i);
	    }
	}
	else
	{
	    //
	    // Unsorted insert (i.e. at the end)
	    //
	    String id = child.getId();
	    java.util.Iterator p = _children.iterator();
	    while(p.hasNext())
	    {
		CommonBase existingChild = (CommonBase)p.next();
		if(id.equals(existingChild.getId()))
		{
		    throw new UpdateFailedException(this, id);
		}
	    }
	    
	    _children.add(child);
	    if(_path != null)
	    {
		child.setParent(this);
	    }

	    if(fireEvent)
	    {
		fireNodeInsertedEvent(this, child, _children.size() - 1);
	    }
	}
    }

    void addChild(int index, CommonBase child, boolean fireEvent)
	throws UpdateFailedException
    {
	assert !_sortChildren;

	String id = child.getId();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase existingChild = (CommonBase)p.next();
	    if(id.equals(existingChild.getId()))
	    {
		throw new UpdateFailedException(this, id);
	    }
	}
	
	_children.add(index, child);
	
	if(_path != null)
	{
	    child.setParent(this);
	}
	
	if(fireEvent)
	{
	    fireNodeInsertedEvent(this, child, index);
	}
    }

    void moveChild(int index, boolean up, boolean fireEvent)
    {
	assert !_sortChildren;

	Object child = _children.remove(index);
	assert child != null;

	if(fireEvent)
	{
	    fireNodeRemovedEvent(this, child, index);
	}

	if(up)
	{
	    _children.add(index - 1, child);
	    if(fireEvent)
	    {
		fireNodeInsertedEvent(this, child, index - 1);
	    }
	}
	else
	{
	    _children.add(index + 1, child);
	    if(fireEvent)
	    {
		fireNodeInsertedEvent(this, child, index + 1);
	    }
	}
    }


    void removeChild(CommonBase child)
    {
	if(_children.remove(child))
	{
	    child.clearParent();
	}
    }

    void removeChild(CommonBase child, boolean fireEvent)
    {
	int index = _children.indexOf(child);
	if(index > -1)
	{
	    _children.remove(child);
	    child.clearParent();
	    if(fireEvent)
	    {
		fireNodeRemovedEvent(this, child, index);
	    }
	}
    }

    void removeChild(String id, boolean fireEvent)
    {
	int i = -1;
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    i++;
	    if(id.equals(child.getId()))
	    {
		p.remove();
		child.clearParent();
		if(fireEvent)
		{
		    fireNodeRemovedEvent(this, child, i);
		}
		break;
	    }
	}
    }

    void clearChildren()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    child.clearParent();
	}
	_children.clear();
    }

    void expandChildren()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    _model.getTree().expandPath(child.getPath());
	}
    }

    //
    // The following methods fire events
    //

    void updateChildren(CommonBaseI[] children)
    {
	for(int i = 0; i < children.length; ++i)
	{
	    children[i].fireStructureChangedEvent(this);
	}
    }

    void addChildren(CommonBaseI[] newChildren)
	throws UpdateFailedException
    {
	if(newChildren.length == 0)
	{
	    //
	    // Nothing to do;
	    //
	    return;
	}

	if(_sortChildren)
	{
	    //
	    // Shallow copy
	    //
	    CommonBaseI[] children =  (CommonBaseI[])newChildren.clone();
	    
	    java.util.Arrays.sort(children, _childComparator);
	    
	    int[] indices = new int[children.length];
	    
	    int i = 0;
	    java.util.Iterator p = _children.iterator();
	    for(int j = 0; j < children.length; ++j)
	    {
		String id = children[j].getId();
		
		while(p.hasNext()) 
		{
		    CommonBase existingChild = (CommonBase)p.next();
		    int cmp = id.compareTo(existingChild.getId());
		    if(cmp == 0)
		    {
			throw new UpdateFailedException(this, id);
		    }
		    if(cmp < 0)
		    {
			break; // while
		    }
		    i++;
		}
		
		if(i < _children.size())
		{    
		    // Insert here, and increment i (since children is sorted)
		    _children.add(i, children[j]);
		    indices[j] = i;
		    i++;
		}
		else
		{
		    // Append
		    _children.add(children[j]);
		    indices[j] = i;
		    i++;
		}
		if(_path != null)
		{
		    children[j].setParent(this);
		}
	    }
	    fireNodesInsertedEvent(this, children, indices);
	}
	else
	{
	    //
	    // Could optimize later!
	    //
	    for(int i = 0; i < newChildren.length; ++i)
	    {
		addChild(newChildren[i], true);
	    }
	}
    }
    
    
    void removeChildren(String[] childIds)
    {
	if(childIds.length == 0)
	{
	    //
	    // Nothing to do;
	    //
	    return;
	}

	if(_sortChildren)
	{
	    //
	    // Shallow copy
	    //
	    String[] ids = (String[])childIds.clone();
	    
	    java.util.Arrays.sort(ids);
	    
	    Object[] childrenToRemove = new Object[ids.length];
	    int[] indices = new int[ids.length];
	    
	    int i = 0;
	    int j = 0;
	    int k = 0;
	    java.util.Iterator p = _children.iterator();

	    while(p.hasNext() && j < ids.length)
	    {
		CommonBase child = (CommonBase)p.next();	    
		if(ids[j].equals(child.getId()))
		{
		    child.clearParent();
		    childrenToRemove[k] = child;
		    indices[k] = i;
		    p.remove();
		    ++j;
		    ++k;
		}
		++i;
	    }
	    
	    //
	    // Should be all removed
	    //
	    assert(k == ids.length);
	    fireNodesRemovedEvent(this, childrenToRemove, indices);
	}
	else
	{
	    //
	    // Could optimize later!
	    //
	    for(int i = 0; i < childIds.length; ++i)
	    {
		removeChild(childIds[i], true);
	    }
	}
    }
    
    java.util.List findChildrenWithType(Class type)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Object child = p.next();
	    if(child.getClass() == type)
	    {
		result.add(child);
	    }
	}
	return result;
    }

    void fireNodeInsertedEvent(Object source, Object child, int index)
    {
	int[] childIndices = new int[1];
	childIndices[0] = index;
	Object[] children = new Object[1];
	children[0] = child;
	
	fireNodesInsertedEvent(source, children, childIndices);
    }
    
    void fireNodesInsertedEvent(Object source, Object[] children, int[] childIndices)
    {	
	assert _path != null;
	TreeModelEvent event = new TreeModelEvent(source, _path, childIndices, children);
	_model.getTreeModel().fireNodesInsertedEvent(event);
    }

    void fireNodeRemovedEvent(Object source, Object child, int index)
    {
	int[] childIndices = new int[1];
	childIndices[0] = index;
	Object[] children = new Object[1];
	children[0] = child;
	
	fireNodesRemovedEvent(source, children, childIndices);
    }

    void fireNodesRemovedEvent(Object source, Object[] children, int[] childIndices)
    {	
	assert _path != null;

	TreeModelEvent event = new TreeModelEvent(source, _path, 
						  childIndices, children);
	_model.getTreeModel().fireNodesRemovedEvent(event);
    }

    javax.swing.ComboBoxModel createComboBoxModel()
    {
	return new ComboBoxModel();
    }

    Parent(String id, Model model, boolean root)
    {
	super(id, model, root);
    }

    Parent(String id, Model model)
    {
	this(id, model, false);
    }
    
    protected Parent(Parent o)
    {
	this(o, false);
	// Derived class is responsible to populate the children
    }

    protected Parent(Parent o, boolean copyChildren)
    {
	super(o);
	if(copyChildren)
	{
	    _children = (java.util.LinkedList)o._children.clone();
	}
    }
    
    protected void sortChildren(boolean val)
    {
	_sortChildren = val;
    }
    
    protected String makeNewChildId(String base)
    {
	String id = base;
	int i = 0;
	while(findChild(id) != null)
	{
	    id = base + "-" + (++i);
	}
	return id;
    }

    protected java.util.LinkedList _children = new java.util.LinkedList();
    private ChildComparator _childComparator = new ChildComparator();
    protected boolean _sortChildren = true;
}
