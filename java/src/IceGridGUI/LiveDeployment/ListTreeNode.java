// **********************************************************************
//
// Copyright (c) 2003-2014 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

//
// An TreeNode that holds a list of children
//
abstract class ListTreeNode extends TreeNode
{
    public Enumeration children()
    {
        return new Enumeration()
            {
                public boolean hasMoreElements()
                {
                    return _p.hasNext();
                }

                public Object nextElement()
                {
                    return _p.next();
                }

                private java.util.Iterator _p = _children.iterator();
            };
    }

    public boolean getAllowsChildren()
    {
        return true;
    }

    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        else if(childIndex < _children.size())
        {
            return (javax.swing.tree.TreeNode)_children.get(childIndex);
        }
        else
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
    }

    public int getChildCount()
    {
        return _children.size();
    }

    public int getIndex(javax.swing.tree.TreeNode node)
    {
        return _children.indexOf(node);
    }

    public boolean isLeaf()
    {
        return _children.isEmpty();
    }

    protected ListTreeNode(TreeNode parent, String id)
    {
        super(parent, id);
    }

    protected final java.util.List<javax.swing.tree.TreeNode> _children =
        new java.util.LinkedList<javax.swing.tree.TreeNode>();
}
