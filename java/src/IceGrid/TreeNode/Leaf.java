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
import IceGrid.Model;

//
// Common implementation for leaf nodes.
// 

class Leaf extends CommonBaseI
{
    public Object getChildAt(int childIndex)
    {
	return null;
    }
    
    public int getChildCount()
    {
	return 0;
    }

    public int getIndex(Object child)
    {
	return -1;
    }
    
    public boolean isLeaf()
    {
	return true;
    }

    public CommonBase findChild(String id)
    {
	return null;
    }
    
    public CommonBase findChildWithDescriptor(Object d)
    {
	return null;
    }
    
    public void setParent(CommonBase parent)
    {
	assert parent != null;
	_parent = (Parent)parent;
	TreePath parentPath = _parent.getPath();
	_path = parentPath.pathByAddingChild(this);
    }

    public void clearParent()
    {
	_parent = null;
	_path = null;
    }


    protected Leaf(String id, Model model)
    {
	super(id, model, false);
    }

    protected Leaf(Leaf o)
    {
	super(o);
    }
}
