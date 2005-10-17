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
import javax.swing.SwingUtilities;

import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.AMI_Admin_startServer;
import IceGrid.AMI_Admin_stopServer;
import IceGrid.AMI_Admin_enableServer;
import IceGrid.AMI_Admin_patchServer;

import IceGrid.AdapterDescriptor;
import IceGrid.DistributionDescriptor;
import IceGrid.IceBoxDescriptor;
import IceGrid.Model;
import IceGrid.PropertyDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;
import IceGrid.ServerDescriptor;
import IceGrid.ServerDynamicInfo;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.SimpleInternalFrame;
import IceGrid.Utils;

//
// We can have 4 kinds of servers:
// - Plain server (no template)
// - Icebox server (no template)
// - Server instance
// - Icebox instance
//
class Server extends EditableParent
{ 
    static public ServerDescriptor
    copyDescriptor(ServerDescriptor sd)
    {
	ServerDescriptor copy = (ServerDescriptor)sd.clone();
	copy.adapters = Adapters.copyDescriptors(copy.adapters);
	copy.dbEnvs = DbEnvs.copyDescriptors(copy.dbEnvs);

	//
	// Update to properties is not atomic because of Adapter endpoints
	// (and possibly other properties set through a PropertiesHolder)
	//
	copy.properties = (java.util.LinkedList)copy.properties.clone();
	copy.distrib = (DistributionDescriptor)copy.distrib.clone();

	if(copy instanceof IceBoxDescriptor)
	{
	    IceBoxDescriptor ib = (IceBoxDescriptor)copy;
	    ib.services = Services.copyDescriptors(ib.services);
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
	into.properties = from.properties;

	into.description = from.description;
	into.id = from.id;
	into.exe = from.exe;
	into.options = from.options;
	into.envs = from.envs;
	into.activation = from.activation;
	into.activationTimeout = from.activationTimeout;
	into.deactivationTimeout = from.deactivationTimeout;
	into.distrib.icepatch = from.distrib.icepatch;
	into.distrib.directories = from.distrib.directories;
    }

    static public ServerDescriptor newServerDescriptor()
    {
	return new ServerDescriptor(
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "",
	    "NewServer",
	    "",
	    "",
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "manual",
	    "0",
	    "0",
	    new IceGrid.DistributionDescriptor("", new java.util.LinkedList()));
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
	    new java.util.LinkedList()
	    );

	java.util.LinkedList adapterList = new java.util.LinkedList();
	adapterList.add(serviceManager);
	
	return new IceBoxDescriptor(
	    adapterList,
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "",
	    "NewIceBox",
	    "",
	    "",
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "manual",
	    "0",
	    "0",
	    new IceGrid.DistributionDescriptor("", new java.util.LinkedList()),
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
	
	Object clipboard = _model.getClipboard();
	if(clipboard != null && 
	   (clipboard instanceof ServerDescriptor
	    || clipboard instanceof ServerInstanceDescriptor))
	{
	    actions[PASTE] = true;
	}

	actions[DELETE] = true;
	actions[SUBSTITUTE_VARS] = !_ephemeral;
	
	if(_instanceDescriptor == null)
	{
	    actions[NEW_ADAPTER] = (_adapters != null && _services == null);
	    actions[NEW_SERVICE] = (_services != null);
	    actions[NEW_DBENV] = (_dbEnvs != null);
	}

	ServerState state = getState();
	if(state != null)
	{
	    actions[START] = state == ServerState.Inactive 
	    && _enabled;
	    actions[STOP] = state != ServerState.Inactive;
	    actions[ENABLE] = true || !_enabled;
	    actions[DISABLE] = true || _enabled;

	    if(!_model.isUpdateInProgress())
	    {
		actions[SERVER_INSTALL_DISTRIBUTION] = 
		    !_serverDescriptor.distrib.icepatch.equals("");
	    }
	}

	return actions;
    }
    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);

	    JMenuItem item = new JMenuItem(_model.getActions()[NEW_ADAPTER]);
	    item.setText("New adapter");
	    _popup.add(item);

	    item = new JMenuItem(_model.getActions()[NEW_DBENV]);
	    item.setText("New DbEnv");
	    _popup.add(item);

	    item = new JMenuItem(_model.getActions()[NEW_SERVICE]);
	    item.setText("New service");
	    _popup.add(item);

	    _popup.addSeparator();
	    _popup.add(_model.getActions()[START]);
	    _popup.add(_model.getActions()[STOP]);
	    _popup.addSeparator();
	    _popup.add(_model.getActions()[ENABLE]);
	    _popup.add(_model.getActions()[DISABLE]);
	    _popup.addSeparator();
	    _popup.add(_model.getActions()[SERVER_INSTALL_DISTRIBUTION]);
	}
	return _popup;
    }
    public void copy()
    {
	if(_instanceDescriptor != null)
	{
	    _model.setClipboard(copyDescriptor(_instanceDescriptor));
	}
	else
	{
	    _model.setClipboard(copyDescriptor(_serverDescriptor));
	}
	_model.getActions()[PASTE].setEnabled(true);
    }
    public void paste()
    {
	_parent.paste();
    }
    public void newAdapter()
    {
	_adapters.newAdapter();
    }
    public void newDbEnv()
    {
	_dbEnvs.newDbEnv();
    }
    public void newService()
    {
	_services.newService();
    }

    public void start()
    {
	final String prefix = "Starting server '" + _id + "'...";
	_model.getStatusBar().setText(prefix);

	AMI_Admin_startServer cb = new AMI_Admin_startServer()
	    {
		//
		// Called by another thread!
		//
		public void ice_response(boolean result)
		{
		    if(result)
		    {
			amiSuccess(prefix);
		    }
		    else
		    {
			amiFailure(prefix, null, (String)null);
		    }
		}
		
		public void ice_exception(Ice.UserException e)
		{
		    amiFailure(prefix, "Failed to start " + _id, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to start " + _id, e.toString());
		}
	    };
	
	try
	{   
	   _model.getAdmin().startServer_async(cb, _id);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to start " + _id, e.toString());
	}
	
	//
	// Recompute actions in case this comes from popup menu
	// 
	_model.showActions(_model.getSelectedNode());
    }
    
    public void stop()
    {
	final String prefix = "Stopping server '" + _id + "'...";
	_model.getStatusBar().setText(prefix);

	AMI_Admin_stopServer cb = new AMI_Admin_stopServer()
	    {
		//
		// Called by another thread!
		//
		public void ice_response()
		{
		    amiSuccess(prefix);
		}
		
		public void ice_exception(Ice.UserException e)
		{
		    amiFailure(prefix, "Failed to stop " + _id, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to stop " + _id, e.toString());
		}
	    };
	
	try
	{   
	   _model.getAdmin().stopServer_async(cb, _id);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to stop " + _id, e.toString());
	}
	
	//
	// Recompute actions in case this comes from popup menu
	// 
	_model.showActions(_model.getSelectedNode());
    }

    public void enable()
    {
	enableServer(true);
    }

    public void disable()
    {
	enableServer(false);
    }
    
    private void amiSuccess(final String prefix)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {
		public void run() 
		{
		    _model.getStatusBar().setText(prefix + "done.");
		}
	    });
    }

    private void amiFailure(String prefix, String title, Ice.UserException e)
    {
	if(e instanceof IceGrid.ServerNotExistException)
	{
	    amiFailure(prefix, title, "This server was not registered with the IceGrid Registry");
	}
	else if(e instanceof IceGrid.NodeUnreachableException)
	{
	    IceGrid.NodeUnreachableException nue = (IceGrid.NodeUnreachableException)e;
	    amiFailure(prefix, title, "Node '" + nue.name + "' is unreachable: " + nue.reason);
	}
	else
	{
	    amiFailure(prefix, title, e.toString());
	}
    }
    
    private void amiFailure(final String prefix, final String title, final String message)
    {
	SwingUtilities.invokeLater(new Runnable() 
	    {	
		public void run() 
		{
		    failure(prefix, title, message);
		}
	    });
    }

    private void failure(String prefix, String title, String message)
    {
	_model.getStatusBar().setText(prefix + "failed!");
	if(message != null)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		message,
		title,
		JOptionPane.ERROR_MESSAGE);
	}
    }


    private void enableServer(boolean enable)
    {
	final String prefix = (enable ?
	    "Enabling" : "Disabling") + " server '" + _id + "'...";
	
	final String action = enable ? "enable" : "disable";
	
	_model.getStatusBar().setText(prefix);

	AMI_Admin_enableServer cb = new AMI_Admin_enableServer()
	    {
		//
		// Called by another thread!
		//
		public void ice_response()
		{
		    amiSuccess(prefix);
		}
		
		public void ice_exception(Ice.UserException e)
		{
		    amiFailure(prefix, "Failed to " + action + " " + _id, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to " + action + " " + _id, e.toString());
		}
	    };
	
	try
	{   
	   _model.getAdmin().enableServer_async(cb, _id, enable);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to " + action + " " + _id, e.toString());
	}
       	
	//
	// Recompute actions in case this comes from popup menu
	// 
	_model.showActions(_model.getSelectedNode());

    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	propertiesFrame.setTitle("Properties for " + _id);
	
	//
	// Pick the appropriate editor
	//
	Editor editor = null;
	if(_instanceDescriptor == null)
	{
	    if(_serverEditor == null)
	    {
		_serverEditor = new ServerEditor(_model.getMainFrame());
	    }
	    _serverEditor.show(this);
	    propertiesFrame.setContent(_serverEditor.getComponent());
	}
	else
	{
	    if(_serverInstanceEditor == null)
	    {
		_serverInstanceEditor = new ServerInstanceEditor(_model.getMainFrame());
	    }
	    _serverInstanceEditor.show(this);
	    propertiesFrame.setContent(_serverInstanceEditor.getComponent());
	}

	propertiesFrame.validate();
	propertiesFrame.repaint();
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
	    _icons = new Icon[7];
	    _icons[0] = Utils.getIcon("/icons/unknown.png");
	    _icons[ServerState.Inactive.value() + 1] = Utils.getIcon("/icons/inactive.png");
	    _icons[ServerState.Activating.value() + 1] = 
		Utils.getIcon("/icons/activating.png");
	    _icons[ServerState.Active.value() + 1] = 
		Utils.getIcon("/icons/active.png");
	    _icons[ServerState.Deactivating.value() + 1] = 
		Utils.getIcon("/icons/deactivating.png");
	    _icons[ServerState.Destroying.value() + 1] = 
		Utils.getIcon("/icons/destroying.png");
	    _icons[ServerState.Destroyed.value() + 1] = 
		Utils.getIcon("/icons/destroyed.png");
	}

	//
	// TODO: separate icons for open and close
	//
	if(expanded)
	{
	    _cellRenderer.setOpenIcon(_icons[_stateIconIndex]);
	}
	else
	{
	    _cellRenderer.setClosedIcon(_icons[_stateIconIndex]);
	}

	_cellRenderer.setToolTipText(_toolTip);
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    public PropertiesHolder getPropertiesHolder()
    {
	return _propertiesHolder;
    }

    public boolean destroy()
    {
	if(_parent == null)
	{
	    return false;
	}
	Node node = (Node)_parent;
	
	if(_ephemeral)
	{
	    node.removeChild(this, true);
	    return true;
	}
	else if(_model.canUpdate())
	{
	    if(_instanceDescriptor != null)
	    {
		node.removeDescriptor(_instanceDescriptor);
	    }
	    else
	    {
		node.removeDescriptor(_serverDescriptor);
	    }
	    node.removeElement(this, true);
	    return true;
	}
	return false;
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
	    clone.distrib = (IceGrid.DistributionDescriptor)clone.distrib.clone();
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
	    
	    ServerTemplate t = getApplication().findServerTemplate(_instanceDescriptor.template);
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
    Server(boolean brandNew, String serverId, 
	   Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor,
	   ServerDescriptor serverDescriptor,
	   Application application) throws UpdateFailedException
    {
	super(brandNew, serverId, application.getModel());
	_ephemeral = false;
	rebuild(resolver, instanceDescriptor, serverDescriptor, application);
    }

    Server(String serverId, ServerInstanceDescriptor instanceDescriptor, 
	   ServerDescriptor serverDescriptor, Model model)
    {
	super(false, serverId, model);
	_ephemeral = true;
	try
	{
	    rebuild(null, instanceDescriptor, serverDescriptor, null);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

    //
    // Update the server and all its subtree
    //
    void rebuild(Utils.Resolver resolver,  
		 ServerInstanceDescriptor instanceDescriptor,
		 ServerDescriptor serverDescriptor,
		 Application application) throws UpdateFailedException
    {
	assert serverDescriptor != null;
	_resolver = resolver;
	_instanceDescriptor = instanceDescriptor;
	_serverDescriptor = serverDescriptor;
	clearChildren();
	
	boolean isEditable = (instanceDescriptor == null);
	_propertiesHolder = new PropertiesHolder(serverDescriptor);	

	if(_ephemeral)
	{
	    _services = null;
	    _dbEnvs = null;
	    _adapters = null;
	}
	else
	{
	    try
	    {
		if(serverDescriptor instanceof IceBoxDescriptor)
		{
		    IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)serverDescriptor;
		    
		    _services = new Services(iceBoxDescriptor.services,
					     isEditable, _resolver, application);
		    addChild(_services);
		    //
		    // IceBox has not dbEnv
		    //
		    assert serverDescriptor.dbEnvs.size() == 0;
		    _dbEnvs = null;
		}
		else
		{
		    _services = null;   	
		    _dbEnvs = new DbEnvs(serverDescriptor.dbEnvs, 
					 isEditable, _resolver, _model);
		    addChild(_dbEnvs);
		}
		
		_adapters = new Adapters(serverDescriptor.adapters, 
					 isEditable, _services != null, 
					 _resolver, _model);
		addChild(_adapters);
	    }
	    catch(UpdateFailedException e)
	    {
		e.addParent(this);
		throw e;
	    }
	}
    }

    
    public void setParent(CommonBase parent)
    {
	if(!_ephemeral)
	{
	    Ice.IntHolder pid = new Ice.IntHolder();
	    Ice.BooleanHolder enabled = new Ice.BooleanHolder();
	    _state = _model.getRoot().registerServer(_resolver.find("node"),
						     this,
						     pid, enabled);
	    
	    _pid = pid.value;
	    _enabled = enabled.value;
	    _toolTip = toolTip(_state, _pid, _enabled);
	    if(_state != null)
	    {
		_stateIconIndex = _state.value() + 1;
	    }
	}
	super.setParent(parent);
    }

    public void clearParent()
    {
	if(_parent != null)
	{
	    if(!_ephemeral)
	    {
		_model.getRoot().unregisterServer(_resolver.find("node"),
						  this);
	    }
	    super.clearParent();
	}
    }
    
    java.util.List findServiceInstances(String template)
    {
	if(_services != null)
	{
	    return _services.findServiceInstances(template);
	}
	else
	{
	    return new java.util.LinkedList();
	}
    }

    void removeServiceInstances(String template)
    {
	if(_services != null)
	{
	    _services.removeServiceInstances(template);
	}
    }

    
    void updateDynamicInfo(ServerState state, int pid, boolean enabled)
    {
	if(state != _state || pid != _pid || enabled != _enabled)
	{
	    _state = state;
	    _pid = pid;
	    _enabled = enabled;
	    
	    _toolTip = toolTip(_state, _pid, _enabled);
	    _stateIconIndex = _state.value() + 1;
	
	    //
	    // Change the node representation
	    //
	    fireNodeChangedEvent(this);
	}
    }

    ServerState getState()
    {
	return _state;
    }

    ServerInstanceDescriptor getInstanceDescriptor()
    {
	return _instanceDescriptor;
    }

    ServerDescriptor getServerDescriptor()
    {
	return _serverDescriptor;
    }

    void setServerDescriptor(ServerDescriptor sd)
    {
	_serverDescriptor = sd;
    }

    Services getServices()
    {
	return _services;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    Utils.Resolver getParentResolver()
    {
	if(_parent != null)
	{
	    return ((Node)_parent).getResolver();
	}
	else
	{
	    return null;
	}
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

    static private String toolTip(ServerState state, int pid, boolean enabled)
    {
	String result = (state == null ? "Unknown" : state.toString());
	
	if(!enabled)
	{
	    result += ", disabled";
	}

	if(pid != 0)
	{
	    result += ", pid: " + pid;
	}
	return result;
    }

    private ServerState _state = null;
    private boolean _enabled = true;
    private int _stateIconIndex = 0;
    private int _pid = 0;
    private String _toolTip;

    private ServerInstanceDescriptor _instanceDescriptor;
    private ServerDescriptor _serverDescriptor;
    private final boolean _ephemeral;

    private Utils.Resolver _resolver;

    private PropertiesHolder _propertiesHolder;

    //
    // Children
    //
    private Services _services;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[] _icons;
 
    static private ServerEditor _serverEditor;
    static private ServerInstanceEditor _serverInstanceEditor;
    static private JPopupMenu _popup;
}
