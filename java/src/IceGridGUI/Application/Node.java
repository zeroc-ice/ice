// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTextField;
import javax.swing.JTree;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class Node extends ListTreeNode
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

	Object descriptor =  getCoordinator().getClipboard();
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
	}
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = getCoordinator().getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_SERVER));
	    _popup.add(actions.get(NEW_SERVER_ICEBOX));
	    _popup.add(actions.get(NEW_SERVER_FROM_TEMPLATE));
	}
	actions.setTarget(this);
	return _popup;
    }
    public void copy()
    {
	getCoordinator().setClipboard(copyDescriptor(_descriptor));
	getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }
    public void paste()
    {
	Object descriptor =  getCoordinator().getClipboard();
	if(descriptor instanceof NodeDescriptor)
	{
	    ((TreeNode)_parent).paste();
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
    
    public void destroy()
    {
	Nodes nodes = (Nodes)_parent;
	if(_ephemeral)
	{
	    nodes.removeChild(this);
	}
	else
	{
	    nodes.removeChild(this);
	    nodes.removeDescriptor(_id);
	    nodes.getEditable().removeElement(_id);
	    getRoot().updated();
	}
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
	    Icon nodeIcon = Utils.getIcon("/icons/16x16/node.png");
	    _cellRenderer.setOpenIcon(nodeIcon);
	    _cellRenderer.setClosedIcon(nodeIcon);
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = (NodeEditor)getRoot().getEditor(NodeEditor.class, this);
	}
	_editor.show(this);
	return _editor;
    }

    protected Editor createEditor()
    {
	return new NodeEditor(getCoordinator().getMainFrame());
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }
    

    Object getDescriptor()
    {
	return _descriptor;
    }

   
    NodeDescriptor saveDescriptor()
    {
	return (NodeDescriptor)_descriptor.clone();
    }

    void restoreDescriptor(NodeDescriptor copy)
    {
	_descriptor.description = copy.description;
	_descriptor.loadFactor = copy.loadFactor;
	_descriptor.variables = copy.variables;
    }

    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    java.util.List attributes = new java.util.LinkedList();
	    attributes.add(createAttribute("name", _id));
	    if(_descriptor.loadFactor.length() > 0)
	    {
		attributes.add(createAttribute("load-factor",
					       _descriptor.loadFactor));
	    }
	    
	    writer.writeStartTag("node", attributes);
	    writeVariables(writer, _descriptor.variables);

	    java.util.Iterator p = _children.iterator();
	    while(p.hasNext())
	    {
		Server server = (Server)p.next();
		server.write(writer);
	    }
	    writer.writeEndTag("node");
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
	Root root = getRoot();
	Backup backup = new Backup();
	backup.resolver = _resolver;

	_resolver = new Utils.Resolver(new java.util.Map[]
	    {_descriptor.variables, root.getVariables()});
				       
	_resolver.put("application", root.getId());
	_resolver.put("node", _id);

	backup.backupList = new java.util.Vector();
	backup.servers = (java.util.LinkedList)_children.clone();

	java.util.Iterator p = backup.servers.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    try
	    {
		backup.backupList.add(server.rebuild(editables));
	    }
	    catch(UpdateFailedException e)
	    {
		restore(backup);
		throw e;
	    }
	}
	return backup;
    }

    void commit()
    {
	_editable.commit();
	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origLoadFactor = _descriptor.loadFactor;

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    server.commit();
	}
    }

    
    void restore(Backup backup)
    {	
	for(int i = backup.backupList.size() - 1; i >= 0; --i)
	{
	    ((Server)backup.servers.get(i)).restore(backup.backupList.get(i));
	}
	_resolver = backup.resolver;
    }
    
    Server createServer(boolean brandNew, ServerInstanceDescriptor instanceDescriptor) 
	throws UpdateFailedException
    {
	Root root = getRoot();

	//
	// Find template
	//
	TemplateDescriptor templateDescriptor = 
	    root.findServerTemplateDescriptor(instanceDescriptor.template);

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
	return new Server(brandNew, this, serverId, instanceResolver, instanceDescriptor, 
			  serverDescriptor);
    }

    Server createServer(boolean brandNew, ServerDescriptor serverDescriptor) 
	throws UpdateFailedException
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
	return new Server(brandNew, this, serverId, instanceResolver, null, serverDescriptor);

    }

    NodeUpdateDescriptor getUpdate()
    {
	NodeUpdateDescriptor update = new NodeUpdateDescriptor();
	update.name = _id;

	//
	// First: servers
	//
	if(_editable.isNew())
	{
	    update.removeServers = new String[0];
	}
	else
	{
	    update.removeServers = _editable.removedElements();
	}

	update.serverInstances = new java.util.LinkedList();
	update.servers = new java.util.LinkedList();

	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();
	    if(_editable.isNew() || server.getEditable().isModified() 
	       || server.getEditable().isNew())
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
	if(!_editable.isNew() && !_editable.isModified() && update.removeServers.length == 0
	   && update.servers.size() == 0
	   && update.serverInstances.size() == 0)
	{
	    return null;
	}

	if(_editable.isNew())
	{
	    update.variables = _descriptor.variables;
	    update.removeVariables = new String[0];
	    update.loadFactor = new IceGrid.BoxedString(_descriptor.loadFactor);
	}
	else
	{
	    if(!_descriptor.description.equals(_origDescription))
	    {
		update.description = new IceGrid.BoxedString(_descriptor.description);
	    }
	    
	    if(!_descriptor.loadFactor.equals(_origLoadFactor))
	    {
		update.loadFactor = new IceGrid.BoxedString(_descriptor.loadFactor);
	    }

	    //
	    // Diff variables (TODO: avoid duplication with same code in Root)
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


    void update(NodeUpdateDescriptor update, 
		java.util.Set serverTemplates, java.util.Set serviceTemplates)
	throws UpdateFailedException
    {
	Root root = getRoot();

	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();

	if(update != null)
	{
	    //
	    // Description
	    //
	    if(update.description != null)
	    {
		_descriptor.description = update.description.value;
		_origDescription = _descriptor.description;
	    }
	    
	    //
	    // Load factor
	    //
	    if(update.loadFactor != null)
	    {
		_descriptor.loadFactor = update.loadFactor.value;
		_origLoadFactor = _descriptor.loadFactor;
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
	    
	    java.util.Iterator p = update.serverInstances.iterator();
	    while(p.hasNext())
	    {
		ServerInstanceDescriptor instanceDescriptor = 
		    (ServerInstanceDescriptor)p.next();
		
		//
		// Find template
		//
		TemplateDescriptor templateDescriptor = 
		    root.findServerTemplateDescriptor(instanceDescriptor.template);
		
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
		    server.rebuild(instanceResolver, instanceDescriptor, serverDescriptor);
		    updatedChildren.add(server);
		}
		else
		{
		    server = new Server(false, this, serverId, instanceResolver, instanceDescriptor, 
					serverDescriptor);
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
		    server.rebuild(instanceResolver, null, serverDescriptor);
		    updatedChildren.add(server);
		}
		else
		{
		    server = new Server(false, this, serverId, instanceResolver, null, 
					serverDescriptor);
		    newChildren.add(server);
		    _descriptor.servers.add(serverDescriptor);
		}
	    }
	}
	
	// 
	// Find servers affected by template updates
	//
	java.util.Set serverSet = new java.util.HashSet();

	java.util.Iterator p = serverTemplates.iterator();
	while(p.hasNext())
	{
	    String template = (String)p.next();
	    java.util.List serverInstances = findServerInstances(template);
	    java.util.Iterator q = serverInstances.iterator();
	    while(q.hasNext())
	    {
		Server server = (Server)q.next();
		if(!updatedChildren.contains(server) && !newChildren.contains(server))
		{
		    serverSet.add(server);
		}
	    }
	}

	p = serviceTemplates.iterator();
	while(p.hasNext())
	{
	    java.util.List serviceInstances = 
		findServiceInstances((String)p.next());
	    java.util.Iterator q = serviceInstances.iterator();
	    while(q.hasNext())
	    {
		Service service = (Service)q.next();
		Server server = (Server)service.getParent().getParent();
		if(!updatedChildren.contains(server) && !newChildren.contains(server))
		{
		    serverSet.add(server);
		}
	    }
	}

	//
	// Rebuild these servers
	//
	p = serverSet.iterator();
	while(p.hasNext())
	{
	    Server server = (Server)p.next();

	    ServerInstanceDescriptor instanceDescriptor = 
		server.getInstanceDescriptor();

	    ServerDescriptor serverDescriptor;
	    Utils.Resolver instanceResolver;

	    if(instanceDescriptor == null)
	    {
		serverDescriptor = server.getServerDescriptor();
		assert serverDescriptor != null;
		instanceResolver = new Utils.Resolver(_resolver);
	    }
	    else
	    {
		TemplateDescriptor templateDescriptor = 
		    root.findServerTemplateDescriptor(instanceDescriptor.template);
		assert templateDescriptor != null;
	    
		serverDescriptor = (ServerDescriptor)templateDescriptor.descriptor;
		assert serverDescriptor != null;

		instanceResolver = 
		    new Utils.Resolver(_resolver, 
				       instanceDescriptor.parameterValues,
				       templateDescriptor.parameterDefaults);
	    }

	    String serverId = instanceResolver.substitute(serverDescriptor.id);
	    assert serverId.equals(server.getId());

	    server.rebuild(instanceResolver, instanceDescriptor, serverDescriptor);
	    updatedChildren.add(server);
	}
	
	childrenChanged(updatedChildren);
	insertChildren(newChildren, true);  
    }

    Node(boolean brandNew, TreeNode parent, String nodeName, NodeDescriptor descriptor)
	throws UpdateFailedException
    {
	super(brandNew, parent, nodeName);
	_ephemeral = false;
	_descriptor = descriptor;

	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origLoadFactor = _descriptor.loadFactor;

	_resolver = new Utils.Resolver(new java.util.Map[]
	    {_descriptor.variables, getRoot().getVariables()});
			       
	_resolver.put("application", getRoot().getId());
	_resolver.put("node", _id);
	
	//
	// Template instances
	//
	java.util.Iterator p = _descriptor.serverInstances.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor instanceDescriptor = 
		(ServerInstanceDescriptor)p.next();
	   
	    insertChild(createServer(false, instanceDescriptor), false);
	}

	//
	// Plain servers
	//
	p = _descriptor.servers.iterator();
	while(p.hasNext())
	{
	    ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();
	    insertChild(createServer(false, serverDescriptor), false);
	}
    } 
    
    Node(TreeNode parent, String nodeName, NodeDescriptor descriptor)
    {
	super(false, parent, nodeName);
	_ephemeral = true;
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
		_editable.removeElement(server.getId());
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


    void tryAdd(ServerInstanceDescriptor instanceDescriptor,
		ServerDescriptor serverDescriptor,
		boolean addDescriptor) throws UpdateFailedException
    {
	if(instanceDescriptor != null)
	{
	    insertChild(createServer(true, instanceDescriptor),
			true);
	}
	else
	{
	    insertChild(createServer(true, serverDescriptor),
			true);
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

    private void newServer(ServerDescriptor descriptor)
    {
	descriptor.id = makeNewChildId(descriptor.id);
	
	Server server = new Server(this, descriptor.id, null, descriptor);
	try
	{
	    insertChild(server, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	getRoot().setSelectedNode(server);
    }

    private void newServer(ServerInstanceDescriptor descriptor)
    {
	String id = makeNewChildId("NewServer");
	Root root = getRoot();

	//
	// Make sure descriptor.template points to a real template
	//
	ServerTemplate t = root.findServerTemplate(descriptor.template);
	
	if(t == null)
	{
	    t = (ServerTemplate)root.getServerTemplates().getChildAt(0);
	    
	    if(t == null)
	    {
		JOptionPane.showMessageDialog(
		    getCoordinator().getMainFrame(),
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

	Server server = new Server(this, id, descriptor, sd);
	try
	{
	    insertChild(server, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	root.setSelectedNode(server);
    }

    private NodeDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private java.util.Map _origVariables;
    private String _origDescription;
    private String _origLoadFactor;

    private final boolean _ephemeral;
    private NodeEditor _editor;

    static private DefaultTreeCellRenderer _cellRenderer;   
    static private JPopupMenu _popup;
}
