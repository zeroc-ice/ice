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
import IceGrid.TreeModelI;

//
// Common implementation for non-leaf nodes
// 

class Parent extends CommonBaseI
{
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
    
    public void addParent(CommonBase parent)
    {
	assert(parent != null);
	for(int i = 0; i < TreeModelI.VIEW_COUNT; i++)
	{ 
	    TreePath parentPath = parent.getPath(i);
	    if(parentPath != null)
	    {
		addParent(parent, parentPath, i);
	    }
	}
    }

    public void addParent(CommonBase parent, TreePath parentPath, int view)
    {
	assert(parent != null);
	assert(parentPath != null);
	assert(_parents[view] == null);
	assert(_paths[view] == null);

	_parents[view] = parent;
	_paths[view] = parentPath.pathByAddingChild(this);

    	//
	// Propagate to children
	//
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    child.addParent(this, _paths[view], view);
	}
    }

    public void removeParent(CommonBase parent)
    {
	assert(parent != null);

	for(int i = 0; i < TreeModelI.VIEW_COUNT; i++)
	{ 
	    if(_parents[i] == parent)
	    {
		removeParent(i);
	    }
	}
    }

    public void removeParent(int view)
    {
	_paths[view] = null;
	_parents[view] = null;

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    child.removeParent(view);
	}
    }

    CommonBase findChild(String id)
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

    void addChild(CommonBase child)
    {
	addChild(child, false);
    }

    void addChild(CommonBase child, boolean fireEvent)
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
	    if(id.compareTo(existingChild.getId()) < 0)
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
	_children.remove(child);
    }

    void removeChild(String id, boolean fireEvent)
    {
	int i = 0;
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    CommonBase child = (CommonBase)p.next();
	    i++;
	    if(id.equals(child.getId()))
	    {
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
	_children.clear();
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
		if(id.compareTo(existingChild.getId()) < 0)
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
	
	System.err.println("childIds.length == " + childIds.length);
	System.err.println("ids.length == " + ids.length);

	Object[] childrenToRemove = new Object[ids.length];
	int[] indices = new int[ids.length];

	int i = 0;
	int j = 0;
	int k = 0;
	java.util.Iterator p = _children.iterator();

	while(p.hasNext() && j < ids.length)
	{
	    CommonBase child = (CommonBase)p.next();
	    System.err.println("child.getId() == " + child.getId());
	    System.err.println("ids[j] == " + ids[j]);
	    
	    if(ids[j].equals(child.getId()))
	    {
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
	for(int i = 0; i < TreeModelI.VIEW_COUNT; ++i)
	{
	    if(_paths[i] != null)
	    {
		TreeModelEvent event = new TreeModelEvent(source, _paths[i], 
							  childIndices, children);
		TreeModelI.getTreeModel(i).fireNodesInsertedEvent(event);
	    }
	}
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
	for(int i = 0; i < TreeModelI.VIEW_COUNT; ++i)
	{
	    if(_paths[i] != null)
	    {
		System.err.print("Path = ");
		for(int j = 0; j < _paths[i].getPath().length; ++j)
		{
		    System.err.print(_paths[i].getPath()[j].toString()); 
		}
		System.err.println("");


		TreeModelEvent event = new TreeModelEvent(source, _paths[i], 
							  childIndices, children);
		TreeModelI.getTreeModel(i).fireNodesRemovedEvent(event);
	    }
	}
    }




    Parent(String id, int rootForView)
    {
	super(id, rootForView);
    }

    Parent(String id)
    {
	this(id, -1);
    }
    

    protected java.util.List _children = new java.util.LinkedList();
    private ChildComparator _childComparator = new ChildComparator();
}
