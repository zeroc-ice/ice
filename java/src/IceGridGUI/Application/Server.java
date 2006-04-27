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
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

//
// We can have 4 kinds of servers:
// - Plain server (no template)
// - Icebox server (no template)
// - Server instance
// - Icebox instance
//
class Server extends Communicator implements TemplateInstance
{ 
    static public ServerDescriptor
    copyDescriptor(ServerDescriptor sd)
    {
	ServerDescriptor copy = (ServerDescriptor)sd.clone();

	copy.adapters = Adapter.copyDescriptors(copy.adapters);
	copy.dbEnvs = DbEnv.copyDescriptors(copy.dbEnvs);

	//
	// Update to properties is not atomic because of Adapter endpoints
	// (and possibly other properties set through a PropertiesHolder)
	//
	copy.propertySet = (PropertySetDescriptor)copy.propertySet.clone();
	copy.propertySet.references = (String[])copy.propertySet.references.clone();
	copy.propertySet.properties = (java.util.LinkedList)copy.propertySet.properties.clone();
	copy.distrib = (DistributionDescriptor)copy.distrib.clone();

	if(copy instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor ib = (IceBoxDescriptor)copy;
	    ib.services = Service.copyDescriptors(ib.services);
	}
	return copy;
    }

    static public ServerInstanceDescriptor
    copyDescriptor(ServerInstanceDescriptor sid)
    {
	return (ServerInstanceDescriptor)sid.clone();
    }
    
    static public void shallowRestore(ServerDescriptor from, ServerDescriptor into)
    {
	//
	// When editing a server or server template, if we update properties, 
	// we replace the entire field
	into.propertySet = from.propertySet;

	into.description = from.description;
	into.id = from.id;
	into.exe = from.exe;
	into.options = from.options;
	into.envs = from.envs;
	into.activation = from.activation;
	into.activationTimeout = from.activationTimeout;
	into.deactivationTimeout = from.deactivationTimeout;
	into.applicationDistrib = from.applicationDistrib;
	into.distrib.icepatch = from.distrib.icepatch;
	into.distrib.directories = from.distrib.directories;
    }

    static public ServerDescriptor newServerDescriptor()
    {
	return new ServerDescriptor(
	    new java.util.LinkedList(),
	    new PropertySetDescriptor(new String[0], new java.util.LinkedList()),
	    new java.util.LinkedList(),
	    "",
	    "NewServer",
	    "",
	    "",
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "manual",
	    "",
	    "",
	    true,
	    new DistributionDescriptor("", new java.util.LinkedList()),
	    false // Allocatable
	    );
    }

    static public IceBoxDescriptor newIceBoxDescriptor()
    {
	AdapterDescriptor serviceManager = new AdapterDescriptor(
	    "IceBox.ServiceManager",
	    "", 
	    "", // direct-adapter by default
	    "",
	    true,
	    true,
	    new java.util.LinkedList(),
	    false // Allocatable
	    );

	java.util.LinkedList adapterList = new java.util.LinkedList();
	adapterList.add(serviceManager);
	
	PropertyDescriptor pd = 
	    new PropertyDescriptor("IceBox.ServiceManager.Endpoints",
				   "tcp -h 127.0.0.1");
	java.util.LinkedList properties = new java.util.LinkedList();
	properties.add(pd);

	return new IceBoxDescriptor(
	    adapterList,
	    new PropertySetDescriptor(new String[0], properties),
	    new java.util.LinkedList(),
	    "",
	    "NewIceBox",
	    "",
	    "",
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "manual",
	    "",
	    "",
	    true,
	    new DistributionDescriptor("", new java.util.LinkedList()),
	    false, // Allocatable
	    new java.util.LinkedList()
	    );
    }
    
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	Object clipboard = getCoordinator().getClipboard();
	if(clipboard != null && 
	   (clipboard instanceof ServerDescriptor
	    || clipboard instanceof ServerInstanceDescriptor))
	{
	    actions[PASTE] = true;
	}

	actions[DELETE] = true;
	if(!_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;
	}
	
	if(_instanceDescriptor == null)
	{
	    actions[NEW_ADAPTER] = !_services.initialized();
	    actions[NEW_SERVICE] = _services.initialized();
	    actions[NEW_DBENV] = !_services.initialized();
	}
       
	return actions;
    }
    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = getCoordinator().getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_ADAPTER));
	    _popup.add(actions.get(NEW_DBENV));
	    _popup.add(actions.get(NEW_SERVICE));
	    _popup.add(actions.get(NEW_SERVICE_FROM_TEMPLATE));
	}
	actions.setTarget(this);
	return _popup;
    }
    public void copy()
    {
	if(_instanceDescriptor != null)
	{
	    getCoordinator().setClipboard(copyDescriptor(_instanceDescriptor));
	}
	else
	{
	    getCoordinator().setClipboard(copyDescriptor(_serverDescriptor));
	}
	getCoordinator().getActionsForMenu().get(PASTE).setEnabled(true);
    }
    
    public Editor getEditor()
    {
	//
	// Pick the appropriate editor
	//
	Editor editor = null;
	if(_instanceDescriptor == null)
	{
	    if(_serverEditor == null)
	    {
		_serverEditor = (ServerEditor)getRoot().getEditor(ServerEditor.class, this);
	    }
	    _serverEditor.show(this);
	    return _serverEditor;
	}
	else
	{
	    if(_serverInstanceEditor == null)
	    {
		_serverInstanceEditor = (ServerInstanceEditor)
		    getRoot().getEditor(ServerInstanceEditor.class, this);
	    }
	    _serverInstanceEditor.show(this);
	    return _serverInstanceEditor;
	}
    }

    protected Editor createEditor()
    {
	if(_instanceDescriptor == null)
	{
	    return new ServerEditor(getCoordinator().getMainFrame());
	}
	else
	{
	    return new ServerInstanceEditor(getCoordinator().getMainFrame());
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

	    _serverIcon = Utils.getIcon("/icons/16x16/server_inactive.png");
	    _iceboxServerIcon = Utils.getIcon("/icons/16x16/icebox_server_inactive.png");
	}
	
	boolean icebox = _serverDescriptor instanceof IceBoxDescriptor;

	if(expanded)
	{	
	    _cellRenderer.setOpenIcon(icebox ? _iceboxServerIcon : _serverIcon);
	}
	else
	{
	    _cellRenderer.setClosedIcon(icebox ? _iceboxServerIcon : _serverIcon);
	} 
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public void destroy()
    {
	Node node = (Node)_parent;
	
	if(_ephemeral)
	{
	    node.removeChild(this);
	}
	else
	{
	    if(_instanceDescriptor != null)
	    {
		node.removeDescriptor(_instanceDescriptor);
	    }
	    else
	    {
		node.removeDescriptor(_serverDescriptor);
	    }
	    node.removeChild(this);
	    node.getEditable().removeElement(_id);
	    getRoot().updated();
	}
    }


    public Object getDescriptor()
    {
	if(_instanceDescriptor != null)
	{
	    return _instanceDescriptor;
	}
	else
	{
	    return _serverDescriptor;
	}
    }

    public Object saveDescriptor()
    {
	if(_instanceDescriptor != null)
	{
	    return _instanceDescriptor.clone();
	}
	else
	{
	    ServerDescriptor clone = (ServerDescriptor)_serverDescriptor.clone();
	    clone.distrib = (DistributionDescriptor)clone.distrib.clone();
	    return clone;
	}
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	if(_instanceDescriptor != null)
	{   
	    ServerInstanceDescriptor copy = (ServerInstanceDescriptor)savedDescriptor;

	    _instanceDescriptor.template = copy.template;
	    _instanceDescriptor.parameterValues = copy.parameterValues;
	    
	    ServerTemplate t = getRoot().findServerTemplate(_instanceDescriptor.template);
	    _serverDescriptor = (ServerDescriptor)
		((TemplateDescriptor)t.getDescriptor()).descriptor;
	}
	else
	{
	    shallowRestore((ServerDescriptor)savedDescriptor, _serverDescriptor);
	}
    }

    //
    // Builds the server and all its sub-tree
    //
    Server(boolean brandNew, TreeNode parent, String serverId, 
	   Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor,
	   ServerDescriptor serverDescriptor) throws UpdateFailedException
    {
	super(parent, serverId);
	_ephemeral = false;
	_editable = new Editable(brandNew);
	rebuild(resolver, instanceDescriptor, serverDescriptor);
    }

    Server(TreeNode parent, String serverId, ServerInstanceDescriptor instanceDescriptor, 
	   ServerDescriptor serverDescriptor)
    {
	super(parent, serverId);
	_ephemeral = true;
	_editable = null;
	try
	{
	    rebuild(null, instanceDescriptor, serverDescriptor);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }


    static java.util.List createAttributes(ServerDescriptor descriptor)
    {
	java.util.List attributes = new java.util.LinkedList();
	attributes.add(createAttribute("id", descriptor.id));
	if(descriptor.activation.length() > 0)
	{
	    attributes.add(createAttribute("activation", 
					   descriptor.activation));
	}
	if(descriptor.activationTimeout.length() > 0)
	{
	    attributes.add(createAttribute("activation-timeout", 
					   descriptor.activationTimeout));
	}
	if(!descriptor.applicationDistrib)
	{
	    attributes.add(createAttribute("application-distrib", "false")); 
	}
	if(descriptor.deactivationTimeout.length() > 0)
	{
	    attributes.add(createAttribute("deactivation-timeout", 
					   descriptor.deactivationTimeout));
	}
	if(descriptor.exe.length() > 0)
	{
	    attributes.add(createAttribute("exe", descriptor.exe));
	}
	if(descriptor.pwd.length() > 0)
	{
	    attributes.add(createAttribute("pwd", descriptor.pwd));
	}

	return attributes;
    }
    
    static void writeOptions(XMLWriter writer, java.util.List options)
	throws java.io.IOException
    {
	java.util.Iterator p = options.iterator();
	while(p.hasNext())
	{
	    writer.writeElement("option", (String)p.next()); 
	}
    }
    
    static void writeEnvs(XMLWriter writer, java.util.List envs)
	throws java.io.IOException
    {
	java.util.Iterator p = envs.iterator();
	while(p.hasNext())
	{
	    writer.writeElement("env", (String)p.next()); 
	}
    }

    void write(XMLWriter writer) throws java.io.IOException
    {
	if(!_ephemeral)
	{
	    if(_instanceDescriptor != null)
	    {
		TemplateDescriptor templateDescriptor 
		    = getRoot().findServerTemplateDescriptor(_instanceDescriptor.template);

		java.util.LinkedList attributes = parameterValuesToAttributes(
		    _instanceDescriptor.parameterValues, templateDescriptor.parameters);
		attributes.addFirst(createAttribute("template", _instanceDescriptor.template));
		
       
		writer.writeElement("server-instance", attributes);
	    }
	    else if(isIceBox())
	    {
		writer.writeStartTag("icebox", 
				     createAttributes(_serverDescriptor));

		if(_serverDescriptor.description.length() > 0)
		{
		    writer.writeElement("description", _serverDescriptor.description);
		}
		writeOptions(writer, _serverDescriptor.options);
		writeEnvs(writer, _serverDescriptor.envs);
		//
		// TODO: BENOIT: Add refernces
		//
		writeProperties(writer, _serverDescriptor.propertySet.properties);
		writeDistribution(writer, _serverDescriptor.distrib);

		_adapters.write(writer);
		_services.write(writer);
		writer.writeEndTag("icebox");
	    }
	    else
	    {
		writer.writeStartTag("server", createAttributes(_serverDescriptor));

		if(_serverDescriptor.description.length() > 0)
		{
		    writer.writeElement("description", _serverDescriptor.description);
		}
		
		writeOptions(writer, _serverDescriptor.options);
		writeEnvs(writer, _serverDescriptor.envs);
		//
		// TODO: BENOIT: Add references
		//
		writeProperties(writer, _serverDescriptor.propertySet.properties);
		writeDistribution(writer, _serverDescriptor.distrib);

		_adapters.write(writer);
		_dbEnvs.write(writer);
		writer.writeEndTag("server");
	    }
	}
    }


    boolean isIceBox()
    {
	return _serverDescriptor instanceof IceBoxDescriptor;
    }

    static private class Backup
    {
	Backup(Editable ne)
	{
	    nodeEditable = ne;
	}

	Editable nodeEditable;
	java.util.Map parameterValues;
    }

    public Object rebuild(java.util.List editables) throws UpdateFailedException
    {
	Backup backup = new Backup(((Node)_parent).getEditable());
	Node node = (Node)_parent;
	Server newServer = null;

	if(_instanceDescriptor != null)
	{
	    TemplateDescriptor templateDescriptor 
		= getRoot().findServerTemplateDescriptor(_instanceDescriptor.template);

	    java.util.Set parameters = new java.util.HashSet(templateDescriptor.parameters);
	    if(!parameters.equals(_instanceDescriptor.parameterValues.keySet()))
	    {
		backup.parameterValues = _instanceDescriptor.parameterValues;
		_instanceDescriptor.parameterValues = EditorBase.makeParameterValues(
		    _instanceDescriptor.parameterValues, templateDescriptor.parameters);
	    }
	    newServer = node.createServer(false, _instanceDescriptor);
	}
	else
	{
	    newServer = node.createServer(false, _serverDescriptor);
	}

	if(_id.equals(newServer.getId()))
	{
	    //
	    // A simple update. We can't simply rebuild server because 
	    // we need to keep a backup
	    //
	    if(_editable.isModified())
	    {
		newServer.getEditable().markModified();
	    }

	    node.removeChild(this);	    
	    try
	    {
		node.insertChild(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		assert false; // impossible, we just removed a child with
		              // this id
	    }

	    if(backup.parameterValues != null)
	    {
		editables.add(newServer.getEditable());
	    }
	}
	else
	{
	    newServer.getEditable().markNew();
	    node.removeChild(this);
	    node.getEditable().removeElement(_id);
	    try
	    {
		node.insertChild(newServer, true);
	    }
	    catch(UpdateFailedException e)
	    {
		restore(backup);
		throw e;
	    }
	}
	
	return backup;
    }

    public void restore(Object backupObj)
    {
	Backup backup = (Backup)backupObj;
	Node node = (Node)_parent;
	
	node.getEditable().restore(backup.nodeEditable);

	if(_instanceDescriptor != null &&  backup.parameterValues != null)
	{
	    _instanceDescriptor.parameterValues = backup.parameterValues;
	}

	TreeNode badServer = null;

	if(_instanceDescriptor != null)
	{
	    badServer = node.findChildWithDescriptor(_instanceDescriptor);
	}
	else
	{
	    badServer = node.findChildWithDescriptor(_serverDescriptor);
	}
	
	if(badServer != null)
	{
	    node.removeChild(badServer);
	}

	try
	{
	    node.insertChild(this, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
    }

    void commit()
    {
	_editable.commit();
    }


    void setServerDescriptor(ServerDescriptor descriptor)
    {
	_serverDescriptor = descriptor;
    }

    //
    // Update the server and all its subtree
    //
    void rebuild(Utils.Resolver resolver,  
		 ServerInstanceDescriptor instanceDescriptor,
		 ServerDescriptor serverDescriptor) throws UpdateFailedException
    {
	assert serverDescriptor != null;
	_resolver = resolver;
	_instanceDescriptor = instanceDescriptor;
	_serverDescriptor = serverDescriptor;

	_adapters.clear();
	_dbEnvs.clear();
	_services.clear();

	if(!_ephemeral)
	{
	    _adapters.init(_serverDescriptor.adapters);
	    if(_serverDescriptor instanceof IceBoxDescriptor)
	    {
		IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)_serverDescriptor;
		_services.init(iceBoxDescriptor.services);
   
		//
		// IceBox has not dbEnv
		//
		assert _serverDescriptor.dbEnvs.size() == 0;
	    }
	    else
	    {
		_dbEnvs.init(_serverDescriptor.dbEnvs);
	    }
	}
    }

   
    ServerInstanceDescriptor getInstanceDescriptor()
    {
	return _instanceDescriptor;
    }

    ServerDescriptor getServerDescriptor()
    {
	return _serverDescriptor;
    }

    CommunicatorDescriptor getCommunicatorDescriptor()
    {
	return _serverDescriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    boolean isEditable()
    {
	return _instanceDescriptor == null;
    }
    
    Editable getEditable()
    {
	return _editable;
    }

    Editable getEnclosingEditable()
    {
	return _editable;
    }

    java.util.List findInstances(boolean includeTemplate)
    {
	java.util.List result = new java.util.LinkedList();
	result.add(this);
	return result;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public String toString()
    {
	if(_instanceDescriptor == null || _ephemeral)
	{
	    return super.toString();
	}
	else
	{
	    return _id + ": " + _instanceDescriptor.template + "<>";
	}
    }

    private ServerInstanceDescriptor _instanceDescriptor;
    private ServerDescriptor _serverDescriptor;
    private final boolean _ephemeral;

    private ServerEditor _serverEditor;
    private ServerInstanceEditor _serverInstanceEditor;

    private Utils.Resolver _resolver;
    private Editable _editable;
    
    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _serverIcon;
    static private Icon _iceboxServerIcon;
 
    static private JPopupMenu _popup;
}
