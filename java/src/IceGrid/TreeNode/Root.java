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


public class Root extends EditableParent
{
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
	super(false, "Applications", model, true);
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
		 Application child = new Application(descriptor, _model);
		addChild(child);
		child.setParent(this);
	    }
	    catch(DuplicateIdException e)
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
	    Application child = new Application(desc, _model); 
	    child.setParent(this);
	    addChild(child, true);
	}
	catch(DuplicateIdException e)
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
	catch(DuplicateIdException e)
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
	    AdapterInstanceId instanceId = new AdapterInstanceId(
		updatedInfo.adapters[i].serverId, updatedInfo.adapters[i].id);
	    info.adapterInfoMap.put(instanceId, updatedInfo.adapters[i].proxy);
	}

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    application.nodeUp(nodeName, info);
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
	
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    if(application.updateServer(updatedInfo))
	    {
		break; // while
	    }
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
	
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Application application = (Application)p.next();
	    if(application.updateAdapter(instanceId, updatedInfo.proxy))
	    {
		break; // while
	    }
	}
    }
    
    DynamicInfo getDynamicInfo(String nodeName)
    {
	return (DynamicInfo)_dynamicInfoMap.get(nodeName);
    }

    void restore(Application copy)
    {
	//
	// TODO: fire event or not?
	//
	removeChild(copy.getId(), true);
	try
	{
	    addChild(copy, true);
	}
	catch(DuplicateIdException e)
	{
	    assert false; // impossible
	}
    }
    
    //
    // Nodename to DynamicInfo
    //
    private java.util.Map _dynamicInfoMap = new java.util.HashMap();

}
