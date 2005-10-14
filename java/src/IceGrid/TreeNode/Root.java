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
    static class DynamicInfo
    {
	//
	// ServerId to ServerDynamicInfo
	//
	java.util.Map serverInfoMap = new java.util.HashMap();

	//
	// AdapterId to Ice.ObjectPrx
	//
	java.util.Map adapterInfoMap = new java.util.HashMap();
    };


    public void paste()
    {
	ApplicationDescriptor ad = 
	    (ApplicationDescriptor)_model.getClipboard();
	newApplication(Application.copyDescriptor(ad));
    }
    public void newApplication()
    {
	newApplication(
	    new ApplicationDescriptor("NewApplication",
				      new java.util.TreeMap(),
				      new java.util.LinkedList(),
				      new java.util.HashMap(),
				      new java.util.HashMap(),
				      new java.util.HashMap(),
				      new IceGrid.DistributionDescriptor(
					  "", new java.util.LinkedList()),
				      ""));
    }

    public void newApplication(ApplicationDescriptor descriptor)
    {
	descriptor.name = makeNewChildId(descriptor.name);
	
	Application application = new Application(descriptor, _model);
	try
	{
	    addChild(application, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(application.getPath());
    }

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
	    try
	    {
		Application child = new Application(false, descriptor, _model);
		addChild(child);
	    }
	    catch(UpdateFailedException e)
	    {
		//
		// Bug in the IceGrid registry
		//
		System.err.println("Failed to create application " 
				   + descriptor.name  + ": "
				   + e.toString());
		assert false;
	    }
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
	try
	{
	    Application child = new Application(false, desc, _model); 
	    addChild(child, true);
	}
	catch(UpdateFailedException e)
	{
	    //
	    // Bug in the IceGrid registry
	    //
	    System.err.println("Failed to create application " 
			       + desc.name  + ": "
			       + e.toString());
	    assert false;
	}
	expandChildren();
    }
 
    public void applicationRemoved(String name)
    {
	Application application = (Application)findChild(name);
	removeChild(name, true);
    }
    
    public void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	try
	{
	    Application application = (Application)findChild(desc.name);
	    application.update(desc);
	}
	catch(UpdateFailedException e)
	{
	    //
	    // Bug in the IceGrid registry
	    //
	    System.err.println("Failed to update application " 
			       + desc.name  + ": "
			       + e.toString());
	    assert false;
	}
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
	
	for(int i = 0; i < updatedInfo.adapters.length; ++i)
	{
	    assert(updatedInfo.adapters[i].proxy != null);
	    info.adapterInfoMap.put(updatedInfo.adapters[i].id, updatedInfo.adapters[i].proxy);
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
		ServerDynamicInfo serverInfo = (ServerDynamicInfo)
		    info.serverInfoMap.get(server.getId());
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
	
	//
	// Tell adapters 
	//
	java.util.List adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	if(adapterList != null)
	{
	    java.util.Iterator p = adapterList.iterator();
	    while(p.hasNext())
	    {
		Adapter adapter = (Adapter)p.next();
		Ice.ObjectPrx proxy = 
		    (Ice.ObjectPrx)info.adapterInfoMap.get(adapter.getAdapterId());
		if(proxy != null)
		{
		    adapter.updateProxy(proxy);
		}
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

	java.util.List serverList = (java.util.List)_nodeServerMap.get(nodeName);
	if(serverList != null)
	{
	    java.util.Iterator p = serverList.iterator();
	    while(p.hasNext())
	    {
		Server server = (Server)p.next();
		server.updateDynamicInfo(null, 0);
	    }
	}

	java.util.List adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	if(adapterList != null)
	{
	    java.util.Iterator p = adapterList.iterator();
	    while(p.hasNext())
	    {
	        Adapter adapter = (Adapter)p.next();
		adapter.updateProxy(null);
	    }
	}

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    application.nodeDown(nodeName);
	}
    }

    public java.util.Set getNodesUp()
    {
	return _dynamicInfoMap.keySet();
    }
    
    ServerState registerServer(String nodeName, String serverId, Server server, 
			       Ice.IntHolder pid)
    {
	java.util.List serverList = (java.util.List)_serverMap.get(serverId);
	
	if(serverList == null)
	{
	    serverList = new java.util.LinkedList();
	    _serverMap.put(serverId, serverList);
	}
	serverList.add(server);
	
	serverList = (java.util.List)_nodeServerMap.get(nodeName);
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
	    ServerDynamicInfo serverInfo = 
		(ServerDynamicInfo)info.serverInfoMap.get(serverId);
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
	java.util.List serverList = (java.util.List)_serverMap.get(serverId);
	serverList.remove(server);
	
	serverList = (java.util.List)_nodeServerMap.get(nodeName);
	serverList.remove(server);
    }
    
    public void updateServer(String nodeName, ServerDynamicInfo updatedInfo)
    {
	//
	// This node must be up
	//
	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	assert info != null;
	info.serverInfoMap.put(updatedInfo.id, updatedInfo);
	
	java.util.List serverList = (java.util.List)_serverMap.get(updatedInfo.id);
	if(serverList != null)
	{
	    java.util.Iterator p = serverList.iterator();
	    while(p.hasNext())
	    {
		Server server = (Server)p.next();
		server.updateDynamicInfo(updatedInfo.state, updatedInfo.pid);
	    }
	}
    }


    
    Ice.ObjectPrx registerAdapter(String nodeName, String adapterId,
				  Adapter adapter)
    {
	java.util.List adapterList = (java.util.List)_adapterMap.get(adapterId);

	if(adapterList == null)
	{
	    adapterList = new java.util.LinkedList();
	    _adapterMap.put(adapterId, adapterList);
	}
	adapterList.add(adapter);

	adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
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
	    return (Ice.ObjectPrx)info.adapterInfoMap.get(adapterId);
	}
    }
    
    void unregisterAdapter(String nodeName, 
			   String adapterId, 
			   Adapter adapter)
    {
	java.util.List adapterList = (java.util.List)_adapterMap.
	    get(adapterId);
	adapterList.remove(adapter);

	adapterList = (java.util.List)_nodeAdapterMap.get(nodeName);
	adapterList.remove(adapter);
    }

    public void updateAdapter(String nodeName, 
			      AdapterDynamicInfo updatedInfo)
    {
	//
	// This node must be up
	//
	DynamicInfo info = (DynamicInfo)_dynamicInfoMap.get(nodeName);
	assert info != null;
	info.adapterInfoMap.put(updatedInfo.id, updatedInfo.proxy);
	
	//
	// Is this Adapter registered?
	//
	Adapter adapter = (Adapter)_adapterMap.get(updatedInfo.id);
	if(adapter != null)
	{
	    adapter.updateProxy(updatedInfo.proxy);
	}
    }

    void tryAdd(ApplicationDescriptor ad) throws UpdateFailedException
    {
	try
	{
	    Application app = new Application(true, ad, _model);
	    addChild(app, true);
	    expandChildren();
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}
    }
  

   
    String identify(TreePath path)
    {
	String result = "";
	for(int i = 1; i < path.getPathCount(); ++i)
	{
	    if(!result.equals(""))
	    {
		result += "/";
	    }
	    result += ((CommonBase)path.getPathComponent(i)).getId();
	}
	return result;
    }

    //
    // Nodename to DynamicInfo
    //
    private java.util.Map _dynamicInfoMap = new java.util.HashMap();

    //
    // AdapterId to list of Adapters
    // The registry enforces a single adapter per adapterId;
    // however in order to support copy & paste, we can have temporarily
    // an inconsistency: several adapters with the same xid.
    //
    private java.util.Map _adapterMap = new java.util.HashMap();

    //
    // Nodename to list of Adapter (used when a node goes down)
    //
    private java.util.Map _nodeAdapterMap = new java.util.HashMap();

    //
    // ServerId to list of Servers
    // See _adapterMap comment above
    //
    private java.util.Map _serverMap = new java.util.HashMap();

    //
    // Nodename to list of Server (used when a node goes down)
    //
    private java.util.Map _nodeServerMap = new java.util.HashMap();

}
