// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.AdapterDynamicInfo;
import IceGrid.ApplicationDescriptor;
import IceGrid.ApplicationUpdateDescriptor;
import IceGrid.Model;
import IceGrid.NodeDynamicInfo;
import IceGrid.ServerDynamicInfo;
import IceGrid.ServerState;
import IceGrid.TreeModelI;


public class Root extends Parent
{
    static class AdapterInstanceId
    {
	AdapterInstanceId(String serverId, String adapterId)
	{
	    this.serverId = serverId;
	    this.adapterId = adapterId;
	}

	public boolean equals(Object obj)
	{
	    try
	    {
		AdapterInstanceId other = (AdapterInstanceId)obj;
		return serverId.equals(other.serverId) 
		    && adapterId.equals(other.adapterId);
	    }
	    catch(ClassCastException e)
	    {
		return false;
	    }
	}
	
	public int hashCode()
	{
	    return serverId.hashCode() ^ adapterId.hashCode();
	}
	
	String serverId;
	String adapterId;
    };

    static class DynamicInfo
    {
	//
	// ServerId to ServerDynamicInfo
	//
	java.util.Map serverInfoMap = new java.util.HashMap();

	//
	// AdapterInstanceId to Ice.ObjectPrx
	//
	java.util.Map adapterInfoMap = new java.util.HashMap();
    };


    public Root(Model model)
    {
	super("Applications", model, true);
    }

    public void init(java.util.List descriptors)
    {
	assert(_children.size() == 0);
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ApplicationDescriptor descriptor = (ApplicationDescriptor)p.next();
	    Application child = new Application(descriptor, _model);
	    addChild(child);
	    child.setParent(this);
	}
	fireStructureChangedEvent(this);
	expandChildren();
    }
    
    public void clear()
    {
	clearChildren();
	fireStructureChangedEvent(this);
    }

    public void applicationAdded(ApplicationDescriptor desc)
    {
	Application child = new Application(desc, _model); 
	child.setParent(this);
	addChild(child, true);
	expandChildren();
    }
 
    public void applicationRemoved(String name)
    {
	Application application = (Application)findChild(name);
	removeChild(name, true);
    }
    
    public void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	Application application = (Application)findChild(desc.name);
	application.update(desc);
	expandChildren();
    }

    public void nodeUp(NodeDynamicInfo updatedInfo)
    {
	String nodeName = updatedInfo.name;

	DynamicInfo info = new DynamicInfo();
	_dynamicInfoMap.put(nodeName, info);

	for(int i = 0; i < updatedInfo.servers.length; ++i)
	{
	    assert(updatedInfo.servers[i].state != ServerState.Destroyed);
	    assert(updatedInfo.servers[i].state != ServerState.Inactive);
	    info.serverInfoMap.put(updatedInfo.servers[i].id, updatedInfo.servers[i]);
	}
	
	//
	// Need to tell *every* server on this node
	//
	java.util.List serverList = (java.util.List)_nodeServerMap.get(nodeName);
	if(serverList != null)
	{
	    java.util.Iterator p = serverList.iterator();
	    while(p.hasNext())
	    {
		Server server = (Server)p.next();
		ServerDynamicInfo serverInfo = (ServerDynamicInfo)info.
		    serverInfoMap.get(server.getId());
		if(serverInfo == null)
		{
		    server.updateDynamicInfo(ServerState.Inactive, 0);
		}
		else
		{
		    server.updateDynamicInfo(serverInfo.state, serverInfo.pid);
		}
	    }
	}

	for(int i = 0; i < updatedInfo.adapters.length; ++i)
	{
	    AdapterInstanceId instanceId = new AdapterInstanceId(
		"" /* updatedInfo.adapters[i].serverId */, updatedInfo.adapters[i].id);

	    info.adapterInfoMap.put(instanceId, updatedInfo.adapters[i].proxy);

	    Adapter adapter = (Adapter)_adapterMap.get(instanceId);
	    if(adapter != null)
	    {
		adapter.updateProxy(updatedInfo.adapters[i].proxy);
	    }
	}

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    application.nodeUp(nodeName);
	}
    }

    public void nodeDown(String nodeName)
    {
	_dynamicInfoMap.remove(nodeName);

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    application.nodeDown(nodeName);
	}

	java.util.List serverList = (java.util.List)_nodeServerMap.get(nodeName);
	if(serverList != null)
	{
	    while(p.hasNext())
	    {
		Server server = (Server)p.next();
		server.updateDynamicInfo(null, 0);
	    }
	}

	java.util.List adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	if(adapterList != null)
	{
	    while(p.hasNext())
	    {
	        Adapter adapter = (Adapter)p.next();
		adapter.updateProxy(null);
	    }
	}
    }

    public java.util.Set getNodesUp()
    {
	return _dynamicInfoMap.keySet();
    }

    public void updateServer(String nodeName, ServerDynamicInfo updatedInfo)
    {
	//
	// This node must be up
	//
	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	assert info != null;
	info.serverInfoMap.put(updatedInfo.id, updatedInfo);
	
	//
	// Is a corresponding Server registered?
	//
	Server server = (Server)_serverMap.get(updatedInfo.id);
	if(server != null)
	{
	    server.updateDynamicInfo(updatedInfo.state, updatedInfo.pid);
	}
    }

    public void updateAdapter(String nodeName, AdapterDynamicInfo updatedInfo)
    {
	//
	// This node must be up
	//
	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	assert info != null;
	AdapterInstanceId instanceId 
	    = new AdapterInstanceId("" /* updatedInfo.serverId */, updatedInfo.id);
	info.adapterInfoMap.put(instanceId, updatedInfo.proxy);
	
	//
	// Is a corresponding Adapter registered?
	//
	Adapter adapter = (Adapter)_adapterMap.get(instanceId);
	if(adapter != null)
	{
	    adapter.updateProxy(updatedInfo.proxy);
	}
    }
    
    Ice.ObjectPrx registerAdapter(String nodeName, String serverId, String adapterId,
				  Adapter adapter)
    {
	AdapterInstanceId instanceId = new AdapterInstanceId(serverId, adapterId);

	_adapterMap.put(instanceId, adapter);
	
	java.util.List adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	if(adapterList == null)
	{
	    adapterList = new java.util.LinkedList();
	    _nodeAdapterMap.put(nodeName, adapterList);
	}
	adapterList.add(adapter);
	
	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	if(info == null)
	{
	    // Node is down
	    return null;
	}
	else
	{
	    return (Ice.ObjectPrx)info.adapterInfoMap.get(instanceId);
	}
    }
    
    void unregisterAdapter(String nodeName, String serverId, String adapterId, 
			   Adapter adapter)
    {
	AdapterInstanceId instanceId = new AdapterInstanceId(serverId, adapterId);
	_adapterMap.remove(instanceId);

	java.util.List adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	if(adapterList != null)
	{
	    adapterList.remove(adapter);
	}
    }

    ServerState registerServer(String nodeName, String serverId, Server server,
			       Ice.IntHolder pid)
    {
	_serverMap.put(serverId, server);
	
	java.util.List serverList = (java.util.List)_nodeServerMap.get(nodeName);
	if(serverList == null)
	{
	    serverList = new java.util.LinkedList();
	    _nodeServerMap.put(nodeName, serverList);
	}
	serverList.add(server);

	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	if(info == null)
	{	
	    // Node is down
	    pid.value = 0;
	    return null;
	}
	else
	{
	    ServerDynamicInfo serverInfo = (ServerDynamicInfo)info.
		serverInfoMap.get(serverId);
	    if(serverInfo == null)
	    {
		pid.value = 0;
		return ServerState.Inactive;
	    }
	    else
	    {
		pid.value = serverInfo.pid;
		return serverInfo.state;
	    }
	}
    }
    
    void unregisterServer(String nodeName, String serverId, Server server)
    {
	_serverMap.remove(serverId);
	java.util.List serverList = (java.util.List)_nodeServerMap.get(nodeName);
	if(serverList != null)
	{
	    serverList.remove(server);
	}
    }

    
    //
    // Nodename to DynamicInfo
    //
    private java.util.Map _dynamicInfoMap = new java.util.HashMap();

    //
    // AdapterInstanceId to Adapter
    //
    private java.util.Map _adapterMap = new java.util.HashMap();

    //
    // Nodename to list of Adapter (used when a node goes down)
    //
    private java.util.Map _nodeAdapterMap = new java.util.HashMap();

    //
    // ServerId to Server
    //
    private java.util.Map _serverMap = new java.util.HashMap();

    //
    // Nodename to list of Server (used when a node goes down)
    //
    private java.util.Map _nodeServerMap = new java.util.HashMap();
}
