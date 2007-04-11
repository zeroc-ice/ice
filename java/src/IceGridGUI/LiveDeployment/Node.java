// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;

import java.text.NumberFormat;

import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

class Node extends ListTreeNode
{
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];
        actions[SHUTDOWN_NODE] = _up;
        actions[RETRIEVE_STDOUT] = _up;
        actions[RETRIEVE_STDERR] = _up;
        return actions;
    }

    public void retrieveOutput(final boolean stdout)
    {
        getRoot().openShowLogDialog(new ShowLogDialog.FileIteratorFactory()
            {
                public FileIteratorPrx open(int count)
                    throws FileNotAvailableException, NodeNotExistException, NodeUnreachableException
                {
                    AdminSessionPrx session = getRoot().getCoordinator().getSession();
                    FileIteratorPrx result;
                    if(stdout)
                    {
                        result = session.openNodeStdOut(_id, count);
                    }
                    else
                    {
                        result = session.openNodeStdErr(_id, count);
                    }
                    if(getRoot().getCoordinator().getCommunicator().getDefaultRouter() == null)
                    {
                        result = FileIteratorPrxHelper.uncheckedCast(
                            result.ice_endpoints(session.ice_getEndpoints()));
                    }
                    return result;
                }

                public String getTitle()
                {
                    return "Node " + _id + " " + (stdout ? "stdout" : "stderr");
                }

                public String getDefaultFilename()
                {
                    return _id + (stdout ? ".out" : ".err");
                }
            });
    } 


    public void shutdownNode()
    {
        final String prefix = "Shutting down node '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        AMI_Admin_shutdownNode cb = new AMI_Admin_shutdownNode()
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
                    amiFailure(prefix, "Failed to shutdown " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to shutdown " + _id, 
                               e.toString());
                }
            };

        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            
            getCoordinator().getAdmin().shutdownNode_async(cb, _id);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to shutdown " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.addSeparator();
            _popup.add(la.get(SHUTDOWN_NODE));
        }
        
        la.setTarget(this);
        return _popup;
    }


    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new NodeEditor();
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
            _nodeUp = Utils.getIcon("/icons/16x16/node_up.png");
            _nodeDown = Utils.getIcon("/icons/16x16/node_down.png");
        }

        if(_up)
        {
            _cellRenderer.setToolTipText("Up and running");
            if(expanded)
            {
                _cellRenderer.setOpenIcon(_nodeUp);
            }
            else
            {
                _cellRenderer.setClosedIcon(_nodeUp);
            }
        }
        else
        {
            _cellRenderer.setToolTipText("Not running");
            if(expanded)
            {
                _cellRenderer.setOpenIcon(_nodeDown);
            }
            else
            {
                _cellRenderer.setClosedIcon(_nodeDown);
            }
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    Node(Root parent, NodeDynamicInfo info)
    {
        super(parent, info.info.name);
        up(info, false);
    }


    Node(Root parent, ApplicationDescriptor appDesc, 
         String nodeName, NodeDescriptor nodeDesc)
    {
        super(parent, nodeName);
        add(appDesc, nodeDesc);
    }

    Node(Root parent, ApplicationDescriptor appDesc, NodeUpdateDescriptor update)
    {
        super(parent, update.name);

        NodeDescriptor nodeDesc = new NodeDescriptor(
            update.variables,
            update.serverInstances,
            update.servers,
            update.loadFactor == null ? "" : update.loadFactor.value,
            update.description == null ? "" : update.description.value,
            update.propertySets);
            
        appDesc.nodes.put(_id, nodeDesc);
        add(appDesc, nodeDesc);
    }

    Utils.ExpandedPropertySet expand(PropertySetDescriptor descriptor, 
                                     String applicationName, Utils.Resolver resolver)
    {
        Utils.ExpandedPropertySet result = new Utils.ExpandedPropertySet();
        result.references = new Utils.ExpandedPropertySet[descriptor.references.length];
        
        for(int i = 0; i < descriptor.references.length; ++i)
        {
            result.references[i] = expand(
                findNamedPropertySet(resolver.substitute(descriptor.references[i]), 
                                     applicationName),
                applicationName, resolver);
        }

        result.properties = descriptor.properties;
        return result;
    }
    
    PropertySetDescriptor findNamedPropertySet(String name, String applicationName)
    {
        ApplicationData appData = (ApplicationData)_map.get(applicationName);
        if(appData != null)
        {
            NodeDescriptor descriptor = appData.descriptor;
            PropertySetDescriptor result = (PropertySetDescriptor)descriptor.propertySets.get(name);
            if(result != null)
            {
                return result;
            }
        }
        return getRoot().findNamedPropertySet(name, applicationName);
    }

    void add(ApplicationDescriptor appDesc, NodeDescriptor nodeDesc)
    {
        ApplicationData data = new ApplicationData();
        data.descriptor = nodeDesc;
        data.resolver = new Utils.Resolver(new java.util.Map[]{appDesc.variables, nodeDesc.variables});
        data.resolver.put("application", appDesc.name);
        data.resolver.put("node", _id);
        putInfoVariables(data.resolver);

        _map.put(appDesc.name, data);
        
        java.util.Iterator p = nodeDesc.serverInstances.iterator();
        while(p.hasNext())
        {
            ServerInstanceDescriptor desc = (ServerInstanceDescriptor)p.next();
            insertServer(createServer(appDesc, data.resolver, desc));
        }
        
        p = nodeDesc.servers.iterator();
        while(p.hasNext())
        {
            ServerDescriptor desc = (ServerDescriptor)p.next();
            insertServer(createServer(appDesc, data.resolver, desc));
        }
    }

    boolean remove(String appName)
    {
        _map.remove(appName);
        if(_map.isEmpty() && !_up)
        {
            return true;
        }

        java.util.List toRemove = new java.util.LinkedList();
        int[] toRemoveIndices = new int[_children.size()];
        int i = 0;
        
        for(int index = 0; index < _children.size(); ++index)
        {
            Server server = (Server)_children.get(index);
            if(server.getApplication().name.equals(appName))
            {
                toRemove.add(server);
                toRemoveIndices[i++] = index;
            }
        }
        toRemoveIndices = resize(toRemoveIndices, toRemove.size());
        _children.removeAll(toRemove);
        getRoot().getTreeModel().nodesWereRemoved(this, toRemoveIndices, toRemove.toArray());

        return false;
    }

    void update(ApplicationDescriptor appDesc, NodeUpdateDescriptor update,
                boolean variablesChanged, java.util.Set serviceTemplates,
                java.util.Set serverTemplates)
    {
        ApplicationData data = (ApplicationData)_map.get(appDesc.name);
        
        if(data == null)
        {
            if(update != null)
            {
                NodeDescriptor nodeDesc = new NodeDescriptor(
                    update.variables,
                    update.serverInstances,
                    update.servers,
                    update.loadFactor == null ? "" : update.loadFactor.value,
                    update.description == null ? "" : update.description.value,
                    update.propertySets);
                
                appDesc.nodes.put(_id, nodeDesc);
                add(appDesc, nodeDesc);
            }
            //
            // Else, nothing to do.
            //

            return;
        }

        NodeDescriptor nodeDesc = data.descriptor;
        java.util.Set freshServers = new java.util.HashSet();
        
        if(update != null)
        {
            //
            // Update various fields of nodeDesc
            //
            if(update.description != null)
            {
                nodeDesc.description = update.description.value;
            }
            if(update.loadFactor != null)
            {
                nodeDesc.loadFactor = update.loadFactor.value;
            }
        
            nodeDesc.variables.keySet().removeAll(java.util.Arrays.asList(update.removeVariables));
            nodeDesc.variables.putAll(update.variables);
            
            if(!variablesChanged)
            {
                variablesChanged = update.removeVariables.length > 0 ||
                    !update.variables.isEmpty();
            }

            nodeDesc.propertySets.keySet().removeAll(
                java.util.Arrays.asList(update.removePropertySets));
            nodeDesc.propertySets.putAll(update.propertySets);

            //
            // Remove servers
            //
            for(int i = 0; i < update.removeServers.length; ++i)
            {
                Server server = findServer(update.removeServers[i]);
                if(server == null)
                {
                    //
                    // This should never happen
                    //
                    String errorMsg = "LiveDeployment/Node: unable to remove server '" + update.removeServers[i] 
                        + "'; please report this bug."; 

                    getCoordinator().getCommunicator().getLogger().error(errorMsg);
                }
                else
                {
                    removeDescriptor(nodeDesc, server);
                    int index = getIndex(server); 
                    _children.remove(server);
                    getRoot().getTreeModel().nodesWereRemoved(this, new int[]{index}, new Object[]{server});
                }
            }

            //
            // Add/update servers
            //
            java.util.Iterator p = update.serverInstances.iterator();
            while(p.hasNext())
            {
                ServerInstanceDescriptor desc = (ServerInstanceDescriptor)p.next();
                Server server = createServer(appDesc, data.resolver, desc);
                
                Server oldServer = findServer(server.getId());
                if(oldServer == null)
                {
                    insertServer(server);
                    freshServers.add(server);
                    nodeDesc.serverInstances.add(desc);
                }
                else
                {
                    removeDescriptor(nodeDesc, oldServer);
                    oldServer.rebuild(server);
                    freshServers.add(oldServer);
                    nodeDesc.serverInstances.add(desc);
                }
            }
            
            p = update.servers.iterator();
            while(p.hasNext())
            {
                ServerDescriptor desc = (ServerDescriptor)p.next();
                
                Server server = createServer(appDesc, data.resolver, desc);
                
                Server oldServer = findServer(server.getId());
                if(oldServer == null)
                {
                    insertServer(server);
                    freshServers.add(server);
                    nodeDesc.servers.add(desc);
                }
                else
                {
                    removeDescriptor(nodeDesc, oldServer);
                    oldServer.rebuild(server);
                    freshServers.add(oldServer);
                    nodeDesc.servers.add(desc);
                }
            }
        }
        
        if(variablesChanged || !serviceTemplates.isEmpty() || !serverTemplates.isEmpty())
        {
            //
            // Rebuild every other server in this application
            //
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                Server server = (Server)p.next();
                if(server.getApplication() == appDesc)
                {
                    if(!freshServers.contains(server))
                    {
                        server.rebuild(data.resolver,
                                       variablesChanged, 
                                       serviceTemplates, serverTemplates);
                    }
                }
            }
        }
    }
    
    NodeInfo getStaticInfo()
    {
        if(_info == null)
        {
            return null;
        }
        else
        {
            return _info.info;
        }
    }
    
    boolean isRunningWindows()
    {
        return _windows;
    }

    private boolean putInfoVariables(Utils.Resolver resolver)
    {
        if(_info == null)
        {
            return false;
        }
        else
        {
            boolean updated = resolver.put("node.os", _info.info.os);
            updated = resolver.put("node.hostname", _info.info.hostname) || updated;
            updated = resolver.put("node.release", _info.info.release) || updated; 
            updated = resolver.put("node.version", _info.info.version) || updated;
            updated = resolver.put("node.machine", _info.info.machine) || updated;
            updated = resolver.put("node.datadir", _info.info.dataDir) || updated;
            return updated;
        }
    }

    void up(NodeDynamicInfo info, boolean fireEvent)
    {
        _up = true;
        _info = info;
        _windows = info.info.os.toLowerCase().startsWith("windows");
        
        //
        // Update variables and rebuild all affected servers
        //
        java.util.Iterator p = _map.values().iterator();
        while(p.hasNext())
        {
            ApplicationData data = (ApplicationData)p.next();

            if(putInfoVariables(data.resolver))
            {
                String appName = data.resolver.find("application");

                java.util.Iterator q = _children.iterator();
                while(q.hasNext())
                {
                    Server server = (Server)q.next();
                    if(server.getApplication().name.equals(appName))
                    {
                        server.rebuild(data.resolver, true, null, null);
                    }
                }
            }
        }
        
        //
        // Tell every server on this node
        //
        java.util.Set updatedServers = new java.util.HashSet();
        p = _info.servers.iterator();
        while(p.hasNext())
        {
            ServerDynamicInfo sinfo = (ServerDynamicInfo)p.next();
            Server server = findServer(sinfo.id);
            if(server != null)
            {
                server.update(sinfo.state, sinfo.pid, sinfo.enabled, true);
                updatedServers.add(server);
            }
        }
        p = _children.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();
            if(!updatedServers.contains(server))
            {
                server.update(ServerState.Inactive, 0, true, true);
            }
        }

        //
        // Tell adapters
        //
        p = _children.iterator();
        int updateCount = 0;
        while(p.hasNext() && updateCount < _info.adapters.size())
        {
            Server server = (Server)p.next();
            updateCount += server.updateAdapters(_info.adapters);
        }

        if(fireEvent)
        {
            getRoot().getTreeModel().nodeChanged(this);
        }
    }

    boolean down()
    {
        _up = false;
        _info.servers.clear();
        _info.adapters.clear();

        if(_children.isEmpty())
        {
            return true;
        }
        else
        {
            java.util.Iterator p = _children.iterator();
            while(p.hasNext())
            {
                Server server = (Server)p.next();
                server.nodeDown();
            }

            getRoot().getTreeModel().nodeChanged(this);
            return false;
        }
    }

    void updateServer(ServerDynamicInfo updatedInfo)
    {
        if(_info != null)
        {
            java.util.ListIterator p = _info.servers.listIterator();
            while(p.hasNext())
            {
                ServerDynamicInfo sinfo = (ServerDynamicInfo)p.next();
                if(sinfo.id.equals(updatedInfo.id))
                {
                    p.set(updatedInfo);
                    break;
                }
            }
        }

        Server server = findServer(updatedInfo.id);
        if(server != null)
        {
            server.update(updatedInfo.state, updatedInfo.pid, 
                          updatedInfo.enabled, true);
        }
    }

    void updateAdapter(AdapterDynamicInfo updatedInfo)
    {
        if(_info != null)
        {
            java.util.ListIterator p = _info.adapters.listIterator();
            while(p.hasNext())
            {
                AdapterDynamicInfo ainfo = (AdapterDynamicInfo)p.next();
                if(ainfo.id.equals(updatedInfo.id))
                {
                    p.set(updatedInfo);
                    break;
                }
            }
        }

        java.util.Iterator p = _children.iterator();
        while(p.hasNext())
        {
            Server server = (Server)p.next();
            if(server.updateAdapter(updatedInfo))
            {
                break;
            }
        }
    }
    
    Ice.ObjectPrx getProxy(String adapterId)
    {
        if(_info != null)
        {
            java.util.ListIterator p = _info.adapters.listIterator();
            while(p.hasNext())
            {
                AdapterDynamicInfo ainfo = (AdapterDynamicInfo)p.next();
                if(ainfo.id.equals(adapterId))
                {
                    return ainfo.proxy;
                }
            }
        }
        return null;
    }

    java.util.SortedMap getLoadFactors()
    {
        java.util.SortedMap result = new java.util.TreeMap();
        
        java.util.Iterator p = _map.entrySet().iterator();
        while(p.hasNext())
        {
            java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
            
            ApplicationData ad = (ApplicationData)entry.getValue();
            
            String val = ad.resolver.substitute(ad.descriptor.loadFactor);
            if(val.length() == 0)
            {
                val = "Default";
            }

            result.put(entry.getKey(), val);
        }
        return result;
    }


    void showLoad()
    {
        AMI_Admin_getNodeLoad cb = new AMI_Admin_getNodeLoad()
            {
                public void ice_response(LoadInfo loadInfo)
                {
                    NumberFormat format;
                    if(_windows)
                    {
                        format = NumberFormat.getPercentInstance();
                        format.setMaximumFractionDigits(1);
                        format.setMinimumFractionDigits(1);
                    }
                    else
                    {
                        format = NumberFormat.getNumberInstance();
                        format.setMaximumFractionDigits(2);
                        format.setMinimumFractionDigits(2);
                    }
                    
                    final String load = 
                        format.format(loadInfo.avg1) + " " +
                        format.format(loadInfo.avg5) + " " +
                        format.format(loadInfo.avg15); 

                    SwingUtilities.invokeLater(new Runnable() 
                        {
                            public void run() 
                            {
                                _editor.setLoad(load, Node.this);
                            }
                        });
                }

                public void ice_exception(final Ice.UserException e)
                {
                    SwingUtilities.invokeLater(new Runnable() 
                        {
                            public void run() 
                            {
                                if(e instanceof IceGrid.NodeNotExistException)
                                {
                                    _editor.setLoad(
                                        "Error: this node is not known to this IceGrid Registry",
                                        Node.this);
                                }
                                else if(e instanceof IceGrid.NodeUnreachableException)
                                {
                                    _editor.setLoad("Error: cannot reach this node", Node.this);
                                }
                                else
                                {
                                    _editor.setLoad("Error: " + e.toString(), Node.this);
                                }
                            }
                        });
                }             
                
                public void ice_exception(final Ice.LocalException e)
                {
                    SwingUtilities.invokeLater(new Runnable() 
                        {
                            public void run() 
                            {
                                _editor.setLoad("Error: " + e.toString(), Node.this);
                            }
                        });
                }
            };

        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            
            IceGrid.AdminPrx admin = getCoordinator().getAdmin();
            if(admin == null)
            {
                _editor.setLoad("Unknown", this);
            }
            else
            {
                admin.getNodeLoad_async(cb, _id);
            }
        }
        catch(Ice.LocalException e)
        {
            _editor.setLoad("Error: " + e.toString(), this);
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    
    private Server createServer(ApplicationDescriptor application, 
                                Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor)
    {
        //
        // Find template
        //
        TemplateDescriptor templateDescriptor = 
            (TemplateDescriptor)application.serverTemplates.get(instanceDescriptor.template);
        assert templateDescriptor != null;
            
        ServerDescriptor serverDescriptor = 
            (ServerDescriptor)templateDescriptor.descriptor;
        
        assert serverDescriptor != null;
        
        //
        // Build resolver
        //
        Utils.Resolver instanceResolver = 
            new Utils.Resolver(resolver, 
                               instanceDescriptor.parameterValues,
                               templateDescriptor.parameterDefaults);
        
        String serverId = instanceResolver.substitute(serverDescriptor.id);
        instanceResolver.put("server", serverId);
        
        //
        // Lookup dynamic info
        //
        ServerState serverState = _up ? ServerState.Inactive : null;
        int pid = 0;
        boolean enabled = true;
        if(_info != null)
        {
            java.util.Iterator p = _info.servers.iterator();
            while(p.hasNext())
            {
                ServerDynamicInfo sinfo = (ServerDynamicInfo)p.next();
                if(sinfo.id.equals(serverId))
                {
                    serverState = sinfo.state;
                    pid = sinfo.pid;
                    enabled = sinfo.enabled;
                    break;
                }
            }
        }

        //
        // Create server
        //
        return new Server(this, serverId, instanceResolver, instanceDescriptor, 
                          serverDescriptor, application,
                          serverState, pid, enabled);

    }

    private Server createServer(ApplicationDescriptor application, 
                                Utils.Resolver resolver, ServerDescriptor serverDescriptor)
    {
        //
        // Build resolver
        //
        Utils.Resolver instanceResolver = new Utils.Resolver(resolver);
        String serverId = instanceResolver.substitute(serverDescriptor.id);
        instanceResolver.put("server", serverId);       

        //
        // Lookup dynamic info
        //
        ServerState serverState = _up ? ServerState.Inactive : null;
        int pid = 0;
        boolean enabled = true;
        if(_info != null)
        {
            java.util.Iterator p = _info.servers.iterator();
            while(p.hasNext())
            {
                ServerDynamicInfo sinfo = (ServerDynamicInfo)p.next();
                if(sinfo.id.equals(serverId))
                {
                    serverState = sinfo.state;
                    pid = sinfo.pid;
                    enabled = sinfo.enabled;
                    break;
                }
            }
        }

        //
        // Create server
        //
        return new Server(this, serverId, instanceResolver, null, serverDescriptor, 
                          application, serverState, pid, enabled);

    }

    private void insertServer(Server server)
    {
        insertSortedChild(server, _children, getRoot().getTreeModel());
    }

    private Server findServer(String id)
    {
        return (Server)find(id, _children);
    }


    private void removeDescriptor(NodeDescriptor nodeDesc, Server server)
    {
        ServerInstanceDescriptor instanceDescriptor = 
            server.getInstanceDescriptor();
        if(instanceDescriptor != null)
        {
            removeDescriptor(nodeDesc, instanceDescriptor);
        }
        else
        {
            removeDescriptor(nodeDesc, server.getServerDescriptor());
        }
    }

    private void removeDescriptor(NodeDescriptor nodeDesc, ServerDescriptor sd)
    {
        //
        // A straight remove uses equals(), which is not the desired behavior
        //
        java.util.Iterator p = nodeDesc.servers.iterator();
        while(p.hasNext())
        {
            if(sd == p.next())
            {
                p.remove();
                break;
            }
        }
    }

    private void removeDescriptor(NodeDescriptor nodeDesc, ServerInstanceDescriptor sd)
    {
        //
        // A straight remove uses equals(), which is not the desired behavior
        //
        java.util.Iterator p = nodeDesc.serverInstances.iterator();
        while(p.hasNext())
        {
            if(sd == p.next())
            {
                p.remove();
                break;
            }
        }
    }

 
    static class ApplicationData
    {
        NodeDescriptor descriptor;
        Utils.Resolver resolver;
    }

    //
    // Application name to ApplicationData
    //
    private final java.util.SortedMap _map = new java.util.TreeMap();
    
    private boolean _up = false;
    private NodeDynamicInfo _info;
    private boolean _windows = false;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUp;
    static private Icon _nodeDown;

    static private NodeEditor _editor;
    static private JPopupMenu _popup;
}
