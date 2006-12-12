// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;

import java.util.Enumeration;
import java.util.prefs.Preferences;

import IceGrid.*;
import IceGridGUI.*;

//
// The Root node of the Live Deployment view
//
public class Root extends ListArrayTreeNode
{
    public Root(Coordinator coordinator) 
    {
	super(null, "Root", 2);
	_coordinator = coordinator;
	_childrenArray[0] = _slaves;
	_childrenArray[1] = _nodes;
	_messageSizeMax = Ice.Util.getInstance(_coordinator.getCommunicator()).messageSizeMax();

	_tree = new JTree(this, true);
	_treeModel = (DefaultTreeModel)_tree.getModel();
	_objectDialog = new ObjectDialog(this);
       
	loadLogPrefs();
    }

    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[ADD_OBJECT] = _coordinator.connectedToMaster();
	actions[SHUTDOWN_REGISTRY] = true;
	actions[RETRIEVE_STDOUT] = true;
	actions[RETRIEVE_STDERR] = true;
	return actions;
    }

    public void shutdownRegistry()
    {
	final String prefix = "Shutting down registry '" + _replicaName + "'...";
	getCoordinator().getStatusBar().setText(prefix);

	AMI_Admin_shutdownRegistry cb = new AMI_Admin_shutdownRegistry()
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
		    amiFailure(prefix, "Failed to shutdown " + _replicaName, e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to shutdown " + _replicaName, 
			       e.toString());
		}
	    };

	try
	{   
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    
	    _coordinator.getAdmin().shutdownRegistry_async(cb, _replicaName);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to shutdown " + _replicaName, e.toString());
	}
	finally
	{
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }


    public ApplicationDescriptor getApplicationDescriptor(String name)
    {
	return (ApplicationDescriptor)_descriptorMap.get(name);
    }

    public Object[] getApplicationNames()
    {
	return _descriptorMap.keySet().toArray();
    }

    public Object[] getPatchableApplicationNames()
    {
	java.util.List result = new java.util.ArrayList();

	java.util.Iterator p = _descriptorMap.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();

	    ApplicationDescriptor app = (ApplicationDescriptor)entry.getValue();
	    if(app.distrib.icepatch.length() > 0)
	    {
		result.add(entry.getKey());
	    }
	}
	return result.toArray();
    }

    
    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new RegistryEditor();
	}
	_editor.show(this);
	return _editor;
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
	 
	    Icon icon = Utils.getIcon("/icons/16x16/registry.png");
	    _cellRenderer.setOpenIcon(icon);
	    _cellRenderer.setClosedIcon(icon);
	}
	
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    public void applicationInit(String instanceName, String replicaName, java.util.List applications)
    {
	closeAllShowLogDialogs();

	_replicaName = replicaName;
	_label = instanceName + " (" + _replicaName + ")";
	_tree.setRootVisible(true);	
	
	java.util.Iterator p = applications.iterator();
	while(p.hasNext())
	{
	    applicationAdded((ApplicationDescriptor)p.next());
	}
    }
    
    //
    // Called when the session to the IceGrid Registry is closed
    //
    public void clear()
    {
	_adapters.clear();
	_objects.clear();
	_replicaName = null;

	_descriptorMap.clear();
	_nodes.clear();
	_slaves.clear();
	_treeModel.nodeStructureChanged(this);
	_tree.setRootVisible(false);
    }

    public void patch(final String applicationName)
    {
	int shutdown = JOptionPane.showConfirmDialog(
	    _coordinator.getMainFrame(),
	    "You are about to install or refresh your" 
	    + " application distribution.\n"
	    + " Do you want shut down all servers affected by this update?", 
	    "Patch Confirmation",
	    JOptionPane.YES_NO_CANCEL_OPTION);
       
	if(shutdown == JOptionPane.CANCEL_OPTION)
	{
	    return;
	}

	final String prefix = "Patching application '" + applicationName + "'...";

	_coordinator.getStatusBar().setText(prefix);
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
		    amiFailure(prefix, "Failed to patch '" 
			       + applicationName + "'", e);
		}

		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to patch '" +
			       applicationName + "'", e.toString());
		}
	    };
	
	try
	{   
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	   _coordinator.getAdmin().
	       patchApplication_async(cb, applicationName, 
				      shutdown == JOptionPane.YES_OPTION);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to patch " + _id, e.toString());
	}
	finally
	{
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }


    //
    // From the Registry Observer:
    //
    public void applicationAdded(ApplicationDescriptor desc)
    {
	_descriptorMap.put(desc.name, desc);
	
	java.util.Iterator p = desc.nodes.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String nodeName = (String)entry.getKey();
	    NodeDescriptor nodeDesc = (NodeDescriptor)entry.getValue();

	    Node node = findNode(nodeName);
	    if(node == null)
	    {
		insertNode(new Node(this, desc, nodeName, nodeDesc));
	    }
	    else
	    {
		node.add(desc, nodeDesc);
	    }
	}
    }

    public void applicationRemoved(String name)
    {
	_descriptorMap.remove(name);

	java.util.List toRemove = new java.util.LinkedList();
	int[] toRemoveIndices = new int[_nodes.size()];

	int i = 0;
	for(int index = 0; index < _nodes.size(); ++index)
	{
	    Node node = (Node)_nodes.get(index);
	    if(node.remove(name))
	    {
		toRemove.add(node);
		toRemoveIndices[i++] = _slaves.size() + index;
	    }
	}

	removeNodes(resize(toRemoveIndices, toRemove.size()), toRemove);
    }
    
    public void applicationUpdated(ApplicationUpdateDescriptor update)
    {
	ApplicationDescriptor appDesc = (ApplicationDescriptor)_descriptorMap.get(update.name);

	//
	// Update various fields of appDesc
	//
	if(update.description != null)
	{
	    appDesc.description = update.description.value;
	}
	if(update.distrib != null)
	{
	    appDesc.distrib = update.distrib.value;
	}

	appDesc.variables.keySet().removeAll(java.util.Arrays.asList(update.removeVariables));
	appDesc.variables.putAll(update.variables);
	boolean variablesChanged = update.removeVariables.length > 0 || !update.variables.isEmpty();

	//
	// Update only descriptors (no tree node shown in this view)
	//
	appDesc.propertySets.keySet().
	    removeAll(java.util.Arrays.asList(update.removePropertySets));
	appDesc.propertySets.putAll(update.propertySets);

	for(int i = 0; i < update.removeReplicaGroups.length; ++i)
	{
	    for(int j = 0; j < appDesc.replicaGroups.size(); ++j)
	    {
		ReplicaGroupDescriptor rgd = (ReplicaGroupDescriptor)appDesc.replicaGroups.get(j);
		if(rgd.id.equals(update.removeReplicaGroups[i]))
		{
		    appDesc.replicaGroups.remove(j);
		    break; // for
		}
	    }
	}

	for(int i = 0; i < update.replicaGroups.size(); ++i)
	{
	    ReplicaGroupDescriptor newRgd = (ReplicaGroupDescriptor)update.replicaGroups.get(i);

	    boolean replaced = false;
	    int j = 0;
	    while(j < appDesc.replicaGroups.size() && !replaced)
	    {
		ReplicaGroupDescriptor oldRgd = (ReplicaGroupDescriptor)appDesc.replicaGroups.get(j);
		
		if(newRgd.id.equals(oldRgd.id))
		{
		    appDesc.replicaGroups.set(j, newRgd);
		    replaced = true;
		}
		j++;
	    }

	    if(!replaced)
	    {
		appDesc.replicaGroups.add(newRgd);
	    }
	}
   
	appDesc.serviceTemplates.keySet().
	    removeAll(java.util.Arrays.asList(update.removeServiceTemplates));
	appDesc.serviceTemplates.putAll(update.serviceTemplates);
	
	appDesc.serverTemplates.keySet().
	    removeAll(java.util.Arrays.asList(update.removeServerTemplates));
	appDesc.serverTemplates.putAll(update.serverTemplates);

	//
	// Nodes
	//

	//
	// Removal 
	//
	appDesc.nodes.keySet().removeAll(java.util.Arrays.asList(update.removeNodes));

	for(int i = 0; i < update.removeNodes.length; ++i)
	{
	    Node node = findNode(update.removeNodes[i]);
	    if(node.remove(update.name))
	    {
		int index = getIndex(node); 
		_nodes.remove(node);
		_treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{node});
	    }
	}
	
	//
	// Add/update
	//
	java.util.Iterator p = update.nodes.iterator();
	java.util.Set freshNodes = new java.util.HashSet();
	while(p.hasNext())
	{
	    NodeUpdateDescriptor nodeUpdateDesc = (NodeUpdateDescriptor)p.next();
	    
	    String nodeName = nodeUpdateDesc.name;

	    Node node = findNode(nodeName);
	    if(node == null)
	    {
		node = new Node(this, appDesc, nodeUpdateDesc);
		insertNode(node);
	    }
	    else
	    {
		node.update(appDesc, nodeUpdateDesc, variablesChanged, 
			    update.serviceTemplates.keySet(), update.serverTemplates.keySet());
	    }
	    freshNodes.add(node);
	}
	
	//
	// Notify non-fresh nodes if needed
	//
	if(variablesChanged || !update.serviceTemplates.isEmpty() || !update.serverTemplates.isEmpty())
	{
	    p = _nodes.iterator();
	    while(p.hasNext())
	    {
		Node node = (Node)p.next();
		if(!freshNodes.contains(node))
		{
		    node.update(appDesc, null, variablesChanged,
				update.serviceTemplates.keySet(), update.serverTemplates.keySet());
		}
	    }
	}
    }

    public void adapterInit(AdapterInfo[] adapters)
    {
	for(int i = 0; i < adapters.length; ++i)
	{
	    _adapters.put(adapters[i].id, adapters[i]);
	}	
    }

    public void adapterAdded(AdapterInfo info)
    {
	_adapters.put(info.id, info);
    }    

    public void adapterUpdated(AdapterInfo info)
    {
	_adapters.put(info.id, info);
    }    

    public void adapterRemoved(String id)
    {
	_adapters.remove(id);
    }    
    
    public void objectInit(ObjectInfo[] objects)
    {
	for(int i = 0; i < objects.length; ++i)
	{
	    _objects.put(Ice.Util.identityToString(objects[i].proxy.ice_getIdentity()), objects[i]);
	}
    }

    public void objectAdded(ObjectInfo info)
    {
	_objects.put(Ice.Util.identityToString(info.proxy.ice_getIdentity()), info);
    }    

    public void objectUpdated(ObjectInfo info)
    {
	_objects.put(Ice.Util.identityToString(info.proxy.ice_getIdentity()), info);
    }    

    public void objectRemoved(Ice.Identity id)
    {
	_objects.remove(Ice.Util.identityToString(id));
    } 


    //
    // From the Registry Observer:
    //
    public void registryUp(RegistryInfo info)
    {
	if(info.name.equals(_replicaName))
	{
	    _info = info;
	}
	else
	{
	    Slave newSlave = new Slave(this, info);

	    int i;
	    for(i = 0; i < _slaves.size(); ++i)
	    {
		String otherName = _slaves.get(i).toString();
		if(info.name.compareTo(otherName) > 0)
		{
		    i++;
		    break;
		}
	    }
	    _slaves.add(i, newSlave);
	    _treeModel.nodesWereInserted(this, new int[]{i});
	}
    }

    public void registryDown(String name)
    {
	TreeNodeBase registry = find(name, _slaves);
	if(registry != null)
	{
	    int index = getIndex(registry);
	    _slaves.remove(registry);
	    _treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{registry});
	}
    }

    //
    // From the Node Observer:
    //
    public void nodeUp(NodeDynamicInfo updatedInfo)
    {
	Node node = findNode(updatedInfo.info.name);
	if(node != null)
	{
	    node.up(updatedInfo, true);
	}
	else
	{
	    insertNode(new Node(this, updatedInfo));
	}
    }

    public void nodeDown(String nodeName)
    {
	Node node = findNode(nodeName);
	if(node != null)
	{
	    if(node.down())
	    {
		int index = getIndex(node); 
		_nodes.remove(node);
		_treeModel.nodesWereRemoved(this, new int[]{index}, new Object[]{node});
	    }
	}
    }

    public void updateServer(String nodeName, ServerDynamicInfo updatedInfo)
    {
	Node node = findNode(nodeName);
	if(node != null)
	{
	    node.updateServer(updatedInfo);
	}
    }

    public void updateAdapter(String nodeName, AdapterDynamicInfo updatedInfo)
    {
	Node node = findNode(nodeName);
	if(node != null)
	{
	    node.updateAdapter(updatedInfo);
	}
    }

    public JPopupMenu getPopupMenu()
    {
	LiveActions la = _coordinator.getLiveActionsForPopup();

	if(_popup == null)
	{
	    _popup = new JPopupMenu();
	    _popup.add(la.get(ADD_OBJECT));
	    _popup.addSeparator();
	    _popup.add(la.get(RETRIEVE_STDOUT));
	    _popup.add(la.get(RETRIEVE_STDERR));
	    _popup.addSeparator();
	    _popup.add(la.get(SHUTDOWN_REGISTRY));
	}
	
	la.setTarget(this);
	return _popup;
    }

    public void setSelectedNode(TreeNode node)
    {
	_tree.setSelectionPath(node.getPath());
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

    public String toString()
    {
	return _label;
    }
    
    //
    // Check that this node is attached to the tree
    //
    public boolean hasNode(TreeNode node)
    {
	while(node != this)
	{
	    TreeNode parent = (TreeNode)node.getParent();
	    if(parent.getIndex(node) == -1)
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

    public void addObject()
    {
	_objectDialog.showDialog();
    }


    Root getRoot()
    {
	return this;
    }

    java.util.SortedMap getObjects()
    {
	return _objects;
    }

    java.util.SortedMap getAdapters()
    {
	return _adapters;
    }

    RegistryInfo getRegistryInfo()
    {
	return _info;
    }

    boolean addObject(String strProxy, String type)
    {
	Ice.ObjectPrx proxy = null;

	try
	{
	    proxy = _coordinator.getCommunicator().stringToProxy(strProxy);
	}
	catch(Ice.LocalException e)
	{
	    JOptionPane.showMessageDialog(
		_coordinator.getMainFrame(),
		"Cannot parse proxy '" + strProxy + "'",
		"addObject failed",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}

	if(proxy == null)
	{
	    JOptionPane.showMessageDialog(
		_coordinator.getMainFrame(),
		"You must provide a non-null proxy",
		"addObject failed",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}

	String strIdentity = Ice.Util.identityToString(proxy.ice_getIdentity());

	String prefix = "Adding well-known object '" + strIdentity + "'...";
	try
	{   
	    _coordinator.getStatusBar().setText(prefix);
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

	    if(type == null)
	    {
		_coordinator.getAdmin().addObject(proxy);
	    }
	    else
	    {
		_coordinator.getAdmin().addObjectWithType(proxy, type);
	    }
	}
	catch(ObjectExistsException e)
	{
	    _coordinator.getStatusBar().setText(prefix + "failed.");
	    JOptionPane.showMessageDialog(
		_coordinator.getMainFrame(),
		"An object with this identity is already registered as a well-known object",
		"addObject failed",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}
	catch(DeploymentException e)
	{
	    _coordinator.getStatusBar().setText(prefix + "failed.");
	    JOptionPane.showMessageDialog(
		_coordinator.getMainFrame(),
		"Deployment exception: " + e.reason,
		"addObject failed",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}
	catch(Ice.LocalException e)
	{
	    _coordinator.getStatusBar().setText(prefix + "failed.");
	    JOptionPane.showMessageDialog(
		_coordinator.getMainFrame(),
		e.toString(),
		"addObject failed",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}
	finally
	{
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
	_coordinator.getStatusBar().setText(prefix + "done.");
	return true;
    }


    void removeObject(String strProxy)
    {
	Ice.ObjectPrx proxy = _coordinator.getCommunicator().stringToProxy(strProxy);
	Ice.Identity identity = proxy.ice_getIdentity();
	final String strIdentity = Ice.Util.identityToString(identity);

	final String prefix = "Removing well-known object '" + strIdentity + "'...";
	_coordinator.getStatusBar().setText(prefix);
	
	AMI_Admin_removeObject cb = new AMI_Admin_removeObject()
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
		    amiFailure(prefix, "Failed to remove object '" + strIdentity + "'", e);
		}
		
		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to remove object '" + strIdentity + "'",
			       e.toString());
		}
	    };
	
	try
	{   
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    _coordinator.getAdmin().removeObject_async(cb, identity);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to remove object '" + strIdentity + "'",
		    e.toString());
	}
	finally
	{
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }

    void removeAdapter(final String adapterId)
    {
	final String prefix = "Removing adapter '" + adapterId + "'...";
	_coordinator.getStatusBar().setText(prefix);
	
	AMI_Admin_removeAdapter cb = new AMI_Admin_removeAdapter()
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
		    amiFailure(prefix, "Failed to remove adapter '" + adapterId + "'", e);
		}
		
		public void ice_exception(Ice.LocalException e)
		{
		    amiFailure(prefix, "Failed to remove adapter '" + adapterId + "'", 
			       e.toString());
		}
	    };
	
	try
	{   
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
	    _coordinator.getAdmin().removeAdapter_async(cb, adapterId);
	}
	catch(Ice.LocalException e)
	{
	    failure(prefix, "Failed to remove adapter '" + adapterId + "'", e.toString());
	}
	finally
	{
	    _coordinator.getMainFrame().setCursor(
		Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
	}
    }
    
    public void retrieveOutput(final boolean stdout)
    {
	getRoot().openShowLogDialog(new ShowLogDialog.FileIteratorFactory()
	    {
		public FileIteratorPrx open(int count)
		    throws FileNotAvailableException, RegistryNotExistException, RegistryUnreachableException
		{
		    AdminSessionPrx session = _coordinator.getSession();

		    if(stdout)
		    {
			return session.openRegistryStdOut(_replicaName, count);
		    }
		    else
		    {
			return session.openRegistryStdErr(_replicaName, count);
		    }
		}

		public String getTitle()
		{
		    return "Registry " + _label + " " + (stdout ? "Stdout" : "Stderr");
		}
		
		public String getDefaultFilename()
		{
		    return _replicaName + (stdout ? ".out" : ".err");
		}

	    });
    } 


    PropertySetDescriptor findNamedPropertySet(String name, String applicationName)
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)
	    _descriptorMap.get(applicationName);
	return (PropertySetDescriptor)descriptor.propertySets.get(name);
    }


    void openShowLogDialog(ShowLogDialog.FileIteratorFactory factory)
    {
	ShowLogDialog d = (ShowLogDialog)_showLogDialogMap.get(factory.getTitle());
	if(d == null)
	{
	    d = new ShowLogDialog(this, factory, 
				  _logMaxLines, _logMaxSize, _logInitialLines, _logMaxReadSize, _logPeriod);
	    
	    _showLogDialogMap.put(factory.getTitle(), d);
	}
	else
	{
	    d.toFront();
	}
    }

    void removeShowLogDialog(String title)
    {
	_showLogDialogMap.remove(title);
    }

    public void closeAllShowLogDialogs()
    {
	java.util.Iterator p = _showLogDialogMap.values().iterator();
	while(p.hasNext())
	{
	    ShowLogDialog d = (ShowLogDialog)p.next();
	    d.close(false);
	}
	_showLogDialogMap.clear();
    }

    public int getMessageSizeMax()
    {
	return _messageSizeMax;
    }


    public void setLogPrefs(int maxLines, int maxSize, int initialLines, int maxReadSize, int period)
    {
	_logMaxLines = maxLines;
	_logMaxSize = maxSize;
	_logInitialLines = initialLines;
	_logMaxReadSize = maxReadSize;
	_logPeriod = period;

	storeLogPrefs();
    }

    
    private void loadLogPrefs()
    {
	Preferences logPrefs = _coordinator.getPrefs().node("Log");
	_logMaxLines = logPrefs.getInt("maxLines", 500);
	_logMaxSize = logPrefs.getInt("maxSize", 20000);
	_logInitialLines = logPrefs.getInt("initialLines", 10);
	_logMaxReadSize = logPrefs.getInt("maxReadSize", 10000);
	_logPeriod = logPrefs.getInt("period", 300);

	if(_logMaxReadSize + 512 > _messageSizeMax)
	{
	    _logMaxReadSize = _messageSizeMax - 512;
	}
    }

    private void storeLogPrefs()
    {
	Preferences logPrefs = _coordinator.getPrefs().node("Log");
	logPrefs.putInt("maxLines", _logMaxLines);
	logPrefs.putInt("maxSize", _logMaxSize);
	logPrefs.putInt("initialLines", _logInitialLines);
	logPrefs.putInt("maxReadSize", _logMaxReadSize);
	logPrefs.putInt("period", _logPeriod);
    }


    private Node findNode(String nodeName)
    {
	return (Node)find(nodeName, _nodes);
    }
    
    private void insertNode(Node node)
    {
	String nodeName = node.toString();
	int i;
	for(i = 0; i < _nodes.size(); ++i)
	{
	    String otherNodeName = _nodes.get(i).toString();
	    if(nodeName.compareTo(otherNodeName) > 0)
	    {
		i++;
		break;
	    }
	}
	_nodes.add(i, node);
	_treeModel.nodesWereInserted(this, new int[]{_slaves.size() + i});
    }

    private void removeNodes(int[] toRemoveIndices, java.util.List toRemove)
    {
	if(toRemove.size() > 0)
	{
	    _nodes.removeAll(toRemove);
	    _treeModel.nodesWereRemoved(this, toRemoveIndices, toRemove.toArray());
	}
    }
  
    private final Coordinator _coordinator;
    private String _replicaName;

    private final java.util.List _nodes = new java.util.LinkedList();
    private final java.util.List _slaves = new java.util.LinkedList();

    //
    // Maps application name to current application descriptor
    //
    private final java.util.Map _descriptorMap = new java.util.TreeMap();

    //
    // Map AdapterId => AdapterInfo
    //
    private java.util.SortedMap _adapters = new java.util.TreeMap();

    //
    // Map stringified identity => ObjectInfo
    //
    private java.util.SortedMap _objects = new java.util.TreeMap();

    //
    // 'this' is the root of the tree
    //
    private final JTree _tree;
    private final DefaultTreeModel _treeModel;

    private RegistryInfo _info;
    
    private String _label;

    private ObjectDialog _objectDialog;

    //
    // ShowLogDialog
    //
    final int _messageSizeMax;

    java.util.Map _showLogDialogMap = new java.util.HashMap();
   
    int _logMaxLines;
    int _logMaxSize;
    int _logInitialLines;
    int _logMaxReadSize;
    int _logPeriod;
  
    static private RegistryEditor _editor;
    static private JPopupMenu _popup;
    static private DefaultTreeCellRenderer _cellRenderer;
}
