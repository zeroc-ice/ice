// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.Icon;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.NodeDescriptor;
import IceGrid.Utils;
import IceGrid.Model;
import IceGrid.NodeDynamicInfo;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ServerState;

class Node extends Parent
{
    //
    // Node creation/deletion/renaming is done by starting/restarting
    // an IceGridNode process. Not through admin calls.
    //
    
    //
    // TODO: consider showing per-application node variables
    //

     public Component getTreeCellRendererComponent(
	    JTree tree,
	    Object value,
	    boolean sel,
	    boolean expanded,
	    boolean leaf,
	    int row,
	    boolean hasFocus) 
    {
	if(_cellRenderer == null)
	{
	    //
	    // Initialization
	    //
	    _cellRenderer = new DefaultTreeCellRenderer();
	    _nodeUpOpen = Utils.getIcon("/icons/node_up_open.png");
	    _nodeDownOpen = Utils.getIcon("/icons/node_down_open.png");
	    _nodeUpClosed = Utils.getIcon("/icons/node_up_closed.png");
	    _nodeDownClosed = Utils.getIcon("/icons/node_down_closed.png");
	}

	//
	// TODO: separate icons for open and close
	//
	if(_serverInfoMap != null) // up
	{
	    _cellRenderer.setToolTipText("Up and running");
	    if(expanded)
	    {
		_cellRenderer.setOpenIcon(_nodeUpOpen);
	    }
	    else
	    {
		_cellRenderer.setClosedIcon(_nodeUpClosed);
	    }
	}
	else
	{
	    _cellRenderer.setToolTipText("Not running");
	    if(expanded)
	    {
		_cellRenderer.setOpenIcon(_nodeDownOpen);
	    }
	    else
	    {
		_cellRenderer.setClosedIcon(_nodeDownClosed);
	    }
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }
    
    
    void up(java.util.Map serverInfoMap, java.util.Map adapterInfoMap)
    {
	_serverInfoMap = serverInfoMap;
	_adapterInfoMap = adapterInfoMap;
	
	//
	// Update the state of all servers
	//
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerInstance child = (ServerInstance)p.next();
	    ServerDynamicInfo info = 
		(ServerDynamicInfo)_serverInfoMap.get(child.getId());

	    if(info == null)
	    {
		info = _inactiveServerDynamicInfo;
	    }
	    child.updateDynamicInfo(info);
	}
	
	//
	// Update the state of all adapters
	//
	/*
	p = _adapters.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String id = (String) entry.getKey();
	    Adapter adapter = (Adapter)entry.getValue();
	    Ice.ObjectPrx proxy = (Ice.ObjectPrx)_adapterInfoMap.get(id);
	    adapter.updateProxy(proxy);
	}
	*/

	fireNodeChangedEvent(this);
    }

    void down()
    {
	_serverInfoMap = null;
	_adapterInfoMap = null;

	//
	// Update the state of all servers
	//
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerInstance child = (ServerInstance)p.next();
	    child.updateDynamicInfo(_unknownServerDynamicInfo);
	}

	//
	// Update the state of all adapters
	//
	/*
	p = _adapters.values().iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.updateProxy(null);
	}
	*/

	fireNodeChangedEvent(this);
    }
  
    ServerDynamicInfo getServerDynamicInfo(String serverName)
    {
	if(_serverInfoMap == null)
	{
	    return _unknownServerDynamicInfo;
	}
	else
	{
	    Object obj = _serverInfoMap.get(serverName);
	    if(obj == null)
	    {
		return _inactiveServerDynamicInfo;
	    }
	    else
	    {
		return (ServerDynamicInfo)obj;
	    }
	}
    }
    
    /*
    ServerInstances(java.util.List descriptors, 
		    Application application,
		    boolean fireEvent)
    {
	super("Server instances", application.getModel());
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    //
	    // The ServerInstance constructor inserts the new object in the 
	    // node view model
	    //
	    ServerInstanceDescriptor descriptor = 
		(ServerInstanceDescriptor)p.next();

	    

	    String serverName = computeServerName(descriptor, application);
		
	    ServerInstance child = new ServerInstance(serverName,
						      descriptor,
						      application,
						      fireEvent);
	    addChild(child);
	}
    }

    void update(java.util.List updates, String[] removeServers)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	Application application = (Application)getParent(TreeModelI.APPLICATION_VIEW);

	//
	// One big set of removes
	//
	for(int i = 0; i < removeServers.length; ++i)
	{
	    ServerInstance server = (ServerInstance)findChild(removeServers[i]);
	    server.removeFromNode();
	}
	removeChildren(removeServers);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor descriptor = (ServerInstanceDescriptor)p.next();
	    
	    String serverName = computeServerName(descriptor, application);

	    ServerInstance child = (ServerInstance)findChild(serverName);
	    if(child == null)
	    {
		newChildren.add(new ServerInstance(serverName, descriptor, application, true));
	    }
	    else
	    {
		child.rebuild(application, descriptor, true);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    void removeFromNodes()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerInstance server = (ServerInstance)p.next();
	    server.removeFromNode();
	}
    }


    static String computeServerName(ServerInstanceDescriptor instanceDescriptor, 
				    Application application)
    {
	String nodeName = instanceDescriptor.node;

	if(instanceDescriptor.template.length() > 0)
	{
	    //
	    // Can't be null
	    //
	    TemplateDescriptor templateDescriptor = 
		application.findServerTemplateDescriptor(instanceDescriptor.template);
	    
	    java.util.Map parameters = 
		Utils.substituteVariables(instanceDescriptor.parameterValues,
					  application.getNodeVariables(nodeName),
					  application.getVariables());

	    return Utils.substituteVariables(templateDescriptor.descriptor.name,
					     parameters,
					     application.getNodeVariables(nodeName),
					     application.getVariables());
	}
	else
	{
	  
	    return Utils.substituteVariables(instanceDescriptor.descriptor.name,
					     application.getNodeVariables(nodeName),
					     application.getVariables());
	}
    }

    */
  

    java.util.Map  getVariables()
    {
	return null;
    }


    Node(String applicationName, Model model, 
	 String nodeName, NodeDescriptor descriptor,
	 java.util.Map serverInfoMap, java.util.Map adapterInfoMap)
    {
	super(nodeName, model);
	_serverInfoMap = serverInfoMap;
	_adapterInfoMap = adapterInfoMap;
    } 
    
    private java.util.Map _serverInfoMap;
    private java.util.Map _adapterInfoMap;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUpOpen;
    static private Icon _nodeUpClosed;
    static private Icon _nodeDownOpen;
    static private Icon _nodeDownClosed;

    static private ServerDynamicInfo _unknownServerDynamicInfo = 
       new ServerDynamicInfo();

    static private ServerDynamicInfo _inactiveServerDynamicInfo = 
       new ServerDynamicInfo(null, ServerState.Inactive, 0);
}
