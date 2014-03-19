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
// A TreeNode that holds an array of list of children
//
abstract class ListArrayTreeNode extends TreeNode
{
    public Enumeration children()
    {
        return new Enumeration()
            {
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

                public Object nextElement()
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
                private java.util.Iterator _p = _childrenArray[0].iterator();
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

    public int getChildCount()
    {
        int result = 0;
        for(java.util.List l : _childrenArray)
        {
            result += l.size();
        }
        return result;
    }

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
