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
	    _selectedItem = obj;
	    fireContentsChanged(this, -1, -1);
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
    
    public void setParent(CommonBase parent)
    {
	_parent = parent;
	if(parent == null)
	{
	    _path = null;
	}
	else
	{
	    TreePath parentPath = _parent.getPath();
	    if(parentPath == null)
	    {
		_path = null;
	    }
	    else
	    {
		_path = parentPath.pathByAddingChild(this);
	    }
	}
	
    	//
	// Propagate to children
	//
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    child.setParent(this);
	}
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

    void addChild(CommonBase child) throws DuplicateIdException
    {
	addChild(child, false);
    }

    void addChild(CommonBase child, boolean fireEvent)
	throws DuplicateIdException
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
		throw new DuplicateIdException(this, id);
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
	if(fireEvent)
	{
	    fireNodeInsertedEvent(this, child, i);
	}
    }

    void removeChild(CommonBase child)
    {
	if(_children.remove(child))
	{
	    child.unregister();
	}
    }

    void removeChild(CommonBase child, boolean fireEvent)
    {
	int index = _children.indexOf(child);
	if(index > -1)
	{
	    child.unregister();
	    _children.remove(child);
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
		child.unregister();
		p.remove();
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
	    child.unregister();
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
	throws DuplicateIdException
    {
	if(newChildren.length == 0)
	{
	    //
	    // Nothing to do;
	    //
	    return;
	}

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
		    throw new DuplicateIdException(this, id);
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
	}
	fireNodesInsertedEvent(this, children, indices);
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
		child.unregister();
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

    
    //
    // in childIds: the children to remove
    // out childIds: the children not removed
    //
    void removeChildren(java.util.List childIds)
    {
	if(childIds.size() == 0)
	{
	    //
	    // Nothing to do;
	    //
	    return;
	}
	
	java.util.Collections.sort(childIds);

	java.util.Vector childrenToRemove = new java.util.Vector(childIds.size());
	int[] indices = new int[childIds.size()];

	java.util.Iterator q = childIds.iterator();
	java.util.Iterator p = _children.iterator();

	int i = -1;
	int k = 0;
	while(q.hasNext() && p.hasNext())
	{
	    String id = (String)q.next();

	    while(p.hasNext())
	    {
		CommonBase child = (CommonBase)p.next();
		i++;

		if(id.equals(child.getId()))
		{
		    child.unregister();
		    childrenToRemove.add(child);
		    indices[k++] = i;
		    p.remove();
		    q.remove();
		    break; // while
		}
		else if(id.compareTo(child.getId()) < 0)
		{
		    //
		    // Need to get next id
		    //		    
		    break; // while
		}
	    }
	}
	
	if(k > 0)
	{
	    childrenToRemove.trimToSize();
	    int[] trimedIndices;
	    if(childIds.size() > 0)
	    {
		trimedIndices = new int[k];
		System.arraycopy(indices, 0, trimedIndices, 0, k); 
	    }
	    else
	    {
		trimedIndices = indices;
	    }

	    fireNodesRemovedEvent(this, 
				  childrenToRemove.toArray(), 
				  trimedIndices);
	}
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

    protected java.util.LinkedList _children = new java.util.LinkedList();
    private ChildComparator _childComparator = new ChildComparator();
}
