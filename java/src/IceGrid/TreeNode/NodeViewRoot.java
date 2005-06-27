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
import IceGrid.NodeDescriptor;
import IceGrid.TreeModelI;

public class NodeViewRoot extends Parent
{
    public NodeViewRoot()
    {
	super("Nodes", TreeModelI.NODE_VIEW);
    }

    public void init(java.util.List applications)
    {
	assert(_children.size() == 0);
	java.util.Iterator p = applications.iterator();
	while(p.hasNext())
	{
	    ApplicationDescriptor descriptor = (ApplicationDescriptor)p.next();
	    put(descriptor.name, descriptor.nodes, false);
	}
    }
    
    public void clear()
    {
	_nodeMap.clear();
	clearChildren();
	fireStructureChangedEvent(this);
    }

    public void put(String applicationName, java.util.List nodeDescriptors,
		    boolean fireEvent)
    {
	java.util.Iterator p = nodeDescriptors.iterator();

	while(p.hasNext())
	{
	    NodeDescriptor descriptor = (NodeDescriptor)p.next();
	    String nodeName = descriptor.name;
	    
	    Node child = (Node)findChild(nodeName);
	    if(child == null)
	    {
		child = new Node(applicationName, descriptor);
		_nodeMap.put(nodeName, child);
		child.addParent(this);
		addChild(child, fireEvent);
	    }
	    else
	    {
		child.addApplication(applicationName, 
				     descriptor);
	    }
	}
    }

    public void remove(String applicationName)
    {
	int i = 0;
	java.util.Iterator p = _children.iterator();

	while(p.hasNext())
	{
	    Node child = (Node)p.next();
	    if(child.removeApplication(applicationName))
	    {
		_nodeMap.remove(child.getId());
		p.remove();
		fireNodeRemovedEvent(this, child, i);
	    }
	    else
	    {
		++i;
	    }
	}
    }
    
    public void remove(String applicationName, String nodeName)
    {
	Node child = (Node)_nodeMap.get(nodeName);
	assert(child != null);

	if(child.removeApplication(applicationName))
	{
	    _nodeMap.remove(nodeName);
	    removeChild(nodeName, true);
	}
    }

    public void removeServers(String[] removeServers)
    {
	if(removeServers.length > 0)
	{
	    java.util.List serverList = 
		new java.util.LinkedList(java.util.Arrays.asList(removeServers));
	    
	    java.util.Iterator p = _nodeMap.values().iterator();
	    while(p.hasNext() && serverList.size() > 0)
	    {
		Node node = (Node)p.next();
		node.removeChildren(serverList);
	    }
	}
    }


    Node findNode(String name)
    {
	return (Node)_nodeMap.get(name);
    }

    private java.util.Map _nodeMap = new java.util.HashMap();
}
