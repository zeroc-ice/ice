// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.util.Enumeration;

//
// An TreeNode that holds a list of children
//
abstract class ListTreeNode extends TreeNode
{
    @Override
    public Enumeration<Object> children()
    {
        return new Enumeration<Object>()
            {
                @Override
                public boolean hasMoreElements()
                {
                    return _p.hasNext();
                }

                @Override
                public Object nextElement()
                {
                    return _p.next();
                }

                private java.util.Iterator _p = _children.iterator();
            };
    }

    @Override
    public boolean getAllowsChildren()
    {
        return true;
    }

    @Override
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        else if(childIndex < _children.size())
        {
            return _children.get(childIndex);
        }
        else
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
    }

    @Override
    public int getChildCount()
    {
        return _children.size();
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        return _children.indexOf(node);
    }

    @Override
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
