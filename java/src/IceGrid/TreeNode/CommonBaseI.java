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
import javax.swing.JPopupMenu;
import javax.swing.JPanel;
import javax.swing.event.TreeModelEvent;

import IceGrid.TreeModelI;

abstract class CommonBaseI implements CommonBase
{ 
    static class ViewData
    {
	public TreePath path;
	public CommonBase parent;
    }

    public void addParent(CommonBase parent)
    {
	//
	// Check if this parent has any view; there is no point in remembering
	// a parent that does not belong to a view: getParent etc is used
	// only to create event to fire using the view's TreeModel.
	//
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
    }

    public TreePath getPath(int view)
    {
	return _paths[view];
    }
    
    public CommonBase getParent(int view)
    {
	return _parents[view];
    }


    public JPopupMenu getPopupMenu()
    {
	//
	// Default = no popup menu
	//
	return null;
    }

    public JPanel getProperties(int view)
    {
	//
	// TODO: nicer emptyx panel
	//
	return null;
    }


    //
    // Fires a nodesChanged event with this node for this specific view
    //
    void fireNodesChanged(Object source, int view)
    {
	//
	// Bug if I am not in this view
	//
	assert(_paths[view] != null);

	int[] childIndices = new int[1];
	Object[] children = new Object[1];
	children[0] = this;

	TreeModelEvent event;

	if(_parents[view] == null)
	{	 
	    //
	    // I am root
	    //
	    childIndices[0] = 0;
	    Object[] path = null;
	    event = new TreeModelEvent(source, path, childIndices, children); 
	}
	else
	{
	    childIndices[0] = _parents[view].getIndex(this);
	    event = new TreeModelEvent(source, _parents[view].getPath(view), childIndices, children);
	}
	TreeModelI.getTreeModel(view).fireNodesChanged(event);
    } 
    
    //
    // Fires a nodesChanged event with this node for all my views (usually just one)
    //
    void fireNodesChanged(Object source)
    {
	for(int i = 0; i < TreeModelI.VIEW_COUNT; ++i)
	{
	    if(_paths[i] != null)
	    {
		fireNodesChanged(source, i);
	    }
	}
    } 

    
    void fireStructureChanged(Object source, int view)
    {
	//
	// Bug if I am not in this view
	//
	assert(_paths[view] != null);

	TreeModelEvent event = new TreeModelEvent(source, _paths[view]);
	TreeModelI.getTreeModel(view).fireStructureChanged(event);
    } 
    
    void fireStructureChanged(Object source)
    {
	for(int i = 0; i < TreeModelI.VIEW_COUNT; ++i)
	{
	    if(_paths[i] != null)
	    {
		fireStructureChanged(source, i);
	    }
	}
    } 

    static String templateLabel(String templateName, String[] parameters)
    {
	String result = templateName + "<";
	int i = 0;
	while(i < parameters.length)
	{
	    result += parameters[i++];
	    if(i < parameters.length)
	    {
		result += ", ";
	    }
	}
	result += ">";
	return result;
    }

    static String templateLabel(String templateName, String[] parameters, java.util.Map parameterValues)
    {
	String result = templateName + "<";
	int i = 0;
	while(i < parameters.length)
	{
	    String value = (String)parameterValues.get(parameters[i]);
	    if(value == null)
	    {
		value = "${" + parameters[i] + "}";
	    }
	    result += value;
	    ++i;
	    if(i < parameters.length)
	    {
		result += ", ";
	    }
	}
	result += ">";
	return result;
    }

    protected CommonBaseI(int rootForView)
    {
	if(rootForView >= 0)
	{
	    _paths[rootForView] = new TreePath(this);
	}
    }

    //
    // view to Path/Parent arrays
    //
    protected TreePath[] _paths = new TreePath[TreeModelI.VIEW_COUNT];
    protected CommonBase[] _parents = new CommonBase[TreeModelI.VIEW_COUNT]; 
}
