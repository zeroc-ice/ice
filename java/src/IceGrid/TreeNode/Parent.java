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
import IceGrid.TreeModelI;

//
// Common implementation for non-leaf nodes
// 

class Parent extends CommonBaseI
{
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

    
    void addChild(CommonBase child)
    {
	//
	// TODO: sort children by alphabetic order according to toString()
	//
	_children.add(child);
    }

    void removeChild(CommonBase child)
    {
	_children.remove(child);
    }

    void clearChildren()
    {
	_children.clear();
    }
    
    Parent(int rootForView)
    {
	super(rootForView);
    }

    Parent()
    {
	this(-1);
    }
    

    protected java.util.List _children = new java.util.LinkedList();
}
