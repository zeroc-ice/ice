// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.Cursor;
import java.awt.Component;

import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.SwingUtilities;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.AMI_Admin_patchApplication;
import IceGrid.AdapterDynamicInfo;
import IceGrid.ApplicationDescriptor;
import IceGrid.ApplicationUpdateDescriptor;
import IceGrid.DistributionDescriptor;
import IceGrid.Model;
import IceGrid.NodeInfo;
import IceGrid.ServerDynamicInfo;
import IceGrid.ServerState;
import IceGrid.SimpleInternalFrame;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;


public class Application extends EditableParent
{
    static public ApplicationDescriptor
    copyDescriptor(ApplicationDescriptor ad)
    {
	ApplicationDescriptor copy = (ApplicationDescriptor)ad.clone();
	
	copy.replicaGroups = 
	    ReplicaGroups.copyDescriptors(copy.replicaGroups);
	
	copy.serverTemplates = 
	    ServerTemplates.copyDescriptors(copy.serverTemplates);
	
	copy.serviceTemplates = 
	    ServiceTemplates.copyDescriptors(copy.serviceTemplates);
	
	copy.nodes = Nodes.copyDescriptors(copy.nodes);

	copy.distrib = (DistributionDescriptor)copy.distrib.clone();
	return copy;
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
	    _cellRenderer = new DefaultTreeCellRenderer();
	    _cellRenderer.setOpenIcon(
		Utils.getIcon("/icons/16x16/application_open.png"));
	    _cellRenderer.setClosedIcon(
		Utils.getIcon("/icons/16x16/application_closed.png"));
	}

	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	actions[COPY] = true;
	actions[DELETE] = true;

	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof ApplicationDescriptor;
	}
	if(!_model.isUpdateInProgress() && _model.getAdmin() != null)
	{
	    actions[APPLICATION_INSTALL_DISTRIBUTION] = 
		!_descriptor.distrib.icepatch.equals("");
	}

	if(!_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;
	}
	
	actions[NEW_NODE] = (_nodes != null);
	actions[NEW_REPLICA_GROUP] = (_replicaGroups != null);
	actions[NEW_TEMPLATE_SERVER] = (_serverTemplates != null);
	actions[NEW_TEMPLATE_SERVER_ICEBOX] = (_serverTemplates != null);
	actions[NEW_TEMPLATE_SERVICE] = (_serviceTemplates != null);

	return actions;
    }
    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	
	    JMenuItem newNodeItem = new JMenuItem(_model.getActions()[NEW_NODE]);
	    newNodeItem.setText("New node");
	    _popup.add(newNodeItem);
	    
	    JMenuItem newReplicaGroupItem = new JMenuItem(_model.getActions()[NEW_REPLICA_GROUP]);
	    newReplicaGroupItem.setText("New replica group");
	    _popup.add(newReplicaGroupItem);
	     _popup.addSeparator();

	    JMenuItem newServerItem = new JMenuItem(_model.getActions()[NEW_TEMPLATE_SERVER]);
	    newServerItem.setText("New server template");
	    _popup.add(newServerItem);
	    JMenuItem newIceBoxItem = 
		new JMenuItem(_model.getActions()[NEW_TEMPLATE_SERVER_ICEBOX]);
	    newIceBoxItem.setText("New IceBox server template");
	    _popup.add(newIceBoxItem); 
	    _popup.addSeparator();

	    JMenuItem newServiceItem = new JMenuItem(_model.getActions()[NEW_TEMPLATE_SERVICE]);
	    newServiceItem.setText("New service template");
	    _popup.add(newServiceItem);
	    _popup.addSeparator();

	    _popup.add(_model.getActions()[APPLICATION_INSTALL_DISTRIBUTION]);
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
	_parent.paste();
    }
    public void newNode()
    {
	_nodes.newNode();
    }
    public void newReplicaGroup()
    {
	_replicaGroups.newReplicaGroup();
    }
    public void newTemplateServer()
    {
	_serverTemplates.newTemplateServer();
    }
    public void newTemplateServerIceBox()
    {
	_serverTemplates.newTemplateServerIceBox();
    }
    public void newTemplateService()
    {
	_serviceTemplates.newTemplateService();
    }
    
    public void applicationInstallDistribution()
    {
	int shutdown = JOptionPane.showConfirmDialog(
	    _model.getMainFrame(),
	    "You are about to install or refresh" 
	    + " the distribution of your application onto this node.\n"
	    + " Do you want shut down all servers affected by this update?", 
	    "Patch Confirmation",
	    JOptionPane.YES_NO_CANCEL_OPTION);
       
	if(shutdown == JOptionPane.CANCEL_OPTION)
	{
	    //
	    // Recompute actions in case this comes from popup menu
	    // 
	    _model.showActions(_model.getSelectedNode());

	    return;
	}

	final String prefix = "Patching application '" + _id + "'...";
	_model.getStatusBar().setText(prefix);

	AMI_Admin_patchApplication cb = new AMI_Admin_patchApplication()
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
		    amiFailure(prefix, "Failed to patch " + _id, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to patch " + _id, e.toString());
		}
	    };
	
	try
	{   
	    _model.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

	   _model.getAdmin().patchApplication_async(cb, _id, 
						    shutdown == JOptionPane.YES_OPTION);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to patch " + _id, e.toString());
	}
	finally
	{
	    _model.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}


	//
	// Recompute actions in case this comes from popup menu
	// 
	_model.showActions(_model.getSelectedNode());
    }

    public ApplicationUpdateDescriptor createUpdateDescriptor()
    {
	ApplicationUpdateDescriptor update = new ApplicationUpdateDescriptor();
	update.name = _descriptor.name;
	if(isModified())
	{
	    //
	    // Diff description
	    //
	    if(!_descriptor.description.equals(_origDescription))
	    {
		update.description = 
		    new IceGrid.BoxedString(_descriptor.description);
	    }

	    //
	    // Diff variables
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

	    //
	    // Diff distribution
	    //
	    if(!_descriptor.distrib.equals(_origDistrib))
	    {
		update.distrib = new IceGrid.BoxedDistributionDescriptor(
		    _descriptor.distrib);
	    }
	}
	else
	{
	    update.variables = new java.util.TreeMap();
	    update.removeVariables = new String[0];
	}

	//
	// Replicated Adapters
	//
	update.removeReplicaGroups = _replicaGroups.removedElements();
	update.replicaGroups = _replicaGroups.getUpdates();
	
	//
	// Server Templates
	//
	update.removeServerTemplates = _serverTemplates.removedElements();
	update.serverTemplates = _serverTemplates.getUpdates();

	//
	// Service Templates
	//
	update.removeServiceTemplates = _serviceTemplates.removedElements();
	update.serviceTemplates =_serviceTemplates.getUpdates();

	//
	// Nodes
	//
	update.removeNodes = _nodes.removedElements();
	update.nodes = _nodes.getUpdates();


	//
	// Return null if nothing changed
	//
	if(!isModified() &&
	   update.removeReplicaGroups.length == 0 &&
	   update.replicaGroups.size() == 0 &&
	   update.removeServerTemplates.length == 0 &&
	   update.serverTemplates.size() == 0 &&
	   update.removeServiceTemplates.length == 0 &&
	   update.serviceTemplates.size() == 0 &&
	   update.removeNodes.length == 0 &&
	   update.nodes.size() == 0)
	{
	    return null;
	}
	else
	{
	    return update;
	}
    }
    
    public void commit()
    {
	super.commit();
	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origDistrib = (DistributionDescriptor)_descriptor.distrib.clone();
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new ApplicationEditor(_model.getMainFrame());
	}
	_editor.show(this);
	return _editor;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public Object saveDescriptor()
    {
	ApplicationDescriptor clone = (ApplicationDescriptor)_descriptor.clone();
	clone.distrib = (IceGrid.DistributionDescriptor)clone.distrib.clone();
	return clone;
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	ApplicationDescriptor clone = (ApplicationDescriptor)savedDescriptor;

	_descriptor.variables = clone.variables;
	_descriptor.distrib.icepatch = clone.distrib.icepatch;
	_descriptor.distrib.directories = clone.distrib.directories;
	_descriptor.description = clone.description;
    }
    
    public boolean destroy()
    {
	if(_parent == null)
	{
	    return false;
	}
	Root root = (Root)_parent;
	
	if(_ephemeral || isNew())
	{
	    root.removeChild(this, true);
	    return true;
	}
	else if(_model.canUpdate())
	{
	    int confirm = JOptionPane.showConfirmDialog(
		_model.getMainFrame(),
		"This will immediately remove this application from "
		+ "the IceGrid Registry. Do you want to proceed?", 
		"Remove Confirmation",
		JOptionPane.YES_NO_OPTION);

	    //
	    // Asks for confirmation
	    //
	    if(confirm == JOptionPane.YES_OPTION)
	    {
		if(_model.removeApplication(_id))
		{
		    root.removeChild(this, true);
		    return true;
		}
	    }
	}
	return false;
    }


    //
    // Builds the application and all its subtrees
    //
    Application(boolean brandNew, ApplicationDescriptor descriptor, Model model)
	throws UpdateFailedException
    {
	super(brandNew, descriptor.name, model);
	_ephemeral = false;
	_descriptor = descriptor;
	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origDistrib = (DistributionDescriptor)_descriptor.distrib.clone();

	_resolver = new Utils.Resolver(_descriptor.variables);
	_resolver.put("application", descriptor.name);

	_replicaGroups = new ReplicaGroups(_descriptor.replicaGroups,
					   _model);
	addChild(_replicaGroups);

	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates,
						 _model);	
	addChild(_serviceTemplates);

	_serverTemplates = new ServerTemplates(descriptor.serverTemplates,
					       this);
	addChild(_serverTemplates);
	
	_nodes = new Nodes(_descriptor.nodes, this);
	addChild(_nodes);
    }

    Application(ApplicationDescriptor descriptor, Model model)
    {
	super(false, descriptor.name, model);
	_ephemeral = true;
	_descriptor = descriptor;
    }

    //
    // Try to rebuild this application;
    // no-op if it fails!
    //
    void rebuild() throws UpdateFailedException
    {
	Utils.Resolver oldResolver = _resolver;
	_resolver = new Utils.Resolver(_descriptor.variables);
	_resolver.put("application", _id);

	try
	{
	    _nodes.rebuild();
	}
	catch(UpdateFailedException e)
	{
	    _resolver = oldResolver;
	    throw e;
	}
    }

    //
    // Called when a server-template is deleted, to remove all
    // corresponding instances
    // children.
    //
    void removeServerInstances(String templateId)
    {
	_nodes.removeServerInstances(templateId);
    }

    //
    // Called when a service-template is deleted, to remove all
    // corresponding instances
    //
    void removeServiceInstances(String templateId)
    {
	_nodes.removeServiceInstances(templateId);
	_serverTemplates.removeServiceInstances(templateId);
    }

    void update(ApplicationUpdateDescriptor desc)
	throws UpdateFailedException
    {
	//
	// Description
	//
	if(desc.description != null)
	{
	    _descriptor.description = desc.description.value;
	    _origDescription = _descriptor.description;
	}

	//
	// Variables
	//
	for(int i = 0; i < desc.removeVariables.length; ++i)
	{
	    _descriptor.variables.remove(desc.removeVariables[i]);
	}
	_descriptor.variables.putAll(desc.variables);

	//
	// Distrib
	//
	if(desc.distrib != null)
	{
	    _descriptor.distrib = desc.distrib.value;
	    _origDistrib = (DistributionDescriptor)_descriptor.distrib.clone();
	}

	//
	// Replica groups
	//
	for(int i = 0; i < desc.removeReplicaGroups.length; ++i)
	{
	    _descriptor.replicaGroups.remove(desc.
						  removeReplicaGroups[i]);
	}
	_descriptor.replicaGroups.addAll(desc.replicaGroups);
	_replicaGroups.update(desc.replicaGroups, 
				   desc.removeReplicaGroups);


	//
	// Service templates
	//
	for(int i = 0; i < desc.removeServiceTemplates.length; ++i)
	{
	    _descriptor.serviceTemplates.remove(desc.
						removeServiceTemplates[i]);
	}
	_descriptor.serviceTemplates.putAll(desc.serviceTemplates);
	_serviceTemplates.update(desc.serviceTemplates, 
				 desc.removeServiceTemplates);

	//
	// Server templates
	//
	for(int i = 0; i < desc.removeServerTemplates.length; ++i)
	{
	    _descriptor.serverTemplates.remove(desc.removeServerTemplates[i]);
	}
	_descriptor.serverTemplates.putAll(desc.serverTemplates);
	_serverTemplates.update(desc.serverTemplates, 
				desc.removeServerTemplates,
				desc.serviceTemplates.keySet());
	
	//
	// Nodes
	//
	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    _descriptor.nodes.remove(desc.removeNodes[i]);
	}
	//
	// Updates also _descriptor.nodes
	//
	_nodes.update(desc.nodes, desc.removeNodes,
		      desc.serverTemplates.keySet(),
		      desc.serviceTemplates.keySet());

    }

    ServerTemplate findServerTemplate(String id)
    {
	return (ServerTemplate)_serverTemplates.findChild(id);
    }

    ServiceTemplate findServiceTemplate(String id)
    {
	return (ServiceTemplate)_serviceTemplates.findChild(id);
    }

    ReplicaGroup findReplicaGroup(String id)
    {
	return (ReplicaGroup)_replicaGroups.findChild(id);
    }

    Node findNode(String id)
    {
	return (Node)_nodes.findChild(id);
    }

    java.util.List findServerInstances(String template)
    {
	return _nodes.findServerInstances(template);
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = _serverTemplates.findServiceInstances(template);
	result.addAll(_nodes.findServiceInstances(template));
	return result;
    }


    TemplateDescriptor findServerTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serverTemplates.get(templateName);
    }

    TemplateDescriptor findServiceTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serviceTemplates.get(templateName);
    }
    
    ServerTemplates getServerTemplates()
    {
	return _serverTemplates;
    }
    
    ServiceTemplates getServiceTemplates()
    {
	return _serviceTemplates;
    }

    ReplicaGroups getReplicaGroups()
    {
	return _replicaGroups;
    }

    //
    // Should only be used for reading
    //
    java.util.Map getVariables()
    {
	return _descriptor.variables;
    }

    void nodeUp(String nodeName, NodeInfo staticInfo)
    {
	_nodes.nodeUp(nodeName, staticInfo);
    }

    void nodeDown(String nodeName)
    {
	_nodes.nodeDown(nodeName);
    }
    
    Utils.Resolver getResolver()
    {
	return _resolver;
    }
    
    private ApplicationDescriptor _descriptor;
    private final boolean _ephemeral;
    private Utils.Resolver _resolver;

    //
    // Keeps original version (as shallow copies) to be able to build 
    // ApplicationUpdateDescriptor
    //
    private java.util.TreeMap _origVariables;
    private String _origDescription;
    private DistributionDescriptor _origDistrib;
   
    //
    // Children
    //
    private ReplicaGroups _replicaGroups;
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private Nodes _nodes;

    static private DefaultTreeCellRenderer _cellRenderer;    
    static private ApplicationEditor _editor;
    static private JPopupMenu _popup;
}
