// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ApplicationDescriptor;
import IceGrid.TreeModelI;

public class NodeViewRoot extends Parent
{
    public NodeViewRoot()
    {
	super(TreeModelI.NODE_VIEW);
    }

    public void init(ApplicationDescriptor[] applications)
    {
	assert(_children.size() == 0);
	
	for(int i = 0; i < applications.length; ++i)
	{
	    for(int j = 0 ; j < applications[i].nodes.length; ++j)
	    {
		String nodeName = applications[i].nodes[j].name;
		if(!_nodeMap.containsKey(nodeName))
		{
		    Node child = new Node(nodeName);
		    addChild(child);
		    child.addParent(this);
		    _nodeMap.put(nodeName, child);
		}
	    }
	}
    }
    
    public void clear()
    {
	_nodeMap.clear();
	clearChildren();
	fireStructureChanged(this);
    }


    Node findNode(String name)
    {
	return (Node)_nodeMap.get(name);
    }

    public String toString()
    {
	return "Nodes";
    }

    private java.util.Map _nodeMap = new java.util.HashMap();
}
