// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

import java.awt.Component;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;
import javax.swing.JTree;

import IceGrid.*;
import IceGridGUI.*;

class Nodes extends ListTreeNode
{
    static public java.util.Map
    copyDescriptors(java.util.Map descriptors)
    {
        java.util.Map copy = new java.util.HashMap();
        java.util.Iterator p = descriptors.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            
            copy.put(entry.getKey(), 
                     Node.copyDescriptor(
                         (NodeDescriptor)entry.getValue()));
        }
        return copy;
    }

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

    public void paste()
    {
        Object descriptor = getCoordinator().getClipboard();
        newNode(Node.copyDescriptor((NodeDescriptor)descriptor));
    }
    
    public void newNode()
    {
        newNode(new NodeDescriptor(
                    new java.util.TreeMap(),
                    new java.util.LinkedList(),
                    new java.util.LinkedList(),
                    "",
                    "",
                    new java.util.HashMap()));
    }
    
    Nodes(TreeNode parent, java.util.Map descriptors)
        throws UpdateFailedException
    {
        super(false, parent, "Nodes");
        _descriptors = descriptors;

        java.util.Iterator p = _descriptors.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String nodeName = (String)entry.getKey();
            NodeDescriptor nodeDescriptor = (NodeDescriptor)entry.getValue();
            insertChild(new Node(false, this, nodeName, nodeDescriptor), false); 
        }
    }

    //
    // Try to rebuild all my children
    // No-op if it fails
    //
    void rebuild() throws UpdateFailedException
    {
        java.util.List backupList = new java.util.Vector();
        java.util.List editables = new java.util.LinkedList();

        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
            try
            {
                backupList.add(node.rebuild(editables));
            }
            catch(UpdateFailedException e)
            {
                for(int i = backupList.size() - 1; i >= 0; --i)
                {
                    ((Node)_children.get(i)).restore((Node.Backup)backupList.get(i));
                }
                throw e;
            }
        }

        //
        // Success
        //
        p = editables.iterator();
        while(p.hasNext())
        {
            Editable editable = (Editable)p.next();
            editable.markModified();
        }
    }

    void commit()
    {
        _editable.commit();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
            node.commit();
        }
    }

    java.util.LinkedList getUpdates()
    {
        java.util.LinkedList updates = new java.util.LinkedList();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
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
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
            node.removeServerInstances(templateId);
        }
    }

    java.util.List findServiceInstances(String template)
    {
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
            result.addAll(node.findServiceInstances(template));
        }
        return result;
    }


    void removeServiceInstances(String templateId)
    {
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
            node.removeServiceInstances(templateId);
        }
    }

    void update(java.util.List updates, String[] removeNodes,
                java.util.Set serverTemplates, java.util.Set serviceTemplates)
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
        java.util.Vector newChildren = new java.util.Vector();
        java.util.Set updatedNodes = new java.util.HashSet();
        
        java.util.Iterator p = updates.iterator();
        while(p.hasNext())
        {
            NodeUpdateDescriptor update = (NodeUpdateDescriptor)p.next();
            Node node = findNode(update.name);

            if(node == null)
            {
                NodeDescriptor nodeDescriptor = new NodeDescriptor(update.variables,
                                                                   update.serverInstances,
                                                                   update.servers,
                                                                   update.loadFactor.value,
                                                                   update.description.value,
                                                                   new java.util.HashMap());
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
        p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
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

    java.util.List findServerInstances(String template)
    {
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Node node = (Node)p.next();
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

    

    private java.util.Map _descriptors;
    static private JPopupMenu _popup;
}
