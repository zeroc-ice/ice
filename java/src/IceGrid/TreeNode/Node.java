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
import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.NodeDescriptor;
import IceGrid.NodeInfo;
import IceGrid.Model;
import IceGrid.NodeInfo;
import IceGrid.NodeDynamicInfo;
import IceGrid.NodeUpdateDescriptor;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ServerDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.SimpleInternalFrame;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;

class Node extends EditableParent
{  
    static public NodeDescriptor
    copyDescriptor(NodeDescriptor nd)
    {
	NodeDescriptor copy = (NodeDescriptor)nd.clone();
	
	copy.serverInstances = new java.util.LinkedList();
	java.util.Iterator p = nd.serverInstances.iterator();
	while(p.hasNext())
	{
	    copy.serverInstances.add(Server.copyDescriptor(
					 (ServerInstanceDescriptor)p.next()));
	}
       
	copy.servers = new java.util.LinkedList();
	p = nd.servers.iterator();
	while(p.hasNext())
	{
	    copy.servers.add(Server.copyDescriptor(
				 (ServerDescriptor)p.next()));
	}
	
	return copy;
    }

    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	actions[COPY] = true;
	actions[DELETE] = true;

	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof NodeDescriptor ||
		descriptor instanceof ServerInstanceDescriptor ||
		descriptor instanceof ServerDescriptor;
	}

	if(!_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;
	    actions[NEW_SERVER] = true;
	    actions[NEW_SERVER_ICEBOX] = true;
	    actions[NEW_SERVER_FROM_TEMPLATE] = true;
	    actions[SHUTDOWN_NODE] = _up;
	}
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);

	    JMenuItem newServerItem = new JMenuItem(_model.getActions()[NEW_SERVER]);
	    newServerItem.setText("New server");
	    _popup.add(newServerItem);

	    JMenuItem newIceBoxItem = new JMenuItem(_model.getActions()[NEW_SERVER_ICEBOX]);
	    newIceBoxItem.setText("New IceBox server");
	    _popup.add(newIceBoxItem);

	    JMenuItem newServerFromTemplateItem = 
		new JMenuItem(_model.getActions()[NEW_SERVER_FROM_TEMPLATE]);
	    newServerFromTemplateItem.setText("New server from template");
	    _popup.add(newServerFromTemplateItem); 
	    
	    _popup.addSeparator();
	    _popup.add(_model.getActions()[SHUTDOWN_NODE]);
	}
	return _popup;
    }
    public void copy()
    {
	_model.setClipboard(copyDescriptor(_descriptor));
	_model.getActions()[PASTE].setEnabled(true);
    }
    public void paste()
    {
	Object descriptor =  _model.getClipboard();
	if(descriptor instanceof NodeDescriptor)
	{
	    _parent.paste();
	}
	else if(descriptor instanceof ServerInstanceDescriptor)
	{
	    newServer(Server.copyDescriptor((ServerInstanceDescriptor)descriptor));
	}
	else
	{
	    newServer(Server.copyDescriptor(((ServerDescriptor)descriptor)));
	}
    }
    public void newServer()
    {
	newServer(Server.newServerDescriptor());
    }
    public void newServerIceBox()
    {
	newServer(Server.newIceBoxDescriptor());
    }
    public void newServerFromTemplate()
    {
	ServerInstanceDescriptor descriptor = 
	    new ServerInstanceDescriptor("",
					 new java.util.HashMap());

	newServer(descriptor);
    }
    public void shutdownNode()
    {
	// TODO: implement!
    }
    
    public boolean destroy()
    {
	if(_parent == null)
	{
	    return false;
	}
	Nodes nodes = (Nodes)_parent;
	
	if(_ephemeral)
	{
	    nodes.removeChild(this, true);
	    return true;
	}
	else if(!_inRegistry)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		"There is no definition for this node in application '" 
		+ getApplication().getId()
		+ "'; this node only appears here because it's up and running.",
		"Nothing to destroy",
		JOptionPane.INFORMATION_MESSAGE);
	    return false;
	}
	else if(_model.canUpdate())
	{
	    Application application = getApplication();

	    nodes.removeDescriptor(_id);
	    nodes.removeElement(this, true);
	    if(_up)
	    {
		try
		{
		    Node cleanNode = new Node(false, _id, null, application, _staticInfo);
		    nodes.addChild(cleanNode, true);
		}
		catch(UpdateFailedException e)
		{
		    assert false;
		}
	    }
	    return true;
	}
	return false;
    }


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

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new NodeEditor(_model.getMainFrame());
	}
	_editor.show(this);
	return _editor;
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }
    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public Object saveDescriptor()
    {
	return _descriptor.clone();
    }

    public void restoreDescriptor(Object savedCopy)
    {
	NodeDescriptor copy = (NodeDescriptor)savedCopy;
	_descriptor.description = copy.description;
	_descriptor.loadFactor = copy.loadFactor;
	_descriptor.variables = copy.variables;
    }

    static private class ServerBackup
    {
	java.util.TreeSet removedElements;
	java.util.Map parameterValues;
    }

    Object rebuildChild(CommonBase child, java.util.List editables) 
	throws UpdateFailedException
    {
	ServerBackup backup = new ServerBackup();

	Server newServer = null;
	Server server = (Server)child;
	ServerInstanceDescriptor instanceDescriptor = server.getInstanceDescriptor();
	
	if(instanceDescriptor != null)
	{
	    TemplateDescriptor templateDescriptor 
		= getApplication().findServerTemplateDescriptor(instanceDescriptor.template);

	    java.util.Set parameters = new java.util.HashSet(templateDescriptor.parameters);
	    if(!parameters.equals(instanceDescriptor.parameterValues.keySet()))
	    {
		backup.parameterValues = instanceDescriptor.parameterValues;
		instanceDescriptor.parameterValues = Editor.makeParameterValues(
		    instanceDescriptor.parameterValues, templateDescriptor.parameters);
	    }
	    try
	    {
		newServer = createServer(false, instanceDescriptor, getApplication());
	    }
	    catch(UpdateFailedException e)
	    {
		e.addParent(this);
		throw e;
	    }
	}
	else
	{
	    try
	    {
		newServer = createServer(false, server.getServerDescriptor(), getApplication());
	    }
	    catch(UpdateFailedException e)
	    {
		e.addParent(this);
		throw e;
	    }
	}

	if(server.getId().equals(newServer.getId()))
	{
	    //
	    // A simple update. We can't simply rebuild server because 
	    // we need to keep a backup
	    //
	    if(server.isModified())
	    {
		newServer.markModified();
	    }

	    removeChild(server, true);	    
	    try
	    {
		addChild(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		assert false; // impossible, we just removed a child with
		              // this id
	    }

	    if(backup.parameterValues != null)
	    {
		editables.add(newServer);
	    }
	}
	else
	{
	    newServer.markNew();
	    backup.removedElements = (java.util.TreeSet)_removedElements.clone();
	    removeElement(server, true);
	    try
	    {
		addChild(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		e.addParent(this);
		restoreChild(server, backup);
		throw e;
	    }
	}
	
	return backup;
    }

    void restoreChild(CommonBase child, Object backupObject)
    {
	ServerBackup backup = (ServerBackup)backupObject;
	if(backup.removedElements != null)
	{
	    _removedElements = backup.removedElements;
	}

	Server goodServer = (Server)child;
	ServerInstanceDescriptor instanceDescriptor = goodServer.getInstanceDescriptor();
	if(instanceDescriptor != null &&  backup.parameterValues != null)
	{
	    instanceDescriptor.parameterValues = backup.parameterValues;
	}

	CommonBase badServer = findChildWithDescriptor(goodServer.getDescriptor());
	if(badServer != null)
	{
	    removeChild(badServer, true);
	}

	try
	{
	    addChild(child, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
    }

    static class Backup
    {
	Utils.Resolver resolver;
	java.util.List backupList;
	java.util.List servers;
    }

    //
    // Try to rebuild this node;
    // returns a backup object if rollback is later necessary
    //
    Backup rebuild(java.util.List editables)
	throws UpdateFailedException
    {
	Application application = getApplication();
	Backup backup = new Backup();
	backup.resolver = _resolver;

	_resolver = new Utils.Resolver(new java.util.Map[]
	    {_descriptor.variables, application.getVariables()});
				       
	_resolver.put("application", application.getId());
	_resolver.put("node", getId());

	backup.backupList = new java.util.Vector();
	backup.servers = (java.util.LinkedList)_children.clone();

	java.util.Iterator p = backup.servers.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    try
	    {
		backup.backupList.add(rebuildChild(server, editables));
	    }
	    catch(UpdateFailedException e)
	    {
		restore(backup);
		throw e;
	    }
	}
	return backup;
    }
    
    void restore(Backup backup)
    {	
	for(int i = backup.backupList.size() - 1; i >= 0; --i)
	{
	    restoreChild((Server)backup.servers.get(i), backup.backupList.get(i));
	}
	_resolver = backup.resolver;
    }
    
    private Server createServer(boolean brandNew, ServerInstanceDescriptor instanceDescriptor,
				Application application) throws UpdateFailedException
    {
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
	    new Utils.Resolver(_resolver, 
			       instanceDescriptor.parameterValues,
			       templateDescriptor.parameterDefaults);
	
	String serverId = instanceResolver.substitute(serverDescriptor.id);
	instanceResolver.put("server", serverId);
	
	//
	// Create server
	//
	return new Server(brandNew, serverId, instanceResolver, instanceDescriptor, 
			  serverDescriptor, application);
    }

    private Server createServer(boolean brandNew, ServerDescriptor serverDescriptor,
				Application application) throws UpdateFailedException
    {
	//
	// Build resolver
	//
	Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
	String serverId = instanceResolver.substitute(serverDescriptor.id);
	instanceResolver.put("server", serverId);
	
	//
	// Create server
	//
	return new Server(brandNew, serverId, instanceResolver, null, serverDescriptor, 
			  application);

    }

    void up(NodeInfo staticInfo)
    {
	_up = true;
	_staticInfo = staticInfo;
	fireNodeChangedEvent(this);
    }

    boolean down()
    {
	_up = false;

	if(_inRegistry)
	{
	    fireNodeChangedEvent(this);
	    return false;
	}
	else
	{
	    return true;
	}
    }

    NodeUpdateDescriptor getUpdate()
    {
	NodeUpdateDescriptor update = new NodeUpdateDescriptor();
	update.name = _id;

	//
	// First: servers
	//
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
	
	//
	// Anything in this update?
	//
	if(!isNew() && !isModified() && update.removeServers.length == 0
	   && update.servers.size() == 0
	   && update.serverInstances.size() == 0)
	{
	    return null;
	}

	if(isNew())
	{
	    update.variables = _descriptor.variables;
	    update.removeVariables = new String[0];
	    update.loadFactor = new IceGrid.BoxedString(_descriptor.loadFactor);
	}
	else
	{
	    if(!_descriptor.loadFactor.equals(_origLoadFactor))
	    {
		update.loadFactor = new IceGrid.BoxedString(_descriptor.loadFactor);
	    }

	    //
	    // Diff variables (TODO: avoid duplication with same code in Application)
	    //
	    update.variables = (java.util.TreeMap)_descriptor.variables.clone();
	    java.util.List removeVariables = new java.util.LinkedList();

	    p = _origVariables.entrySet().iterator();
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
	       

	return update;
    }


    void update(NodeUpdateDescriptor update, Application application)
	throws UpdateFailedException
    {
	//
	// Load factor
	//
	if(update.loadFactor != null)
	{
	    _descriptor.loadFactor = update.loadFactor.value;
	}

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
		new Utils.Resolver(_resolver, 
				   instanceDescriptor.parameterValues,
				   templateDescriptor.parameterDefaults);
	    
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

	if(!_inRegistry)
	{
	    ((Nodes)_parent).addDescriptor(update.name, _descriptor);
	    _inRegistry = true;
	}
    }
   
    public void commit()
    {
	super.commit();
	if(_descriptor != null)
	{
	    _origVariables = (java.util.Map)_descriptor.variables.clone();
	    _origLoadFactor = _descriptor.loadFactor;
	}
    }

    Node(boolean brandNew, String nodeName, NodeDescriptor descriptor, 
	 Application application, NodeInfo staticInfo)
	throws UpdateFailedException
    {
	super(brandNew, nodeName, application.getModel());
	_ephemeral = false;
	_inRegistry = (descriptor != null);
	_staticInfo = staticInfo;
	_up = staticInfo != null;

	if(!_inRegistry)
	{
	    assert !brandNew;
	    descriptor = new NodeDescriptor(new java.util.TreeMap(), 
					    new java.util.LinkedList(),
					    new java.util.LinkedList(),
					    "",
					    "");
	}
	
	_descriptor = descriptor;
	_origVariables = (java.util.Map)_descriptor.variables.clone();
	_origLoadFactor = _descriptor.loadFactor;


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
	   
	    addChild(createServer(false, instanceDescriptor, application));
	}

	//
	// Plain servers
	//
	p = _descriptor.servers.iterator();
	while(p.hasNext())
	{
	    ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();
	    addChild(createServer(false, serverDescriptor, application));
	}
    } 
    
    Node(String nodeName, NodeDescriptor descriptor, Model model)
    {
	super(false, nodeName, model);
	_ephemeral = true;
	_inRegistry = false;
	_descriptor = descriptor;
    }

    void removeInstanceDescriptor(ServerInstanceDescriptor d)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    if(d == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }

    java.util.List findServerInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    ServerInstanceDescriptor instanceDescriptor
		= server.getInstanceDescriptor();

	    if(instanceDescriptor != null && 
	       instanceDescriptor.template.equals(template))
	    {
		result.add(server);
	    }
	}
	return result;
    }


    void removeServerInstances(String template)
    {
	java.util.List toRemove = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    ServerInstanceDescriptor instanceDescriptor
		= server.getInstanceDescriptor();

	    if(instanceDescriptor != null && 
	       instanceDescriptor.template.equals(template))
	    {
		//
		// Remove instance
		//
		removeInstanceDescriptor(instanceDescriptor);
		_removedElements.add(server.getId());
		toRemove.add(server.getId());
	    }
	}

	if(toRemove.size() > 0)
	{
	    removeChildren((String[])toRemove.toArray(new String[0]));
	}
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    result.addAll(server.findServiceInstances(template));
	}
	return result;
    }


    void removeServiceInstances(String template)
    {	
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    server.removeServiceInstances(template);
	}
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    NodeInfo getStaticInfo()
    {
	return _staticInfo;
    }

    void tryAdd(ServerInstanceDescriptor instanceDescriptor,
		ServerDescriptor serverDescriptor,
		boolean addDescriptor) throws UpdateFailedException
    {
	try
	{
	    if(instanceDescriptor != null)
	    {
		addChild(createServer(true, instanceDescriptor, getApplication()),
			 true);
	    }
	    else
	    {
		addChild(createServer(true, serverDescriptor, getApplication()),
			 true);
	    }
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
	}

	if(addDescriptor)
	{
	    if(instanceDescriptor != null)
	    {
		_descriptor.serverInstances.add(instanceDescriptor);
	    }
	    else
	    {
		_descriptor.servers.add(serverDescriptor);
	    }
	}

	if(!_inRegistry)
	{
	    moveToRegistry();
	}
    }

    void removeDescriptor(ServerDescriptor sd)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptor.servers.iterator();
	while(p.hasNext())
	{
	    if(sd == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }
    void removeDescriptor(ServerInstanceDescriptor sd)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    if(sd == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }

    boolean inRegistry()
    {
	return _inRegistry;
    }

    void moveToRegistry()
    {
	assert !_inRegistry;
	assert !_ephemeral;

	((Nodes)_parent).addDescriptor(_id, _descriptor);
	_inRegistry = true;
	markNew();
    }

    private void newServer(ServerDescriptor descriptor)
    {
	descriptor.id = makeNewChildId(descriptor.id);
	
	Server server = new Server(descriptor.id, null, descriptor, _model);
	try
	{
	    addChild(server, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(server.getPath());
    }

    private void newServer(ServerInstanceDescriptor descriptor)
    {
	String id = makeNewChildId("NewServer");
	
	//
	// Make sure descriptor.template points to a real template
	//
	ServerTemplate t = getApplication().findServerTemplate(descriptor.template);
	
	if(t == null)
	{
	    t = (ServerTemplate)getApplication().getServerTemplates().getChildAt(0);
	    
	    if(t == null)
	    {
		JOptionPane.showMessageDialog(
		    _model.getMainFrame(),
		    "You need to create a server template before you can create a server from a template.",
		    "No Server Template",
		    JOptionPane.INFORMATION_MESSAGE);
		return;
	    }
	    else
	    {
		descriptor.template = t.getId();
		descriptor.parameterValues = new java.util.HashMap();
	    }
	}
	
	ServerDescriptor sd = (ServerDescriptor)
	    ((TemplateDescriptor)t.getDescriptor()).descriptor;

	Server server = new Server(id, descriptor, sd, _model);
	try
	{
	    addChild(server, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(server.getPath());
    }

    private NodeDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private java.util.Map _origVariables;
    private String _origLoadFactor;

    private boolean _up = false;
    private final boolean _ephemeral;

    private boolean _inRegistry;

    private NodeInfo _staticInfo;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUpOpen;
    static private Icon _nodeUpClosed;
    static private Icon _nodeDownOpen;
    static private Icon _nodeDownClosed;

    static private NodeEditor _editor;
    static private JPopupMenu _popup;
}
