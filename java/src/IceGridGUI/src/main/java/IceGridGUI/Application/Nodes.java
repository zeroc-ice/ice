// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.Application;

import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import IceGrid.*;
import IceGridGUI.*;

class Nodes extends ListTreeNode
{
    static public java.util.Map<String, NodeDescriptor>
    copyDescriptors(java.util.Map<String, NodeDescriptor> descriptors)
    {
        java.util.Map<String, NodeDescriptor> copy = new java.util.HashMap<String, NodeDescriptor>();
        for(java.util.Map.Entry<String, NodeDescriptor> p : descriptors.entrySet())
        {
            copy.put(p.getKey(), Node.copyDescriptor(p.getValue()));
        }
        return copy;
    }

    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

        Object descriptor =  getCoordinator().getClipboard();
        if(descriptor != null)
        {
            actions[PASTE] = descriptor instanceof NodeDescriptor;
        }
        actions[NEW_NODE] = true;
        return actions;
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        ApplicationActions actions = getCoordinator().getActionsForPopup();
        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(actions.get(NEW_NODE));
        }
        actions.setTarget(this);
        return _popup;
    }

    @Override
    public void paste()
    {
        Object descriptor = getCoordinator().getClipboard();
        NodeDescriptor nd = Node.copyDescriptor((NodeDescriptor)descriptor);

        //
        // Verify / fix all template instances
        //

        for(ServerInstanceDescriptor p : nd.serverInstances)
        {
            TemplateDescriptor td = getRoot().findServerTemplateDescriptor(p.template);
            if(td == null)
            {
                JOptionPane.showMessageDialog(
                    getCoordinator().getMainFrame(),
                    "Descriptor refers to undefined server template '" + p.template + "'",
                    "Cannot paste",
                    JOptionPane.ERROR_MESSAGE);
                return;
            }
            else
            {
                p.parameterValues.keySet().retainAll(td.parameters);
            }
        }

        for(ServerDescriptor p : nd.servers)
        {
            if(p instanceof IceBoxDescriptor)
            {
                if(!getRoot().pasteIceBox((IceBoxDescriptor)p))
                {
                    return;
                }
            }
        }

        newNode(nd);
    }

    @Override
    public void newNode()
    {
        newNode(new NodeDescriptor(
                    new java.util.TreeMap<String, String>(),
                    new java.util.LinkedList<ServerInstanceDescriptor>(),
                    new java.util.LinkedList<ServerDescriptor>(),
                    "",
                    "",
                    new java.util.HashMap<String, PropertySetDescriptor>()));
    }

    Nodes(TreeNode parent, java.util.Map<String, NodeDescriptor> descriptors)
        throws UpdateFailedException
    {
        super(false, parent, "Nodes");
        _descriptors = descriptors;

        for(java.util.Map.Entry<String, NodeDescriptor> p : _descriptors.entrySet())
        {
            String nodeName = p.getKey();
            NodeDescriptor nodeDescriptor = p.getValue();
            insertChild(new Node(false, this, nodeName, nodeDescriptor), false);
        }
    }

    //
    // Try to rebuild all my children
    // No-op if it fails
    //
    void rebuild()
        throws UpdateFailedException
    {
        java.util.List<Node.Backup> backupList = new java.util.ArrayList<Node.Backup>();
        java.util.List<Editable> editables = new java.util.LinkedList<Editable>();

        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            try
            {
                backupList.add(node.rebuild(editables));
            }
            catch(UpdateFailedException e)
            {
                for(int i = backupList.size() - 1; i >= 0; --i)
                {
                    ((Node)_children.get(i)).restore(backupList.get(i));
                }
                throw e;
            }
        }

        //
        // Success
        //
        for(Editable p : editables)
        {
            p.markModified();
        }
    }

    void commit()
    {
        _editable.commit();
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            node.commit();
        }
    }

    java.util.LinkedList<NodeUpdateDescriptor> getUpdates()
    {
        java.util.LinkedList<NodeUpdateDescriptor> updates = new java.util.LinkedList<NodeUpdateDescriptor>();
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            NodeUpdateDescriptor d = node.getUpdate();
            if(d != null)
            {
                updates.add(d);
            }
        }
        return updates;
    }

    void removeServerInstances(String templateId)
    {
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            node.removeServerInstances(templateId);
        }
    }

    java.util.List<ServiceInstance> findServiceInstances(String template)
    {
        java.util.List<ServiceInstance> result = new java.util.LinkedList<ServiceInstance>();
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            result.addAll(node.findServiceInstances(template));
        }
        return result;
    }

    void removeServiceInstances(String templateId)
    {
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            node.removeServiceInstances(templateId);
        }
    }

    void update(java.util.List<NodeUpdateDescriptor> updates, String[] removeNodes,
                java.util.Set<String> serverTemplates, java.util.Set<String> serviceTemplates)
        throws UpdateFailedException
    {
        Root root = getRoot();

        //
        // Note: _descriptors has already the nodes removed but
        // needs to get the updated and new NodeDescriptors
        //

        //
        // One big set of removes
        //
        removeChildren(removeNodes);

        //
        // One big set of updates, followed by inserts
        //
        java.util.List<TreeNodeBase> newChildren = new java.util.ArrayList<TreeNodeBase>();
        java.util.Set<Node> updatedNodes = new java.util.HashSet<Node>();

        for(NodeUpdateDescriptor update : updates)
        {
            Node node = findNode(update.name);

            if(node == null)
            {
                NodeDescriptor nodeDescriptor =
                    new NodeDescriptor(update.variables,
                                       update.serverInstances,
                                       update.servers,
                                       update.loadFactor == null ? "" : update.loadFactor.value,
                                       update.description == null ? "" : update.description.value,
                                       new java.util.HashMap<String, PropertySetDescriptor>());
                _descriptors.put(update.name, nodeDescriptor);
                node = new Node(false, this, update.name, nodeDescriptor);
                newChildren.add(node);
            }
            else
            {
                node.update(update, serverTemplates, serviceTemplates);
                updatedNodes.add(node);
            }
        }

        //
        // Some nodes are only affected by template updates
        //
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            if(!updatedNodes.contains(node))
            {
                node.update(null, serverTemplates, serviceTemplates);
            }
        }

        insertChildren(newChildren, true);
    }

    Node findNode(String nodeName)
    {
        return (Node)findChild(nodeName);
    }

    java.util.List<ServerInstance> findServerInstances(String template)
    {
        java.util.List<ServerInstance> result = new java.util.LinkedList<ServerInstance>();
        for(TreeNodeBase p : _children)
        {
            Node node = (Node)p;
            result.addAll(node.findServerInstances(template));
        }
        return result;
    }

    void addDescriptor(String nodeName, NodeDescriptor descriptor)
    {
        _descriptors.put(nodeName, descriptor);
    }

    void removeDescriptor(String nodeName)
    {
        _descriptors.remove(nodeName);
    }

    @Override
    Object getDescriptor()
    {
        return _descriptors;
    }

    void tryAdd(String nodeName, NodeDescriptor descriptor)
        throws UpdateFailedException
    {
        Node node = new Node(true, this, nodeName, descriptor);
        insertChild(node, true);
        _descriptors.put(nodeName, descriptor);
    }

    private void newNode(NodeDescriptor descriptor)
    {
        String name = makeNewChildId("NewNode");

        Node node = new Node(this, name, descriptor);
        try
        {
            insertChild(node, true);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
        getRoot().setSelectedNode(node);
    }

    private java.util.Map<String, NodeDescriptor> _descriptors;
    static private JPopupMenu _popup;
}
