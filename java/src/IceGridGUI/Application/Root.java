// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;

import java.io.File;

import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

public class Root extends ListTreeNode
{
    //
    // Construct a normal, existing Application
    //
    public Root(Coordinator coordinator, ApplicationDescriptor desc, 
		boolean live, File file)
    {
	super(false, null, desc.name);
	_ephemeral = false;
	_coordinator = coordinator;
	_descriptor = desc;

	_file = file;
	_live = live;

	try
	{
	    init();
	}
	catch(UpdateFailedException e)
	{
	    //
	    // Impossible
	    //
	    assert false;
	}
    }
   
    //
    // Construct a new, ephemeral Application
    //
    public Root(Coordinator coordinator, ApplicationDescriptor desc)
    {
	super(false, null, desc.name);
	_ephemeral = true;
	_coordinator = coordinator;
	_descriptor = desc;

	_tree = new JTree(this, true);
	_treeModel = (DefaultTreeModel)_tree.getModel();
    }

    //
    // Contruct a brand-new Application from an ephemeral Application
    //
    Root(Root root) throws UpdateFailedException
    {
	super(true, null, root._descriptor.name);
	_ephemeral = false;
	_coordinator = root._coordinator;
	_descriptor = root._descriptor;

	assert root._ephemeral;	
	init();
    }

    private void init() throws UpdateFailedException
    {
	_resolver = new Utils.Resolver(_descriptor.variables);
	_resolver.put("application", _descriptor.name);

	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origDistrib = (DistributionDescriptor)_descriptor.distrib.clone();

	_replicaGroups = new ReplicaGroups(this, _descriptor.replicaGroups);       
	_serviceTemplates = new ServiceTemplates(this, _descriptor.serviceTemplates);
	_serverTemplates = new ServerTemplates(this, _descriptor.serverTemplates);
	_nodes = new Nodes(this, _descriptor.nodes);

	_children.add(_nodes);
	_children.add(_replicaGroups);
	_children.add(_serverTemplates);
	_children.add(_serviceTemplates);

	_tree = new JTree(this, true);
	_treeModel = (DefaultTreeModel)_tree.getModel();
    }


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

    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = (ApplicationEditor) 
		getEditor(ApplicationEditor.class, this);
	}
	_editor.show(this);
	return _editor;
    }

    protected Editor createEditor()
    {
	return new ApplicationEditor(_coordinator.getMainFrame());
    }

    public TreeNode findNodeLike(TreePath path, boolean exactMatch)
    {
	TreeNode result = null;
	if(path == null)
	{
	    return null;
	}

	for(int i = 0; i < path.getPathCount(); ++i)
	{
	    TreeNode node = (TreeNode)path.getPathComponent(i);
	    
	    if(result == null)
	    {
		if(node.getId().equals(_id))
		{
		    result = this;
		}
		else
		{
		    return null;
		}
	    }
	    else
	    {
		TreeNode newNode = result.findChildLike(node);
		if(newNode == null)
		{
		    if(exactMatch)
		    {
			return null;
		    }
		    else
		    {
			return result;
		    }
		}
		else
		{
		    result = newNode;
		}
	    }
	}

	return result;
    }
    
    //
    // Check that this node is attached to the tree
    //
    public boolean hasNode(TreeNode node)
    {
	while(node != this)
	{
	    TreeNode parent = (TreeNode)node.getParent();
	    if(parent == null || parent.getIndex(node) == -1)
	    {
		return false;
	    }
	    else
	    {
		node = parent;
	    }
	}
	return true;
    }

    public void setSelectedNode(TreeNode node)
    {
	_tree.setSelectionPath(node.getPath());
    }

    public TreeNode getSelectedNode()
    {
	TreePath path = _tree.getSelectionPath();
	if(path == null)
	{
	    return null;
	}
	else
	{
	    return (TreeNode)path.getLastPathComponent();
	}
    }

    public void selectServer(String nodeName, String serverId)
    {
	TreeNode target = _nodes;
	if(target != null)
	{
	    Node node = (Node)target.findChild(nodeName);
	    if(node != null)
	    {
		target = node;
		Server server = (Server)target.findChild(serverId);
		if(server != null)
		{
		    target = server;
		}
	    }
	    setSelectedNode(target);
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

	Object descriptor =  _coordinator.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = descriptor instanceof ApplicationDescriptor;
	}
	if(!_ephemeral)
	{
	    actions[SHOW_VARS] = true;
	    actions[SUBSTITUTE_VARS] = true;	
	    actions[NEW_NODE] = true;
	    actions[NEW_REPLICA_GROUP] = true;
	    actions[NEW_TEMPLATE_SERVER] = true;
	    actions[NEW_TEMPLATE_SERVER_ICEBOX] = true;
	    actions[NEW_TEMPLATE_SERVICE] = true;
	}
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	ApplicationActions actions = _coordinator.getActionsForPopup();
	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(actions.get(NEW_NODE));
	    _popup.add(actions.get(NEW_REPLICA_GROUP));
	    _popup.addSeparator();
	    _popup.add(actions.get(NEW_TEMPLATE_SERVER));
	    _popup.add(actions.get(NEW_TEMPLATE_SERVER_ICEBOX));
	    _popup.addSeparator();
	    _popup.add(actions.get(NEW_TEMPLATE_SERVICE));
	}
	actions.setTarget(this);
	return _popup;
    }

    public void copy()
    {
	_coordinator.setClipboard(copyDescriptor(_descriptor));
	_coordinator.getActionsForMenu().get(PASTE).setEnabled(true);
    }
    public void paste()
    {
	_coordinator.pasteApplication();
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
    
    public void save()
    {
	if(_live)
	{
	    saveToRegistry();
	}
	else if(_file != null)
	{
	    File file = _coordinator.saveToFile(false, this, _file);
	    if(file != null)
	    {
		_file = file;
		commit();
		_coordinator.getSaveAction().setEnabled(false);
		_coordinator.getDiscardUpdatesAction().setEnabled(false);
	    }
	}
	else
	{
	    assert false;
	}
    }

    public void saveToRegistry()
    {
	//
	// To be run in GUI thread when exclusive write access is acquired
	//
	Runnable runnable = new Runnable()
	    {
		public void run()
		{
		    _coordinator.getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
		    try
		    {
			if(_live && _canUseUpdateDescriptor)
			{
			    ApplicationUpdateDescriptor updateDescriptor = createUpdateDescriptor();
			    if(updateDescriptor != null)
			    {
				_coordinator.getSession().updateApplication(updateDescriptor);
				commit();
				_skipUpdates++;
			    }
			}
			else
			{
			    //
			    // Add or sync application
			    //
			    if(_coordinator.getLiveDeploymentRoot().getApplicationDescriptor(_id) == null)
			    {
				assert _live == false;
				_coordinator.getSession().addApplication(_descriptor);
				commit();
				liveReset();
				_coordinator.addLiveApplication(Root.this);
			    }
			    else
			    {
				_coordinator.getSession().syncApplication(_descriptor);
				commit();

				if(_live)
				{
				    liveReset();
				    _skipUpdates++;
				}
				else
				{
				    //
				    // Make this tab live or close it if there is one already open
				    //
				    ApplicationPane app = _coordinator.getLiveApplication(_id);
				    if(app == null)
				    {
					liveReset();
					_coordinator.addLiveApplication(Root.this);
				    }
				    else
				    {
					boolean selected = isSelected();
					_coordinator.getMainPane().removeApplication(Root.this);
					if(selected)
					{
					    _coordinator.getMainPane().setSelectedComponent(app);
					}
				    }
				}
			    }
			}
			if(isSelected())
			{
			    _coordinator.getSaveAction().setEnabled(false);
			    _coordinator.getDiscardUpdatesAction().setEnabled(false);
			}
		    }
		    catch(DeploymentException e)
		    {
			JOptionPane.showMessageDialog(
			    _coordinator.getMainFrame(),
			    "Application '" + _id + "': "+ e.reason,
			    "Deployment Exception",
			    JOptionPane.ERROR_MESSAGE);
		    }
		    catch(ApplicationNotExistException e)
		    {
			//
			// Should never happen
			//
			JOptionPane.showMessageDialog(
			    _coordinator.getMainFrame(),
			    "Application '" + _id + "' was not found in the IceGrid registry",
			    "Deployment Exception",
			    JOptionPane.ERROR_MESSAGE);
		    }
		    catch(AccessDeniedException e)
		    {
			//
			// Should never happen
			//
			_coordinator.accessDenied(e);
		    }
		    finally
		    {
			_coordinator.getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
			_coordinator.releaseExclusiveWriteAccess();
		    }
		}
	    };
	
	try
	{
	    _coordinator.acquireExclusiveWriteAccess(runnable);
	}
	catch(AccessDeniedException e)
	{
	    _coordinator.accessDenied(e);
	}
    }

    public void saveToFile()
    {
	File file = _coordinator.saveToFile(true, this, _file);
	if(file != null)
	{
	    _file = file;
	    _live = false;

	    _coordinator.removeLiveApplication(_id);
	    _coordinator.getMainPane().resetIcon(this);

	    commit();
	    _coordinator.getSaveAction().setEnabled(false);
	    _coordinator.getDiscardUpdatesAction().setEnabled(false);
	}
    }

    public void discardUpdates()
    {
	if(_live)
	{
	   ApplicationDescriptor desc = _coordinator.getLiveDeploymentRoot().getApplicationDescriptor(_id);
	   assert desc != null;
	   desc = IceGridGUI.Application.Root.copyDescriptor(desc);

	   Root newRoot = new Root(_coordinator, desc, true, null);
	   ApplicationPane app = _coordinator.getMainPane().findApplication(this);
	   assert app != null;
	   app.setRoot(newRoot);
	   
	   TreeNode node = newRoot.findNodeLike(_tree.getSelectionPath(), false);
	   if(node == null)
	   {
	       newRoot.setSelectedNode(newRoot);
	   }
	   else
	   {
	       newRoot.setSelectedNode(node);
	   }
	}
	else if(_file != null)
	{
	    //
	    // TODO: re-read file
	    //
	}
	else
	{
	    assert false;
	}
	_coordinator.getCurrentTab().selected();
    }

    private void liveReset()
    {
	_live = true;
	_file = null;
	_coordinator.getMainPane().resetIcon(this);
    }


    private ApplicationUpdateDescriptor createUpdateDescriptor()
    {
	ApplicationUpdateDescriptor update = new ApplicationUpdateDescriptor();
	update.name = _descriptor.name;
	if(_editable.isModified())
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
	// Replica Groups
	//
	update.removeReplicaGroups = _replicaGroups.getEditable().removedElements();
	update.replicaGroups = _replicaGroups.getUpdates();
	
	//
	// Server Templates
	//
	update.removeServerTemplates = _serverTemplates.getEditable().removedElements();
	update.serverTemplates = _serverTemplates.getUpdates();

	//
	// Service Templates
	//
	update.removeServiceTemplates = _serviceTemplates.getEditable().removedElements();
	update.serviceTemplates =_serviceTemplates.getUpdates();

	//
	// Nodes
	//
	update.removeNodes = _nodes.getEditable().removedElements();
	update.nodes = _nodes.getUpdates();


	//
	// Return null if nothing changed
	//
	if(!_editable.isModified() &&
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

    private void commit()
    {
	_updated = false;
	_registryUpdatesEnabled = true;
	_canUseUpdateDescriptor = true;

	assert _concurrentUpdates.size() == 0;

	_editable.commit();
	_origVariables = _descriptor.variables;
	_origDescription = _descriptor.description;
	_origDistrib = (DistributionDescriptor)_descriptor.distrib.clone();
	
	_nodes.commit();
	_replicaGroups.commit();
	_serverTemplates.commit();
	_serviceTemplates.commit();
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    public void destroy()
    {
	if(_ephemeral || (!_live && _file == null))
	{
	    _coordinator.getMainPane().removeApplication(this);
	}
	else if(_live)
	{
	    int confirm = JOptionPane.showConfirmDialog(
		_coordinator.getMainFrame(),
		"You are about to remove application '" + _id + "' from the IceGrid registry. "
		+ "Do you want to proceed?", 
		"Remove Confirmation",
		JOptionPane.YES_NO_OPTION);

	    if(confirm == JOptionPane.YES_OPTION)
	    {
		if(_coordinator.removeApplicationFromRegistry(_id))
		{
		    _coordinator.getMainPane().removeApplication(this);
		}
	    }
	}
	else
	{
	    assert _file != null;

	    int confirm = JOptionPane.showConfirmDialog(
		_coordinator.getMainFrame(),
		"You are about to remove application '" + _id + "' and its associated file. "
		+ "Do you want to proceed?", 
		"Remove Confirmation",
		JOptionPane.YES_NO_OPTION);

	    if(confirm == JOptionPane.YES_OPTION)
	    {
		if(_file.delete())
		{
		    _coordinator.getMainPane().removeApplication(this);
		}
	    }
	}
    }

    public boolean update(ApplicationUpdateDescriptor desc)
    {
	assert _live == true;
	
	if(_skipUpdates > 0)
	{
	    _skipUpdates--;
	    return false;
	}

	if(!_registryUpdatesEnabled)
	{
	    if(_updated)
	    {
		_canUseUpdateDescriptor = false;
	    }
	    else
	    {
		_concurrentUpdates.add(desc);
	    }
	    return false;
	}

	try
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
	catch(UpdateFailedException e)
	{
	    //
	    // Received invalid update from IceGrid registry: a bug!
	    //
	    assert false;
	}

	return true;
    }

    public JTree getTree()
    {
	return _tree;
    }

    public DefaultTreeModel getTreeModel()
    {
	return _treeModel;
    }

    public Coordinator getCoordinator()
    {
	return _coordinator;
    }

    public boolean isSelectionListenerEnabled()
    {
	return _selectionListenerEnabled;
    }

    public void enableSelectionListener()
    {
	_selectionListenerEnabled = true;
    }

    public void disableSelectionListener()
    {
	_selectionListenerEnabled = false;
    }

    public void cancelEdit()
    {
	if(!_updated)
	{
	    _registryUpdatesEnabled = true;

	    //
	    // Apply now any delayed concurrent update
	    //
	    java.util.Iterator p = _concurrentUpdates.iterator();
	    while(p.hasNext())
	    {
		ApplicationUpdateDescriptor d = (ApplicationUpdateDescriptor)p.next();
		boolean ok = update(d);
		assert ok;
	    }
	    _concurrentUpdates.clear();
	}
    }

    public boolean kill()
    {
	_live = false;

	if(_registryUpdatesEnabled)
	{
	    return true;
	}
	else
	{
	    _coordinator.getMainPane().resetIcon(this);
	    return false;
	}
    }

    public boolean isLive()
    {
	return _live;
    }

    public boolean hasFile()
    {
	return _file != null;
    }

    public boolean needsSaving()
    {
	return _updated;
    }

    Editor getEditor(Class c, TreeNode node)
    {
	Editor result = (Editor)_editorMap.get(c);
	if(result == null)
	{
	    result = node.createEditor();
	    _editorMap.put(c, result);
	}
	return result;
    }

    Object getDescriptor()
    {
	return _descriptor;
    }

    ApplicationDescriptor saveDescriptor()
    {
	ApplicationDescriptor clone = (ApplicationDescriptor)_descriptor.clone();
	clone.distrib = (IceGrid.DistributionDescriptor)clone.distrib.clone();
	return clone;
    }

    void restoreDescriptor(ApplicationDescriptor clone)
    {
	_descriptor.name = clone.name;
	_descriptor.variables = clone.variables;
	_descriptor.distrib.icepatch = clone.distrib.icepatch;
	_descriptor.distrib.directories = clone.distrib.directories;
	_descriptor.description = clone.description;
    }

    public void write(XMLWriter writer) throws java.io.IOException
    {
	writer.writeStartTag("icegrid");

	java.util.List attributes = new java.util.LinkedList();
	attributes.add(createAttribute("name", _id));

	writer.writeStartTag("application", attributes);

	writeVariables(writer, _descriptor.variables);

	_serviceTemplates.write(writer);
	_serverTemplates.write(writer);
	_replicaGroups.write(writer);
	_nodes.write(writer);
	
	writer.writeEndTag("application");
	writer.writeEndTag("icegrid");
    }

    //
    // Methods called by editors; see also cancelEdit() above
    //
    void disableRegistryUpdates()
    {
	_registryUpdatesEnabled = false;
    }

    void updated()
    {
	_updated = true;
	_concurrentUpdates.clear();
	
	_coordinator.getSaveAction().setEnabled(_live || _file != null);
	_coordinator.getDiscardUpdatesAction().setEnabled(_live || _file != null);

	_coordinator.getSaveToRegistryAction().setEnabled(_coordinator.getSession() != null);
	_coordinator.getSaveToFileAction().setEnabled(true);
    }

    void rebuild() throws UpdateFailedException
    {
	Utils.Resolver oldResolver = _resolver;
	String oldId = _id;
	_id = _descriptor.name;
	_resolver = new Utils.Resolver(_descriptor.variables);
	_resolver.put("application", _id);

	try
	{
	    _nodes.rebuild();
	}
	catch(UpdateFailedException e)
	{
	    _id = oldId;
	    _resolver = oldResolver;
	    throw e;
	}
    }
    
    //
    // Called when a server-template is deleted, to remove all
    // corresponding instances.
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

    Root getRoot()
    {
	return this;
    }
    
    Utils.Resolver getResolver()
    {
	return _resolver;
    }

    //
    // Should only be used for reading
    //
    java.util.Map getVariables()
    {
	return _descriptor.variables;
    }

    private boolean isSelected()
    {
	if(_coordinator.getCurrentTab() instanceof ApplicationPane)
	{
	    return ((ApplicationPane)_coordinator.getCurrentTab()).getRoot() == Root.this;
	}
	else
	{
	    return false;
	}
    }

    private Coordinator _coordinator;

    //
    // 'this' is the root of the tree
    //
    private JTree _tree;
    private DefaultTreeModel _treeModel;
    private final boolean _ephemeral;
    private Utils.Resolver _resolver;

    private boolean _live;

    //
    // null when this application is not tied to a file 
    //
    private File _file;
    
    private ApplicationDescriptor _descriptor;

    //
    // Keeps original version (as shallow copies) to be able to build 
    // ApplicationUpdateDescriptor. Only used when _live == true
    //
    private java.util.TreeMap _origVariables;
    private String _origDescription;
    private DistributionDescriptor _origDistrib;

    //
    // When this application (and children) is being updated, we
    // no longer apply updates from the Registry. Only meaningful when
    // _live == true
    //
    private boolean _registryUpdatesEnabled = true;

    //
    // True when any update was applies to this application 
    // (including children)
    //
    private boolean _updated = false;
    
    //
    // Updates saved when _updated == false and 
    // _registryUpdatesEnabled == false
    //
    private java.util.List _concurrentUpdates = new java.util.LinkedList();

    //
    // When _live is true and _canUseUpdateDescriptor is true, we can
    // save the updates using an ApplicationUpdateDescriptor
    //
    private boolean _canUseUpdateDescriptor = true;
    //
    // Updates to skip (because already applied locally)
    //
    private int  _skipUpdates = 0;


    private Nodes _nodes;
    private ReplicaGroups _replicaGroups;
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;

    private boolean _selectionListenerEnabled = true;

    private ApplicationEditor _editor;

    //
    // Map editor-class to Editor object
    //
    private java.util.Map _editorMap = new java.util.HashMap();

    static private DefaultTreeCellRenderer _cellRenderer;    
    static private JPopupMenu _popup;
}
