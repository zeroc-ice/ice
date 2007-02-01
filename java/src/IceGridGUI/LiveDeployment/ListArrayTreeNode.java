// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
        for(int i = 0; i < _childrenArray.length; ++i)
        {
            if(childIndex < offset + _childrenArray[i].size())
            {
                return (javax.swing.tree.TreeNode)_childrenArray[i].get(childIndex - offset);
            }
            else
            {
                offset += _childrenArray[i].size();
            }
        }
        throw new ArrayIndexOutOfBoundsException(childIndex);
    }
   
    public int getChildCount()
    {
        int result = 0;
        for(int i = 0; i < _childrenArray.length; ++i)
        {
            result += _childrenArray[i].size();
        }
        return result;
    }
    
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        int offset = 0;
        for(int i = 0; i < _childrenArray.length; ++i)
        {
            int index = _childrenArray[i].indexOf(node);
            if(index == -1)
            {
                offset += _childrenArray[i].size();
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
        for(int i = 0; i < _childrenArray.length; ++i)
        {
            if(!_childrenArray[i].isEmpty())
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
