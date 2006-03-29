// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI;

import java.util.Enumeration;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.JPopupMenu;

//
// This class behaves like a leaf; derived class that represent non-leaf nodes must 
// override various methods.
//
public class TreeNodeBase implements javax.swing.tree.TreeNode, TreeCellRenderer 
{
    public Coordinator getCoordinator()
    {
	return _parent.getCoordinator();
    }

    public Enumeration children()
    {
	return new Enumeration()
	    {
		public boolean hasMoreElements()
		{
		    return false;
		}

		public Object nextElement()
		{
		    throw new java.util.NoSuchElementException();
		}
	    };
    }
    
    public boolean getAllowsChildren()
    {
	return false;
    }
    
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
	return null;
    }
   
    public int getChildCount()
    {
	return 0;
    }

    public int getIndex(javax.swing.tree.TreeNode node)
    {
	return -1;
    }
        
    public javax.swing.tree.TreeNode getParent()
    {
	return _parent;
    }

    public boolean isLeaf()
    {
	return true;
    }

    public Component getTreeCellRendererComponent(
	JTree tree,
	Object value,
	boolean sel,
	boolean expanded,
	boolean leaf,
	int row,
	boolean hasFocus) 
    {
	return null;
    }

    public String toString()
    {
	return _id;
    }

    public JPopupMenu getPopupMenu()
    {
	return null;
    }

    public String getId()
    {
	return _id;
    }
    
    public TreePath getPath()
    {
	if(_parent == null)
	{
	    return new TreePath(this);
	}
	else
	{
	    return _parent.getPath().pathByAddingChild(this);
	}
    }

    public java.util.LinkedList getFullId()
    {
	java.util.LinkedList result = _parent == null ?
	    new java.util.LinkedList() :
	    _parent.getFullId();

	result.add(_id);
	return result;
    }

    public TreeNodeBase findChild(String id)
    {
	Enumeration p = children();
	while(p.hasMoreElements())
	{
	    TreeNodeBase child = (TreeNodeBase)p.nextElement();
	    if(child.getId().equals(id))
	    {
		return child;
	    }
	}
	return null;
    }

    protected TreeNodeBase(TreeNodeBase parent, String id)
    {
	_parent = parent;
	_id = id;
    }

    //
    // Helper functions
    //
    protected boolean insertSortedChild(TreeNodeBase newChild, java.util.List children,
					DefaultTreeModel treeModel)
    {
	String id = newChild.getId();
	int i;
	for(i = 0; i < children.size(); ++i)
	{
	    String otherId = ((TreeNodeBase)children.get(i)).getId();
	    int cmp = id.compareTo(otherId);

	    if(cmp == 0)
	    {
		return false;
	    }
	    else if(cmp < 0)
	    {
		break;
	    }
	}
	children.add(i, newChild);

	if(treeModel != null)
	{
	    treeModel.nodesWereInserted(this, new int[]{getIndex(newChild)});
	}
	return true;
    }
    

    protected void removeSortedChildren(String[] childIds, 
					java.util.List fromChildren, 
					DefaultTreeModel treeModel)
    {
	if(childIds.length == 0)
	{
	    return; // nothing to do
	}
	assert fromChildren.size() > 0;

	String[] ids = (String[])childIds.clone();
	java.util.Arrays.sort(ids);
	
	Object[] childrenToRemove = new Object[ids.length];
	int[] indices = new int[ids.length];
	    
	int i = getIndex((TreeNodeBase)fromChildren.get(0));
	int j = 0;
	java.util.Iterator p = fromChildren.iterator();

	while(p.hasNext() && j < ids.length)
	{
	    TreeNodeBase child = (TreeNodeBase)p.next();	    
	    if(ids[j].equals(child.getId()))
	    {
		childrenToRemove[j] = child;
		indices[j] = i;
		p.remove();
		++j;
	    }
	    ++i;
	}
	    
	//
	// Should be all removed
	//
	assert(j == ids.length);

	if(treeModel != null)
	{
	    treeModel.nodesWereRemoved(this, indices, childrenToRemove);
	}
    }

    protected void childrenChanged(java.util.List children, DefaultTreeModel treeModel)
    {
	int[] indices = new int[children.size()];
	int i = 0;
	java.util.Iterator p = children.iterator();
	while(p.hasNext())
	{
	    TreeNodeBase child = (TreeNodeBase)p.next();
	    indices[i++] = getIndex(child);
	}
	treeModel.nodesChanged(this, indices);
    }
    
    public int[] resize(int[] array, int size)
    {
	int[] result = new int[size];
	System.arraycopy(array, 0, result, 0, size);
	return result;
    }

    protected TreeNodeBase find(String id, java.util.List inList)
    {
	java.util.Iterator p = inList.iterator();
	while(p.hasNext())
	{
	    TreeNodeBase node = (TreeNodeBase)p.next();
	    if(node.getId().equals(id))
	    {
		return node;
	    }
	}
	return null;
    }

    protected TreeNodeBase _parent;
    protected String _id;

    protected java.util.Comparator _childComparator = new java.util.Comparator()
	{
	    public int compare(Object o1, Object o2)
	    {
		TreeNodeBase child1 = (TreeNodeBase)o1;
		TreeNodeBase child2 = (TreeNodeBase)o2;
		return child1.getId().compareTo(child2.getId());
	    }
	};
}
