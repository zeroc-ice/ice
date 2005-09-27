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
import IceGrid.NodeDescriptor;
import IceGrid.NodeDynamicInfo;
import IceGrid.NodeUpdateDescriptor;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ServerState;

public class Nodes extends EditableParent
{
    public Nodes(java.util.Map nodeMap, Application application)
	throws UpdateFailedException
    {
	super(false, "Nodes", application.getModel());
	_descriptors = nodeMap;

	java.util.Iterator p = nodeMap.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String nodeName = (String)entry.getKey();
	    NodeDescriptor nodeDescriptor = (NodeDescriptor)entry.getValue();
	    addChild(new Node(false, nodeName, nodeDescriptor, application));
	}
	
	//
	// Also create a Node for each node that is up
	//
	Root root = _model.getRoot();
	p = root.getNodesUp().iterator();
	while(p.hasNext())
	{
	    String nodeName = (String)p.next();
	    if(findChild(nodeName) == null)
	    {
		addChild(new Node(nodeName, application.getModel()));
	    }
	}
    }

    Nodes(Nodes o)
    {
	super(o);
	_descriptors = o._descriptors;

	//
	// Deep-copy children
	//
	java.util.Iterator p = o._children.iterator();
	while(p.hasNext())
	{
	    try
	    {
		addChild(new Node((Node)p.next()));
	    }
	    catch(UpdateFailedException e)
	    {
		assert false;
	    }
	}
    }

    void getUpdates(java.util.LinkedList updates)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Node node = (Node)p.next();
	    if(node.isNew() || node.isModified())
	    {
		updates.add(node.getUpdate());
	    }
	}
    }

    void update() throws UpdateFailedException
    {
	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String nodeName = (String)entry.getKey();
	    NodeDescriptor nodeDescriptor = (NodeDescriptor)entry.getValue();

	    Node node = findNode(nodeName);
	    if(node != null)
	    {
		node.update();
	    }
	}
	purgeChildren(_descriptors.keySet());
	fireStructureChangedEvent(this);
    }
   
    void removeServerInstances(String templateId)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Node node = (Node)p.next();
	    node.removeServerInstances(templateId);
	}
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Node node = (Node)p.next();
	    result.addAll(node.findServiceInstances(template));
	}
	return result;
    }


    void removeServiceInstances(String templateId)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Node node = (Node)p.next();
	    node.removeServiceInstances(templateId);
	}
    }

    void update(java.util.List updates, String[] removeNodes)
	throws UpdateFailedException
    {
	Application application = (Application)getParent();

	//
	// Note: _descriptors has already the nodes removed but
	// needs to get the updated and new NodeDescriptors
	//

	//
	// One big set of removes
	//
	removeChildren(removeNodes);
	
	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.iterator();
	while(p.hasNext())
	{
	    NodeUpdateDescriptor update = (NodeUpdateDescriptor)p.next();
	    Node node = findNode(update.name);
	    if(node == null)
	    {
		NodeDescriptor nodeDescriptor = new NodeDescriptor(update.variables,
								   update.serverInstances,
								   update.servers,
								   update.loadFactor.value);
		_descriptors.put(update.name, nodeDescriptor);
		node = new Node(false, update.name, nodeDescriptor, application);
		newChildren.add(node);
	    }
	    else
	    {
		NodeDescriptor nodeDescriptor = node.update(update, application);
		if(nodeDescriptor != null)
		{
		    _descriptors.put(update.name, nodeDescriptor);
		}
	    }
	}
	
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    void nodeUp(String nodeName)
    {
	Node node = findNode(nodeName);
	if(node == null)
	{
	    node = new Node(nodeName, _model);
	    try
	    {
		addChild(node, true);
	    }
	    catch(UpdateFailedException e)
	    {
		// Impossible
		assert false;
	    }
	}
	else
	{
	    node.up();
	}
    }

    void nodeDown(String nodeName)
    {
	//
	// We remove nodes that are down and without descriptor
	//
	Node node = findNode(nodeName);
	if(node != null)
	{
	    if(node.down())
	    {
		removeChild(nodeName, true);
	    }
	}
	//
	// Else log a warning?
	//
    }

    Node findNode(String nodeName)
    {
	return (Node)findChild(nodeName);
    }

    java.util.List findServerInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Node node = (Node)p.next();
	    result.addAll(node.findServerInstances(template));
	}
	return result;
    }

    private java.util.Map _descriptors;
}
