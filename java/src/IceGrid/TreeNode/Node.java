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
import IceGrid.Model;
import IceGrid.NodeDynamicInfo;
import IceGrid.NodeUpdateDescriptor;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ServerDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Node extends EditableParent
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
	if(_up)
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
    
    
    void up()
    {
	_up = true;
	fireNodeChangedEvent(this);
    }

    boolean down()
    {
	_up = false;

	if(_descriptor == null)
	{
	    return true;
	}
	else
	{
	    fireNodeChangedEvent(this);
	    return false;
	}
    }

    public void unregister()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    server.unregister();
	}
    }
    

    NodeUpdateDescriptor getUpdate()
    {
	if(!isNew() && !isModified())
	{
	    return null;
	}

	NodeUpdateDescriptor update = new NodeUpdateDescriptor();
	update.name = _id;
	
	if(isNew())
	{
	    update.variables = _descriptor.variables;
	    update.removeVariables = new String[0];
	}
	else
	{
	    //
	    // Diff variables (TODO: avoid duplication with same code in Application)
	    //
	    update.variables = (java.util.TreeMap)_descriptor.variables.clone();
	    java.util.List removeVariables = new java.util.LinkedList();

	    java.util.Iterator p = _origVariables.entrySet().iterator();
	    while(p.hasNext())
	    {
		java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
		Object key = entry.getKey();
		Object newValue =  update.variables.get(key);
		if(newValue == null)
		{
		    removeVariables.add(key);
		}
		else
		{
		    Object value = entry.getValue();
		    if(newValue.equals(value))
		    {
			update.variables.remove(key);
		    }
		}
	    }
	    update.removeVariables = (String[])removeVariables.toArray(new String[0]);
	}
	       
	if(isNew())
	{
	    update.removeServers = new String[0];
	}
	else
	{
	    update.removeServers = removedElements();
	}

	update.serverInstances = new java.util.LinkedList();
	update.servers = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    if(isNew() || server.isModified() || server.isNew())
	    {
		ServerInstanceDescriptor instanceDescriptor = server.getInstanceDescriptor();
		if(instanceDescriptor != null)
		{
		    update.serverInstances.add(instanceDescriptor);
		}
		else
		{
		    update.servers.add(server.getDescriptor());
		}
	    }
	}
	return update;
    }


    NodeDescriptor update(NodeUpdateDescriptor update, Application application)
	throws DuplicateIdException
    {
	if(_descriptor == null)
	{
	    NodeDescriptor descriptor = new NodeDescriptor(update.variables,
							   update.serverInstances,
							   update.servers,
							   update.loadFactor.value);

	    init(descriptor, application, true);
	    return _descriptor;
	}

	//
	// Otherwise it's a real update
	//

	//
	// Variables
	//
	for(int i = 0; i < update.removeVariables.length; ++i)
	{
	    _descriptor.variables.remove(update.removeVariables[i]);
	}
	_descriptor.variables.putAll(update.variables);

	//
	// One big set of removes
	//
	removeChildren(update.removeServers);

	//
	// Update _descriptor
	//
	for(int i = 0; i < update.removeServers.length; ++i)
	{
	    _descriptor.serverInstances.remove(update.removeServers[i]);
	    _descriptor.servers.remove(update.removeServers[i]);
	} 

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = update.serverInstances.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor instanceDescriptor = 
		(ServerInstanceDescriptor)p.next();
	    
	    //
	    // Find template
	    //
	    TemplateDescriptor templateDescriptor = 
		application.findServerTemplateDescriptor(instanceDescriptor.template);

	    assert templateDescriptor != null;
	    
	    ServerDescriptor serverDescriptor = 
		(ServerDescriptor)templateDescriptor.descriptor;
	    
	    assert serverDescriptor != null;
	    
	    //
	    // Build resolver
	    //
	    Utils.Resolver instanceResolver = 
		new Utils.Resolver(_resolver, instanceDescriptor.parameterValues);
	    
	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    instanceResolver.put("server", serverId);
	    
	    //
	    // Lookup server
	    //
	    Server server = (Server)findChild(serverId);
	    if(server != null)
	    {
		server.rebuild(instanceResolver, instanceDescriptor, serverDescriptor,
			       application);
		updatedChildren.add(server);
	    }
	    else
	    {
		server = new Server(false, serverId, instanceResolver, instanceDescriptor, 
				    serverDescriptor, application);
		newChildren.add(server);
		_descriptor.serverInstances.add(instanceDescriptor);
	    }
	    
	}
	
	//
	// Plain servers
	//
	p = update.servers.iterator();
	while(p.hasNext())
	{
	    ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();

	    //
	    // Build resolver
	    //
	    Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    instanceResolver.put("server", serverId);
	    
	    //
	    // Lookup server
	    //
	    Server server = (Server)findChild(serverId);
	    
	    if(server != null)
	    {
		server.rebuild(instanceResolver, null, serverDescriptor,
			       application);
		updatedChildren.add(server);
	    }
	    else
	    {
		server = new Server(false, serverId, instanceResolver, null, 
				    serverDescriptor, application);
		newChildren.add(server);
		_descriptor.servers.add(serverDescriptor);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));

	p = newChildren.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    server.setParent(this);
	}

	return null;
    }
   
    public void commit()
    {
	super.commit();
	if(_descriptor != null)
	{
	    _origVariables = (java.util.Map)_descriptor.variables.clone();
	}
    }

    Node(String nodeName, Model model)
    {
	super(false, nodeName, model);  
	_up = true;
    }

    Node(boolean brandNew, String nodeName, NodeDescriptor descriptor, Application application)
	throws DuplicateIdException
    {
	super(brandNew, nodeName, application.getModel());
	init(descriptor, application, false);
    } 
    
    Node(Node o)
    {
	super(o);
	_descriptor = o._descriptor;
	_resolver = o._resolver;
	_origVariables = o._origVariables;
	_up = o._up;

	//
	// Deep-copy children
	//
	java.util.Iterator p = o._children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    try
	    {
		addChild(new Server(server));
	    }
	    catch(DuplicateIdException e)
	    {
		assert false; // impossible
	    }
	}
    }

    void init(NodeDescriptor descriptor, Application application, boolean fireEvent)
	throws DuplicateIdException
    {
	assert _descriptor == null;
	_descriptor = descriptor;
	_origVariables = (java.util.Map)_descriptor.variables.clone();
	

	_resolver = new Utils.Resolver(new java.util.Map[]
	    {_descriptor.variables, application.getVariables()});
				       
	_resolver.put("application", application.getId());
	_resolver.put("node", getId());
	
	//
	// Template instances
	//
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor instanceDescriptor = 
		(ServerInstanceDescriptor)p.next();
	    
	    //
	    // Find template
	    //
	    TemplateDescriptor templateDescriptor = 
		application.findServerTemplateDescriptor(instanceDescriptor.template);

	    assert templateDescriptor != null;
	    
	    ServerDescriptor serverDescriptor = 
		(ServerDescriptor)templateDescriptor.descriptor;
	    
	    assert serverDescriptor != null;
	    
	    //
	    // Build resolver
	    //
	    Utils.Resolver instanceResolver = 
		new Utils.Resolver(_resolver, instanceDescriptor.parameterValues);
	    
	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    instanceResolver.put("server", serverId);
	    
	    //
	    // Create server
	    //
	    Server server = new Server(false, serverId, instanceResolver, instanceDescriptor, 
				       serverDescriptor, application);
	    addChild(server);
	    server.setParent(this);
	}

	//
	// Plain servers
	//
	p = _descriptor.servers.iterator();
	while(p.hasNext())
	{
	    ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();

	    //
	    // Build resolver
	    //
	    Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    instanceResolver.put("server", serverId);
	    
	    //
	    // Create server
	    //
	    Server server = new Server(false, serverId, instanceResolver, null, serverDescriptor, 
				       application);
	    addChild(server);
	    server.setParent(this);
	}

	if(fireEvent)
	{
	    fireStructureChangedEvent(this);
	}
    }
    
    
    void update() throws DuplicateIdException
    {
	if(_descriptor == null)
	{
	    //
	    // Nothing to do
	    //
	    return;
	}

	Application application = getApplication();

	_resolver = new Utils.Resolver(new java.util.Map[]
	    {_descriptor.variables, application.getVariables()});
				       
	_resolver.put("application", application.getId());
	_resolver.put("node", getId());
	
	//
	// Existing servers not in this list will be removed
	//
	java.util.Set serverIdSet = new java.util.HashSet();

	//
	// Template instances
	//
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor instanceDescriptor = 
		(ServerInstanceDescriptor)p.next();
	    
	    //
	    // Find template
	    //
	    TemplateDescriptor templateDescriptor = 
		application.findServerTemplateDescriptor(instanceDescriptor.template);

	    if(templateDescriptor != null)
	    {    
		ServerDescriptor serverDescriptor = 
		    (ServerDescriptor)templateDescriptor.descriptor;
		
		assert serverDescriptor != null;
		
		//
		// Build resolver
		//
		Utils.Resolver instanceResolver = 
		    new Utils.Resolver(_resolver, instanceDescriptor.parameterValues);
	    
		String serverId = instanceResolver.substitute(serverDescriptor.id);
		instanceResolver.put("server", serverId);
		serverIdSet.add(serverId);
		
		//
		// Lookup server
		//
		Server server = (Server)findChild(serverId);
		if(server != null)
		{
		    server.rebuild(instanceResolver, instanceDescriptor, 
				   serverDescriptor, application);
		}
		else
		{
		    //
		    // Create server
		    //
		    server = new Server(true, serverId, instanceResolver, instanceDescriptor, 
					serverDescriptor, application);
		    addChild(server, true);
		    server.setParent(this);   
		}
	    }
	    else
	    {
		//
		// The child will be deleted through purgeChildren, and the descriptor
		// will be deleted (later) through cascadeDeleteServerInstance
		//
	    }
	}

	//
	// Plain servers
	//
	p = _descriptor.servers.iterator();
	while(p.hasNext())
	{
	    ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();

	    //
	    // Build resolver
	    //
	    Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    instanceResolver.put("server", serverId);
	    serverIdSet.add(serverId);

	    //
	    // Lookup server
	    //
	    Server server = (Server)findChild(serverId);
	    if(server != null)
	    {
		server.rebuild(instanceResolver, null, serverDescriptor, application);
	    }
	    else
	    {
		//
		// Create server
		//
		server = new Server(true, serverId, instanceResolver, null, serverDescriptor, 
				    application);
		addChild(server);
		server.setParent(this);
	    }
	}

	purgeChildren(serverIdSet);
    }

    void cascadeDeleteServerInstance(String templateId)
    {
	if(_descriptor == null)
	{
	    //
	    // Nothing to do
	    //
	    return;
	}
	
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor instanceDescriptor = 
		(ServerInstanceDescriptor)p.next();

	    if(instanceDescriptor.template.equals(templateId))
	    {
		p.remove();
	    }
	}
    }

    void cascadeDeleteServiceInstance(String templateId)
    {
	if(_descriptor == null)
	{
	    //
	    // Nothing to do
	    //
	    return;
	}
	
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    server.cascadeDeleteServiceInstance(templateId);
	}
    }

    java.util.List findServerInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    Object descriptor = server.getDescriptor();
	    if(descriptor instanceof ServerInstanceDescriptor)
	    {
		ServerInstanceDescriptor sid = (ServerInstanceDescriptor)descriptor;
		if(sid.template.equals(template))
		{
		    result.add(server);
		}
	    }
	}
	return result;
    }

    private NodeDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private java.util.Map _origVariables;

    private boolean _up = false;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUpOpen;
    static private Icon _nodeUpClosed;
    static private Icon _nodeDownOpen;
    static private Icon _nodeDownClosed;
}
