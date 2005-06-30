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
    
    protected Leaf(String id, Model model)
    {
	super(id, model, -1);
    }
}
