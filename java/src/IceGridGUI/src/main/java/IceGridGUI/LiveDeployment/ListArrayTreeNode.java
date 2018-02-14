// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.util.Enumeration;

//
// A TreeNode that holds an array of list of children
//
abstract class ListArrayTreeNode extends TreeNode
{
    @Override
    @SuppressWarnings("unchecked")
    public Enumeration<javax.swing.tree.TreeNode> children()
    {
        return new Enumeration<javax.swing.tree.TreeNode>()
            {
                @Override
                public boolean hasMoreElements()
                {
                    if(_p.hasNext())
                    {
                        return true;
                    }

                    while(++_index < _childrenArray.length)
                    {
                        _p = _childrenArray[_index].iterator();
                        if(_p.hasNext())
                        {
                            return true;
                        }
                    }
                    return false;
                }

                @Override
                public javax.swing.tree.TreeNode nextElement()
                {
                    try
                    {
                        return _p.next();
                    }
                    catch(java.util.NoSuchElementException nse)
                    {
                        if(hasMoreElements())
                        {
                            return _p.next();
                        }
                        else
                        {
                            throw nse;
                        }
                    }
                }

                private int _index = 0;
                private java.util.Iterator<javax.swing.tree.TreeNode> _p = _childrenArray[0].iterator();
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
        int offset = 0;
        for(java.util.List l : _childrenArray)
        {
            if(childIndex < offset + l.size())
            {
                return (javax.swing.tree.TreeNode)l.get(childIndex - offset);
            }
            else
            {
                offset += l.size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }

    @Override
    public int getChildCount()
    {
        int result = 0;
        for(java.util.List l : _childrenArray)
        {
            result += l.size();
        }
        return result;
    }

    @Override
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        int offset = 0;
        for(java.util.List l : _childrenArray)
        {
            int index = l.indexOf(node);
            if(index == -1)
            {
                offset += l.size();
            }
            else
            {
                return offset + index;
            }
        }
        return -1;
    }

    @Override
    public boolean isLeaf()
    {
        for(java.util.List l : _childrenArray)
        {
            if(!l.isEmpty())
            {
                return false;
            }
        }
        return true;
    }

    protected ListArrayTreeNode(TreeNode parent, String id, int arraySize)
    {
        super(parent, id);
        _childrenArray = new java.util.List[arraySize];
    }

    protected final java.util.List[] _childrenArray;
}
