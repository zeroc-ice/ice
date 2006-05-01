// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import javax.swing.JTree;
import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreePath;

import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

//
// The Root node of the Live Deployment view
//
public class Root extends ListTreeNode
{
    public Root(Coordinator coordinator) 
    {
	super(null, "Root");
	_coordinator = coordinator;

	_tree = new JTree(this, true);
	_treeModel = (DefaultTreeModel)_tree.getModel();
    }

    public ApplicationDescriptor getApplicationDescriptor(String name)
    {
	return (ApplicationDescriptor)_descriptorMap.get(name);
    }

    public Object[] getApplicationNames()
    {
	return _descriptorMap.keySet().toArray();
    }
    
    public Editor getEditor()
    {
	if(_editor == null)
	{
	    _editor = new RegistryEditor();
	}
	return _editor;
    }

    public void init(String instanceName, java.util.List applications)
    {
	_label = instanceName;
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
	_descriptorMap.clear();
	_children.clear();
	_treeModel.nodeStructureChanged(this);
	_tree.setRootVisible(false);
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
	int[] toRemoveIndices = new int[_children.size()];

	int i = 0;
	for(int index = 0; index < _children.size(); ++index)
	{
	    Node node = (Node)_children.get(index);
	    if(node.remove(name))
	    {
		toRemove.add(node);
		toRemoveIndices[i++] = index;
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
		}
	    }
	}
	appDesc.replicaGroups.addAll(update.replicaGroups);
	
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
		_children.remove(node);
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
	    p = _children.iterator();
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

    //
    // From the Node Observer:
    //
    public void nodeUp(NodeDynamicInfo updatedInfo)
    {
	Node node = findNode(updatedInfo.name);
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
		_children.remove(node);
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


    Root getRoot()
    {
	return this;
    }

    PropertySetDescriptor findNamedPropertySet(String name, String applicationName)
    {
	ApplicationDescriptor descriptor = (ApplicationDescriptor)
	    _descriptorMap.get(applicationName);
	return (PropertySetDescriptor)descriptor.propertySets.get(name);
    }

    private Node findNode(String nodeName)
    {
	return (Node)find(nodeName, _children);
    }
    
    private void insertNode(Node node)
    {
	String nodeName = node.toString();
	int i;
	for(i = 0; i < _children.size(); ++i)
	{
	    String otherNodeName = _children.get(i).toString();
	    if(nodeName.compareTo(otherNodeName) > 0)
	    {
		i++;
		break;
	    }
	}
	_children.add(i, node);
	_treeModel.nodesWereInserted(this, new int[]{i});
    }

    private void removeNodes(int[] toRemoveIndices, java.util.List toRemove)
    {
	if(toRemove.size() > 0)
	{
	    _children.removeAll(toRemove);
	    _treeModel.nodesWereRemoved(this, toRemoveIndices, toRemove.toArray());
	}
    }
  
    private final Coordinator _coordinator;

    //
    // Maps application name to current application descriptor
    //
    private final java.util.Map _descriptorMap = new java.util.TreeMap();

    //
    // 'this' is the root of the tree
    //
    private final JTree _tree;
    private final DefaultTreeModel _treeModel;
    
    private String _label;

    static private RegistryEditor _editor;
}
