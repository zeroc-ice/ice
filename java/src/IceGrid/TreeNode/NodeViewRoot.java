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
import IceGrid.Model;
import IceGrid.NodeDynamicInfo;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ServerState;

public class NodeViewRoot extends Parent
{
    public NodeViewRoot(Model model)
    {
	super("Nodes", model, TreeModelI.NODE_VIEW);
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
	_nodeServerInfoMap.clear();
	_nodeAdapterInfoMap.clear();
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
		java.util.Map serverInfoMap = (java.util.Map)_nodeServerInfoMap.get(nodeName);
		java.util.Map adapterInfoMap = (java.util.Map)_nodeAdapterInfoMap.get(nodeName);
		child = new Node(applicationName, _model, descriptor, serverInfoMap, adapterInfoMap);
		_nodeMap.put(nodeName, child);
		child.addParent(this);
		addChild(child, fireEvent);
	    }
	    else
	    {
		child.addApplication(applicationName, descriptor);
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

    public void nodeUp(NodeDynamicInfo updatedInfo)
    {
	String nodeName = updatedInfo.name;

	java.util.Map serverMap = new java.util.HashMap();
	System.err.println("nodeUp gives info on:");
	for(int i = 0; i < updatedInfo.servers.length; ++i)
	{
	    System.err.println(updatedInfo.servers[i].name);
	    if(updatedInfo.servers[i].state != ServerState.Destroyed)
	    {
		serverMap.put(updatedInfo.servers[i].name, updatedInfo.servers[i]);
	    }
	}

	java.util.Map adapterMap = new java.util.HashMap();
	for(int i = 0; i < updatedInfo.adapters.length; ++i)
	{
	    if(updatedInfo.adapters[i].proxy != null)
	    {
		adapterMap.put(updatedInfo.adapters[i].id, updatedInfo.adapters[i].proxy);
	    }
	}
	
	_nodeServerInfoMap.put(nodeName, serverMap);
	_nodeAdapterInfoMap.put(nodeName, adapterMap);

	Node node = findNode(nodeName);
	if(node != null)
	{
	    node.up(serverMap, adapterMap);
	}
    }

    public void nodeDown(String nodeName)
    {
	_nodeServerInfoMap.remove(nodeName);
	_nodeAdapterInfoMap.remove(nodeName);

	Node node = findNode(nodeName);
	if(node != null)
	{
	    node.down();
	}
    }
    
    public void updateServer(String nodeName, ServerDynamicInfo updatedInfo)
    {
	java.util.Map serverMap = (java.util.Map)_nodeServerInfoMap.get(nodeName);
	if(serverMap != null)
	{
	    if(updatedInfo.state == ServerState.Destroyed)
	    {
		serverMap.remove(updatedInfo.name);
	    }
	    else
	    {
		serverMap.put(updatedInfo.name, updatedInfo);
	    }
	    
	    Node node = findNode(nodeName);
	    if(node != null)
	    {
		node.updateServer(updatedInfo);
	    }
	}
	//
	// Else log a warning?
	//

    }

    public void updateAdapter(String nodeName, AdapterDynamicInfo updatedInfo)
    {
	java.util.Map adapterMap = (java.util.Map)_nodeAdapterInfoMap.get(nodeName);
	if(adapterMap != null)
	{
	    if(updatedInfo.proxy == null)
	    {
		adapterMap.remove(updatedInfo.id);
	    }
	    else
	    {
		adapterMap.put(updatedInfo.id, updatedInfo.proxy);
	    }

	    Node node = findNode(nodeName);
	    if(node != null)
	    {
		node.updateAdapter(updatedInfo);
	    }
	}
	//
	// Else log a warning?
	//
    }
    

    Node findNode(String name)
    {
	return (Node)_nodeMap.get(name);
    }

    private java.util.Map _nodeMap = new java.util.HashMap();
    private java.util.Map _nodeServerInfoMap = new java.util.HashMap();
    private java.util.Map _nodeAdapterInfoMap = new java.util.HashMap();
}
