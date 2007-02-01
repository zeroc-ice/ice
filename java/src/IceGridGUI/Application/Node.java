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
import java.util.Enumeration;

import javax.swing.Icon;
import javax.swing.JMenuItem;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTextField;
import javax.swing.JTree;

import javax.swing.tree.DefaultTreeModel;
import javax.swing.tree.TreeCellRenderer;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.*;
import IceGridGUI.*;

class Node extends TreeNode implements PropertySetParent
{  
    static public NodeDescriptor
    copyDescriptor(NodeDescriptor nd)
    {
        NodeDescriptor copy = (NodeDescriptor)nd.clone();
        
        copy.propertySets = PropertySets.copyDescriptors(copy.propertySets);
        

        copy.serverInstances = new java.util.LinkedList();
        java.util.Iterator p = nd.serverInstances.iterator();
        while(p.hasNext())
        {
            copy.serverInstances.add(ServerInstance.copyDescriptor(
                                         (ServerInstanceDescriptor)p.next()));
        }
       
        copy.servers = new java.util.LinkedList();
        p = nd.servers.iterator();
        while(p.hasNext())
        {
            copy.servers.add(PlainServer.copyDescriptor(
                                 (ServerDescriptor)p.next()));
        }
        
        return copy;
    }
    
    public Enumeration children()
    {
        return new Enumeration()
            {
                public boolean hasMoreElements()
                {
                    if(!_p.hasNext())
                    {
                        if(!_iteratingOverServers)
                        {
                            _p = _servers.iterator();
                            _iteratingOverServers = true;
                            return _p.hasNext();
                        }
                        return false;
                    }
                    return true;
                }

                public Object nextElement()
                {
                    return _p.next();
                }
                
                private java.util.Iterator _p = _propertySets.iterator();
                private boolean _iteratingOverServers = false;
            };
    }
    
    public boolean getAllowsChildren()
    {
        return true;
    }
    
    public javax.swing.tree.TreeNode getChildAt(int childIndex)
    {
        if(childIndex < 0)
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
        else if(childIndex < _propertySets.size())
        {
            return (javax.swing.tree.TreeNode)_propertySets.get(childIndex);
        }
        else if(childIndex < (_propertySets.size() + _servers.size()))
        {
            return (javax.swing.tree.TreeNode)_servers.get(
                childIndex - _propertySets.size());
        }
        else
        {
            throw new ArrayIndexOutOfBoundsException(childIndex);
        }
    }
   
    public int getChildCount()
    {
        return _propertySets.size() + _servers.size();
    }
    
    public int getIndex(javax.swing.tree.TreeNode node)
    {
        if(node instanceof PropertySet)
        {
            return _propertySets.indexOf(node);
        }
        else
        {
            int index = _servers.indexOf(node);
            if(index != -1)
            {
                index += _propertySets.size();
            }
            return index;
        }
    }

    public boolean isLeaf()
    {
        return _propertySets.isEmpty() && _servers.isEmpty();
    }

    void removeServers(String[] childIds)
    {
        removeSortedChildren(childIds, _servers, getRoot().getTreeModel());
    }

    void removePropertySets(String[] childIds)
    {
        removeSortedChildren(childIds, _propertySets, getRoot().getTreeModel());
    }
    
    void childrenChanged(java.util.List children)
    {
        childrenChanged(children, getRoot().getTreeModel());
    }

    Server findServer(String id)
    {
        return (Server)find(id, _servers);
    }

    PropertySet findPropertySet(String id)
    {
        return (PropertySet)find(id, _propertySets);
    }

    void insertPropertySets(java.util.List newChildren, boolean fireEvent)
        throws UpdateFailedException
    {
        DefaultTreeModel treeModel = fireEvent ?
            getRoot().getTreeModel() : null;
        
        String badChildId = insertSortedChildren(newChildren, _propertySets, treeModel);
        
        if(badChildId != null)
        {
            throw new UpdateFailedException(this, badChildId);
        }
    }
        

    void insertServer(TreeNode child, boolean fireEvent)
        throws UpdateFailedException
    {
        DefaultTreeModel treeModel = fireEvent ?
            getRoot().getTreeModel() : null;
        
        if(!insertSortedChild(child, _servers, treeModel))
        {
            throw new UpdateFailedException(this, child.getId());
        }
    }

    void insertServers(java.util.List newChildren, boolean fireEvent)
        throws UpdateFailedException
    {
        DefaultTreeModel treeModel = fireEvent ?
            getRoot().getTreeModel() : null;
        
        String badChildId = insertSortedChildren(newChildren, _servers, treeModel);
        
        if(badChildId != null)
        {
            throw new UpdateFailedException(this, badChildId);
        }
    }

    void removeServer(TreeNode child)
    {
        int index = getIndex(child);
        _servers.remove(child);
        
        getRoot().getTreeModel().nodesWereRemoved(this,
                                                  new int[]{index},
                                                  new Object[]{child});
    }

    public void insertPropertySet(PropertySet child, boolean fireEvent)
        throws UpdateFailedException
    {
        DefaultTreeModel treeModel = fireEvent ?
            getRoot().getTreeModel() : null;
        
        if(!insertSortedChild(child, _propertySets, treeModel))
        {
            throw new UpdateFailedException(this, child.getId());
        }
    }

    public void removePropertySet(PropertySet child)
    {
        int index = getIndex(child);
        _propertySets.remove(child);
        
        getRoot().getTreeModel().nodesWereRemoved(this,
                                                  new int[]{index},
                                                  new Object[]{child});
    }

    public void removeDescriptor(String id)
    {
        _descriptor.propertySets.remove(id);
    }

    public Editable getEditable()
    {
        return _editable;
    }

    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

        actions[COPY] = !_ephemeral;
        actions[DELETE] = true;

        Object descriptor =  getCoordinator().getClipboard();
        if(descriptor != null)
        {
            actions[PASTE] = descriptor instanceof NodeDescriptor ||
                descriptor instanceof ServerInstanceDescriptor ||
                descriptor instanceof ServerDescriptor ||
                descriptor instanceof PropertySetDescriptor;
        }

        if(!_ephemeral)
        {
            actions[SHOW_VARS] = true;
            actions[SUBSTITUTE_VARS] = true;
            actions[NEW_PROPERTY_SET] = true;
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
            _popup.add(actions.get(NEW_PROPERTY_SET));
            _popup.addSeparator();
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
        else if(descriptor instanceof PropertySetDescriptor)
        {
            newPropertySet(PropertySet.copyDescriptor((PropertySetDescriptor)descriptor));
        }
        else if(descriptor instanceof ServerInstanceDescriptor)
        {
            newServer(ServerInstance.copyDescriptor((ServerInstanceDescriptor)descriptor));
        }
        else
        {
            newServer(PlainServer.copyDescriptor(((ServerDescriptor)descriptor)));
        }
    }
    
    public void newPropertySet()
    {
        newPropertySet(new PropertySetDescriptor(
                           new String[0], new java.util.LinkedList()));
    }

    public void newServer()
    {
        newServer(PlainServer.newServerDescriptor());
    }
    public void newServerIceBox()
    {
        newServer(PlainServer.newIceBoxDescriptor());
    }
    public void newServerFromTemplate()
    {
        ServerInstanceDescriptor descriptor = 
            new ServerInstanceDescriptor("",
                                         new java.util.HashMap(),
                                         new PropertySetDescriptor(new String[0], new java.util.LinkedList()),
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
            nodes.getEditable().removeElement(_id, _editable, Node.class);
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
        return new NodeEditor();
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
            
            if(_descriptor.description.length() > 0)
            {
                writer.writeElement("description", _descriptor.description);
            }
            writeVariables(writer, _descriptor.variables);

            
            java.util.Iterator p = _propertySets.iterator();
            while(p.hasNext())
            {
                PropertySet ps = (PropertySet)p.next();
                ps.write(writer);
            }

            p = _servers.iterator();
            while(p.hasNext())
            {
                TreeNode server = (TreeNode)p.next();
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
    // We don't rebuild the property sets since they don't 
    // depend on the variables.
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
        backup.servers = (java.util.LinkedList)_servers.clone();

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

        java.util.Iterator p = _propertySets.iterator();
        while(p.hasNext())
        {
            PropertySet ps = (PropertySet)p.next();
            ps.commit();
        }

        p = _servers.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();
            server.getEditable().commit();
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
    
    ServerInstance createServer(boolean brandNew, 
                                ServerInstanceDescriptor instanceDescriptor) 
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
        boolean isIceBox = serverDescriptor instanceof IceBoxDescriptor;
        
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
        return new ServerInstance(brandNew, this, serverId, 
                                  instanceResolver, instanceDescriptor, isIceBox);
    }

    PlainServer createServer(boolean brandNew, ServerDescriptor serverDescriptor) 
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
        return new PlainServer(brandNew, this, serverId, 
                               instanceResolver, serverDescriptor);

    }

    NodeUpdateDescriptor getUpdate()
    {
        NodeUpdateDescriptor update = new NodeUpdateDescriptor();
        update.name = _id;

        //
        // First: property sets
        //
        if(_editable.isNew())
        {
            update.removePropertySets = new String[0];
            update.propertySets = _descriptor.propertySets;
        }
        else
        {
            update.removePropertySets = _editable.removedElements(PropertySet.class);
            update.propertySets = new java.util.HashMap();

            java.util.Iterator p = _propertySets.iterator();
            while(p.hasNext())
            {
                PropertySet ps = (PropertySet)p.next();
                if(ps.getEditable().isNew() || ps.getEditable().isModified())
                {
                    update.propertySets.put(ps.getId(), ps.getDescriptor());
                }
            }
        }

        //
        // Then: servers
        //
        if(_editable.isNew())
        {
            update.removeServers = new String[0];
        }
        else
        {
            update.removeServers = _editable.removedElements(Server.class);
        }

        update.serverInstances = new java.util.LinkedList();
        update.servers = new java.util.LinkedList();

        java.util.Iterator p = _servers.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();
            if(_editable.isNew() || server.getEditable().isModified() 
               || server.getEditable().isNew())
            {
                if(server instanceof PlainServer)
                {
                    update.servers.add(server.getDescriptor());
                }
                else
                {
                    update.serverInstances.add(server.getDescriptor());
                }
            }
        }
        
        //
        // Anything in this update?
        //
        if(!_editable.isNew() && !_editable.isModified() 
           && update.removePropertySets.length == 0
           && update.propertySets.size() == 0
           && update.removeServers.length == 0
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
            update.variables = new java.util.TreeMap(_descriptor.variables);
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

        java.util.Vector newServers = new java.util.Vector();
        java.util.Vector updatedServers = new java.util.Vector();

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
            // Property Sets
            //
            removePropertySets(update.removePropertySets);
            for(int i = 0; i < update.removePropertySets.length; ++i)
            {
                _descriptor.propertySets.remove(update.removePropertySets[i]);
            }
            
            java.util.Vector newPropertySets = new java.util.Vector();
            java.util.Vector updatedPropertySets = new java.util.Vector();

            java.util.Iterator p = update.propertySets.entrySet().iterator();
            while(p.hasNext())
            {
                java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                
                String id = (String)entry.getKey();
                PropertySetDescriptor psd = (PropertySetDescriptor)entry.getValue();
                
                //
                // Lookup named property set
                //
                PropertySet ps = findPropertySet(id);
                if(ps != null)
                {
                    ps.rebuild(psd);
                    updatedPropertySets.add(ps);
                }
                else
                {
                    ps = new PropertySet(false, this, id, id, psd);
                    newPropertySets.add(ps);
                    _descriptor.propertySets.put(id, psd);
                }
            }
            childrenChanged(updatedPropertySets);
            insertPropertySets(newPropertySets, true);  

           
            //
            // Update _descriptor
            //
            for(int i = 0; i < update.removeServers.length; ++i)
            {
                Server server = findServer(update.removeServers[i]);
                removeDescriptor(server);
            } 

            //
            // One big set of removes
            //
            removeServers(update.removeServers);
            
            //
            // One big set of updates, followed by inserts
            //
            p = update.serverInstances.iterator();
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
                // Lookup servers
                //
                ServerInstance server = (ServerInstance)findServer(serverId);
                if(server != null)
                {
                    removeDescriptor(server);
                    server.rebuild(instanceResolver, instanceDescriptor,
                                   serverDescriptor instanceof IceBoxDescriptor);
                    updatedServers.add(server);
                    _descriptor.serverInstances.add(instanceDescriptor);
                }
                else
                {
                    server = new ServerInstance(false, this, serverId, instanceResolver,
                                                instanceDescriptor,
                                                serverDescriptor instanceof IceBoxDescriptor);
                    newServers.add(server);
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
                PlainServer server = (PlainServer)findServer(serverId);
                
                if(server != null)
                {
                    removeDescriptor(server);
                    server.rebuild(instanceResolver, serverDescriptor);
                    updatedServers.add(server);
                    _descriptor.servers.add(serverDescriptor);
                }
                else
                {
                    server = new PlainServer(false, this, serverId, instanceResolver,
                                             serverDescriptor);
                    newServers.add(server);
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
                ServerInstance server = (ServerInstance)q.next();
                if(!updatedServers.contains(server) && !newServers.contains(server))
                {
                    serverSet.add(server);
                }
            }
        }

        //
        // Servers affected by service-template updates
        //
        p = serviceTemplates.iterator();
        while(p.hasNext())
        {
            java.util.List serviceInstances = 
                findServiceInstances((String)p.next());
            java.util.Iterator q = serviceInstances.iterator();
            while(q.hasNext())
            {
                ServiceInstance service = (ServiceInstance)q.next();
                Server server = (Server)service.getParent().getParent();
                if(!updatedServers.contains(server) && !newServers.contains(server))
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

            if(server instanceof PlainServer)
            {
                PlainServer ps = (PlainServer)server;
                ServerDescriptor serverDescriptor = (ServerDescriptor)ps.getDescriptor();
                Utils.Resolver instanceResolver = new Utils.Resolver(_resolver);
                
                String serverId = instanceResolver.substitute(serverDescriptor.id);
                assert serverId.equals(ps.getId());
                
                ps.rebuild(instanceResolver, serverDescriptor);
            }
            else
            {
                ServerInstance si = (ServerInstance)server;
                ServerInstanceDescriptor instanceDescriptor = (ServerInstanceDescriptor)si.getDescriptor();

                TemplateDescriptor templateDescriptor = 
                    root.findServerTemplateDescriptor(instanceDescriptor.template);
                assert templateDescriptor != null;
            
                ServerDescriptor serverDescriptor = (ServerDescriptor)templateDescriptor.descriptor;
                assert serverDescriptor != null;

                Utils.Resolver instanceResolver = 
                    new Utils.Resolver(_resolver, 
                                       instanceDescriptor.parameterValues,
                                       templateDescriptor.parameterDefaults);
          
                String serverId = instanceResolver.substitute(serverDescriptor.id);
                assert serverId.equals(si.getId());

                si.rebuild(instanceResolver, instanceDescriptor, 
                           serverDescriptor instanceof IceBoxDescriptor);
            }
            updatedServers.add(server);
        }
        
        childrenChanged(updatedServers);
        insertServers(newServers, true);  
    }

    Node(boolean brandNew, TreeNode parent, String nodeName, NodeDescriptor descriptor)
        throws UpdateFailedException
    {
        super(parent, nodeName);
        _editable = new Editable(brandNew);

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
        // Property Sets
        //
        java.util.Iterator p = _descriptor.propertySets.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            String id = (String)entry.getKey();
            insertPropertySet(new PropertySet(false, this, 
                                              id, id, 
                                              (PropertySetDescriptor)entry.getValue()),
                              false);
        }

        //
        // Template instances
        //
        p = _descriptor.serverInstances.iterator();
        while(p.hasNext())
        {
            ServerInstanceDescriptor instanceDescriptor = 
                (ServerInstanceDescriptor)p.next();
           
            insertServer(createServer(false, instanceDescriptor), false);
        }

        //
        // Plain servers
        //
        p = _descriptor.servers.iterator();
        while(p.hasNext())
        {
            ServerDescriptor serverDescriptor = (ServerDescriptor)p.next();
            insertServer(createServer(false, serverDescriptor), false);
        }
    } 
    
    Node(TreeNode parent, String nodeName, NodeDescriptor descriptor)
    {
        super(parent, nodeName);
        _editable = new Editable(false);
        _ephemeral = true;
        _descriptor = descriptor;
    }

    java.util.List findServerInstances(String template)
    {
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _servers.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();

            if(server instanceof ServerInstance)
            {
                ServerInstanceDescriptor instanceDescriptor
                    = (ServerInstanceDescriptor)server.getDescriptor();

                if(instanceDescriptor.template.equals(template))
                {
                    result.add(server);
                }
            }
        }
        return result;
    }


    void removeServerInstances(String template)
    {
        java.util.List toRemove = new java.util.LinkedList();

        java.util.Iterator p = _servers.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();

            if(server instanceof ServerInstance)
            {
                ServerInstanceDescriptor instanceDescriptor
                    = (ServerInstanceDescriptor)server.getDescriptor();

                if(instanceDescriptor.template.equals(template))
                {
                    //
                    // Remove instance
                    //
                    removeDescriptor(instanceDescriptor);
                    String id = ((TreeNode)server).getId();
                    _editable.removeElement(id, server.getEditable(), Server.class);
                    toRemove.add(id);
                }
            }
        }

        if(toRemove.size() > 0)
        {
            removeServers((String[])toRemove.toArray(new String[0]));
        }
    }
    
    java.util.List findServiceInstances(String template)
    {
        java.util.List result = new java.util.LinkedList();
        java.util.Iterator p = _servers.iterator();
        while(p.hasNext())
        {
            Object o = p.next();
            if(o instanceof PlainServer)
            {
                result.addAll(((PlainServer)o).findServiceInstances(template));
            }
        }
        return result;
    }

    void removeServiceInstances(String template)
    {   
        java.util.Iterator p = _servers.iterator();
        while(p.hasNext())
        {
            Object o = p.next();
            if(o instanceof PlainServer)
            {
                ((PlainServer)o).removeServiceInstances(template);
            }
        }
    }

    Utils.Resolver getResolver()
    {
        return _resolver;
    }


    public void tryAdd(String id, PropertySetDescriptor descriptor) 
        throws UpdateFailedException
    {
        insertPropertySet(new PropertySet(true, this, id, id, descriptor), 
                          true);
        _descriptor.propertySets.put(id, descriptor);
    }

    public void tryRename(String oldId, String oldId2, String newId)
        throws UpdateFailedException
    {
        PropertySet oldChild = findPropertySet(oldId);
        assert oldChild != null;
        removePropertySet(oldChild);
        PropertySetDescriptor descriptor = (PropertySetDescriptor)oldChild.getDescriptor();

        try
        {
            insertPropertySet(
                new PropertySet(true, this, newId, newId, descriptor),
                true);
        }
        catch(UpdateFailedException ex)
        {
            try
            {
                insertPropertySet(oldChild, true);
            }
            catch(UpdateFailedException ufe)
            {
                assert false;
            }
            throw ex;
        }

        _editable.removeElement(oldId, oldChild.getEditable(), PropertySet.class);
        _descriptor.propertySets.remove(oldId);
        _descriptor.propertySets.put(newId, descriptor);
    }

    void tryAdd(ServerInstanceDescriptor instanceDescriptor,
                boolean addDescriptor) throws UpdateFailedException
    {
        insertServer(createServer(true, instanceDescriptor), true);

        if(addDescriptor)
        {
            _descriptor.serverInstances.add(instanceDescriptor);   
        }
    }

    void tryAdd(ServerDescriptor serverDescriptor,
                boolean addDescriptor) throws UpdateFailedException
    {
        insertServer(createServer(true, serverDescriptor), true);

        if(addDescriptor)
        {
            _descriptor.servers.add(serverDescriptor);
        }
    }

    void removeDescriptor(Server server)
    {
        if(server instanceof ServerInstance)
        {
            removeDescriptor((ServerInstanceDescriptor)server.getDescriptor());
        }
        else
        {
            removeDescriptor((ServerDescriptor)server.getDescriptor());
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

    private void newPropertySet(PropertySetDescriptor descriptor)
    {
        String id = makeNewChildId("PropertySet");
        
        PropertySet ps = new PropertySet(this, id, descriptor);
        try
        {
            insertPropertySet(ps, true);
        }
        catch(UpdateFailedException e)
        {
            assert false;
        }
        getRoot().setSelectedNode(ps);
    }

    private void newServer(ServerDescriptor descriptor)
    {
        descriptor.id = makeNewChildId(descriptor.id);
        
        PlainServer server = new PlainServer(this, descriptor.id, descriptor);
        try
        {
            insertServer(server, true);
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
            if(root.getServerTemplates().getChildCount() == 0)
            {
                JOptionPane.showMessageDialog(
                    getCoordinator().getMainFrame(),
                    "You need to create a server template before you can create a server from a template.",
                    "No Server Template",
                    JOptionPane.INFORMATION_MESSAGE);
                return;
            }
            
            t = (ServerTemplate)root.getServerTemplates().getChildAt(0);
            descriptor.template = t.getId();
            descriptor.parameterValues = new java.util.HashMap();
        }

        ServerInstance server = new ServerInstance(this, id, descriptor);
        try
        {
            insertServer(server, true);
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

    private java.util.LinkedList _propertySets = new java.util.LinkedList();
    private java.util.LinkedList _servers = new java.util.LinkedList();
   
    private Editable _editable;

    static private DefaultTreeCellRenderer _cellRenderer;   
    static private JPopupMenu _popup;
}
