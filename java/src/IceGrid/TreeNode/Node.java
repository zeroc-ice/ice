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
		info = _unknownServerDynamicInfo;
	    }
	    child.updateDynamicInfo(info);
	}
	
	//
	// Update the state of all adapters
	//
	p = _adapters.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String id = (String) entry.getKey();
	    Adapter adapter = (Adapter)entry.getValue();
	    Ice.ObjectPrx proxy = (Ice.ObjectPrx)_adapterInfoMap.get(id);
	    adapter.updateProxy(proxy);
	}
	
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
	p = _adapters.values().iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.updateProxy(null);
	}

	fireNodeChangedEvent(this);
    }

    void updateServer(ServerDynamicInfo info)
    {
	//
	// NodeViewRoot updates the map
	//
	ServerInstance child = (ServerInstance)findChild(info.name);
	if(child != null)
	{
	    child.updateDynamicInfo(info);
	}
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
		return _unknownServerDynamicInfo;
	    }
	    else
	    {
		return (ServerDynamicInfo)obj;
	    }
	}
    }

    void updateAdapter(AdapterDynamicInfo info)
    {
	//
	// NodeViewRoot updates the map
	//
	Adapter adapter = (Adapter)_adapters.get(info.id);
	if(adapter != null)
	{
	    adapter.updateProxy(info.proxy);
	}
    }
    
    Node(String applicationName, Model model, NodeDescriptor descriptor,
	 java.util.Map serverInfoMap, java.util.Map adapterInfoMap)
    {
	super(descriptor.name, model);
	_serverInfoMap = serverInfoMap;
	_adapterInfoMap = adapterInfoMap;
	_applicationMap.put(applicationName, descriptor);
    }

    void addApplication(String applicationName, NodeDescriptor descriptor)
    {
	_applicationMap.put(applicationName, descriptor);
    }
 
    //
    // Returns true when this node should be destroyed
    //
    boolean removeApplication(String applicationName)
    {
	_applicationMap.remove(applicationName);
	return (_applicationMap.size() == 0);
    }


    //
    // The node maintains a map adapter id => Adapter
    //
    Ice.ObjectPrx registerAdapter(String id, Adapter adapter)
    {
	_adapters.put(id, adapter);
	Ice.ObjectPrx result = null;
	if(_adapterInfoMap != null)
	{
	    result = (Ice.ObjectPrx)_adapterInfoMap.get(id);
	}
	return result;
    }

    void unregisterAdapter(String id)
    {
	_adapters.remove(id);
    }

    private java.util.Map _applicationMap = new java.util.HashMap();
    
    private java.util.Map _serverInfoMap;
    private java.util.Map _adapterInfoMap;

    //
    // Adapter id => Adapter
    //
    private java.util.Map _adapters = new java.util.HashMap();


    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUpOpen;
    static private Icon _nodeUpClosed;
    static private Icon _nodeDownOpen;
    static private Icon _nodeDownClosed;

    static private ServerDynamicInfo _unknownServerDynamicInfo = 
       new ServerDynamicInfo();
}
