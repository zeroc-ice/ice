// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;

import java.text.NumberFormat;

import IceGrid.*;
import IceGridGUI.*;

class Node extends ListTreeNode
{
    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];
        actions[SHUTDOWN_NODE] = _up;
        actions[RETRIEVE_ICE_LOG] = _up;
        actions[RETRIEVE_STDOUT] = _up;
        actions[RETRIEVE_STDERR] = _up;
        return actions;
    }

    @Override
    public void retrieveIceLog()
    {
        if(_showIceLogDialog == null)
        {
            final String prefix = "Retrieving Admin proxy for Node " + _id + "...";
            final String errorTitle = "Failed to retrieve Admin Proxy for Node " + _id;
            getRoot().getCoordinator().getStatusBar().setText(prefix);
        
            Callback_Admin_getNodeAdmin cb = new Callback_Admin_getNodeAdmin()
            {
                @Override
                public void response(Ice.ObjectPrx prx)
                {
                    final Ice.LoggerAdminPrx loggerAdmin = Ice.LoggerAdminPrxHelper.uncheckedCast(prx.ice_facet("Logger"));
                    final String title = "Node " + _id + " Ice log";
                    final String defaultFileName = "node-" + _id;
                    
                    SwingUtilities.invokeLater(new Runnable()
                    {
                        @Override
                        public void run()
                        {
                            success(prefix);
                            if(_showIceLogDialog == null)
                            {
                                _showIceLogDialog = new ShowIceLogDialog(Node.this, title, loggerAdmin, defaultFileName, 
                                            getRoot().getLogMaxLines(), getRoot().getLogInitialLines());
                            }
                            else
                            {
                                _showIceLogDialog.toFront();
                            }
                        }
                    });
                }
                
                @Override
                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, errorTitle, e);
                }

                @Override
                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, errorTitle, e.toString());
                }
            };
        
            try
            {
                getRoot().getCoordinator().getSession().getAdmin().begin_getNodeAdmin(_id, cb);
            }
            catch(Ice.LocalException e)
            {
                failure(prefix, errorTitle, e.toString());
            }
        }
        else
        {
            _showIceLogDialog.toFront();
        }
    }
    
    @Override
    public void retrieveOutput(final boolean stdout)
    {
        getRoot().openShowLogFileDialog(new ShowLogFileDialog.FileIteratorFactory()
            {
                @Override
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
                    return result;
                }

                @Override
                public String getTitle()
                {
                    return "Node " + _id + " " + (stdout ? "stdout" : "stderr");
                }

                @Override
                public String getDefaultFilename()
                {
                    return _id + (stdout ? ".out" : ".err");
                }
            });
    }

    @Override
    public void shutdownNode()
    {
        final String prefix = "Shutting down node '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_shutdownNode cb = new Callback_Admin_shutdownNode()
            {
                //
                // Called by another thread!
                //
                @Override
                public void response()
                {
                    amiSuccess(prefix);
                }

                @Override
                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to shutdown " + _id, e);
                }

                @Override
                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to shutdown " + _id,
                               e.toString());
                }
            };

        try
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

            getCoordinator().getAdmin().begin_shutdownNode(_id, cb);
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

    @Override
    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(la.get(RETRIEVE_ICE_LOG));
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.addSeparator();
            _popup.add(la.get(SHUTDOWN_NODE));
        }

        la.setTarget(this);
        return _popup;
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new NodeEditor();
        }
        _editor.show(this);
        return _editor;
    }

    @Override
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

        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }
    
    @Override
    public void clearShowIceLogDialog()
    {
        _showIceLogDialog = null;
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

    Utils.ExpandedPropertySet expand(PropertySetDescriptor descriptor, String applicationName, Utils.Resolver resolver)
    {
        Utils.ExpandedPropertySet result = new Utils.ExpandedPropertySet();
        result.references = new Utils.ExpandedPropertySet[descriptor.references.length];

        int i = 0;
        for(String ref : descriptor.references)
        {
            result.references[i++] = expand(findNamedPropertySet(resolver.substitute(ref), applicationName),
                                            applicationName, resolver);
        }

        result.properties = descriptor.properties;
        return result;
    }

    PropertySetDescriptor findNamedPropertySet(String name, String applicationName)
    {
        ApplicationData appData = _map.get(applicationName);
        if(appData != null)
        {
            NodeDescriptor descriptor = appData.descriptor;
            PropertySetDescriptor result = descriptor.propertySets.get(name);
            if(result != null)
            {
                return result;
            }
        }
        return getRoot().findNamedPropertySet(name, applicationName);
    }

    boolean hasServersFromApplication(String name)
    {
        return _map.containsKey(name);
    }

    void add(ApplicationDescriptor appDesc, NodeDescriptor nodeDesc)
    {
        ApplicationData data = new ApplicationData();
        data.descriptor = nodeDesc;
        @SuppressWarnings("unchecked")
        Utils.Resolver resolver = new Utils.Resolver(new java.util.Map[]{nodeDesc.variables, appDesc.variables});
        data.resolver = resolver;
        data.resolver.put("application", appDesc.name);
        data.resolver.put("node", _id);
        putInfoVariables(data.resolver);

        _map.put(appDesc.name, data);

        for(ServerInstanceDescriptor p : nodeDesc.serverInstances)
        {
            insertServer(createServer(appDesc, data.resolver, p));
        }

        for(ServerDescriptor p : nodeDesc.servers)
        {
            insertServer(createServer(appDesc, data.resolver, p));
        }
    }

    boolean remove(String appName)
    {
        _map.remove(appName);
        if(_map.isEmpty() && !_up)
        {
            return true;
        }

        java.util.List<Server> toRemove = new java.util.LinkedList<Server>();
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

    void update(ApplicationDescriptor appDesc, NodeUpdateDescriptor update, boolean variablesChanged,
                java.util.Set<String> serviceTemplates, java.util.Set<String> serverTemplates)
    {
        ApplicationData data = _map.get(appDesc.name);

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
        java.util.Set<Server> freshServers = new java.util.HashSet<Server>();

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
                variablesChanged = update.removeVariables.length > 0 || !update.variables.isEmpty();
            }

            nodeDesc.propertySets.keySet().removeAll(java.util.Arrays.asList(update.removePropertySets));
            nodeDesc.propertySets.putAll(update.propertySets);

            //
            // Remove servers
            //
            for(String id : update.removeServers)
            {
                Server server = findServer(id);
                if(server == null)
                {
                    //
                    // This should never happen
                    //
                    String errorMsg = "LiveDeployment/Node: unable to remove server '" + id +
                        "'; please report this bug.";

                    getCoordinator().getCommunicator().getLogger().error(errorMsg);
                }
                else
                {
                    server.removeCallbacks();
                    removeDescriptor(nodeDesc, server);
                    int index = getIndex(server);
                    _children.remove(server);
                    getRoot().getTreeModel().nodesWereRemoved(this, new int[]{index}, new Object[]{server});
                }
            }

            //
            // Add/update servers
            //
            for(ServerInstanceDescriptor desc : update.serverInstances)
            {
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
                    oldServer.rebuild(server, true);
                    freshServers.add(oldServer);
                    nodeDesc.serverInstances.add(desc);
                }
            }

            for(ServerDescriptor desc : update.servers)
            {
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
                    oldServer.rebuild(server, true);
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
            for(javax.swing.tree.TreeNode p : _children)
            {
                Server server = (Server)p;
                if(server.getApplication() == appDesc)
                {
                    if(!freshServers.contains(server))
                    {
                        server.rebuild(data.resolver, variablesChanged, serviceTemplates, serverTemplates);
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
        for(ApplicationData data : _map.values())
        {
            if(putInfoVariables(data.resolver))
            {
                String appName = data.resolver.find("application");

                for(javax.swing.tree.TreeNode p : _children)
                {
                    Server server = (Server)p;
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
        java.util.Set<Server> updatedServers = new java.util.HashSet<Server>();
        for(ServerDynamicInfo sinfo : _info.servers)
        {
            Server server = findServer(sinfo.id);
            if(server != null)
            {
                server.update(sinfo.state, sinfo.pid, sinfo.enabled, true);
                updatedServers.add(server);
            }
        }
        for(javax.swing.tree.TreeNode p : _children)
        {
            Server server = (Server)p;
            if(!updatedServers.contains(server))
            {
                server.update(ServerState.Inactive, 0, true, true);
            }
        }

        //
        // Tell adapters
        //
        java.util.Iterator<javax.swing.tree.TreeNode> p = _children.iterator();
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
        
        if(_showIceLogDialog != null)
        {
            _showIceLogDialog.stopped();
        }

        if(_children.isEmpty())
        {
            return true;
        }
        else
        {
            for(javax.swing.tree.TreeNode p : _children)
            {
                Server server = (Server)p;
                server.nodeDown();
            }

            getRoot().getTreeModel().nodeChanged(this);

            return false;
        }
    }

    void updateServer(ServerDynamicInfo updatedInfo)
    {
        boolean destroyed = updatedInfo.state == ServerState.Destroyed;

        if(_info != null)
        {
            java.util.ListIterator<ServerDynamicInfo> p = _info.servers.listIterator();
            boolean found = false;
            while(p.hasNext())
            {
                ServerDynamicInfo sinfo = p.next();
                if(sinfo.id.equals(updatedInfo.id))
                {
                    if(destroyed)
                    {
                        p.remove();
                    }
                    {
                        p.set(updatedInfo);
                    }
                    found = true;
                    break;
                }
            }
            if(!found && !destroyed)
            {
                _info.servers.add(updatedInfo);
            }
        }

        Server server = findServer(updatedInfo.id);
        if(server != null)
        {
            server.update(updatedInfo.state, updatedInfo.pid, updatedInfo.enabled, true);
        }
    }

    void updateAdapter(AdapterDynamicInfo updatedInfo)
    {
        if(_info != null)
        {
            java.util.ListIterator<AdapterDynamicInfo> p = _info.adapters.listIterator();
            boolean found = false;
            while(p.hasNext())
            {
                AdapterDynamicInfo ainfo = p.next();
                if(ainfo.id.equals(updatedInfo.id))
                {
                    p.set(updatedInfo);
                    found = true;
                    break;
                }
            }
            if(!found)
            {
                _info.adapters.add(updatedInfo);
            }
        }

        for(javax.swing.tree.TreeNode p : _children)
        {
            Server server = (Server)p;
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
            java.util.ListIterator<AdapterDynamicInfo> p = _info.adapters.listIterator();
            while(p.hasNext())
            {
                AdapterDynamicInfo ainfo = p.next();
                if(ainfo.id.equals(adapterId))
                {
                    return ainfo.proxy;
                }
            }
        }
        return null;
    }

    java.util.SortedMap<String, String> getLoadFactors()
    {
        java.util.SortedMap<String, String> result = new java.util.TreeMap<String, String>();

        for(java.util.Map.Entry<String, ApplicationData> p : _map.entrySet())
        {
            ApplicationData ad = p.getValue();

            String val = ad.resolver.substitute(ad.descriptor.loadFactor);
            if(val.length() == 0)
            {
                val = "Default";
            }

            result.put(p.getKey(), val);
        }
        return result;
    }

    void showLoad()
    {
        Callback_Admin_getNodeLoad cb = new Callback_Admin_getNodeLoad()
            {
                @Override
                public void response(LoadInfo loadInfo)
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
                            @Override
                            public void run()
                            {
                                _editor.setLoad(load, Node.this);
                            }
                        });
                }

                @Override
                public void exception(final Ice.UserException e)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            @Override
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

                @Override
                public void exception(final Ice.LocalException e)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            @Override
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
                admin.begin_getNodeLoad(_id, cb);
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

    private Server createServer(ApplicationDescriptor application, Utils.Resolver resolver,
                                ServerInstanceDescriptor instanceDescriptor)
    {
        //
        // Find template
        //
        TemplateDescriptor templateDescriptor =
            application.serverTemplates.get(instanceDescriptor.template);
        assert templateDescriptor != null;

        ServerDescriptor serverDescriptor = (ServerDescriptor)templateDescriptor.descriptor;

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
            for(ServerDynamicInfo info : _info.servers)
            {
                if(info.id.equals(serverId))
                {
                    serverState = info.state;
                    pid = info.pid;
                    enabled = info.enabled;
                    break;
                }
            }
        }

        //
        // Create server
        //
        return new Server(this, serverId, instanceResolver, instanceDescriptor, serverDescriptor, application,
                          serverState, pid, enabled);
    }

    private Server createServer(ApplicationDescriptor application, Utils.Resolver resolver,
                                ServerDescriptor serverDescriptor)
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
            for(ServerDynamicInfo info : _info.servers)
            {
                if(info.id.equals(serverId))
                {
                    serverState = info.state;
                    pid = info.pid;
                    enabled = info.enabled;
                    break;
                }
            }
        }

        //
        // Create server
        //
        return new Server(this, serverId, instanceResolver, null, serverDescriptor, application, serverState, pid,
                          enabled);
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
        ServerInstanceDescriptor instanceDescriptor = server.getInstanceDescriptor();
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
        java.util.Iterator<ServerDescriptor> p = nodeDesc.servers.iterator();
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
        java.util.Iterator<ServerInstanceDescriptor> p = nodeDesc.serverInstances.iterator();
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

    public java.util.List<Server>
    getServers()
    {
        java.util.List<Server> servers = new java.util.ArrayList<Server>();
        for(Object obj : _children)
        {
            servers.add((Server)obj);
        }
        return servers;
    }

    //
    // Application name to ApplicationData
    //
    private final java.util.SortedMap<String, ApplicationData> _map = new java.util.TreeMap<String, ApplicationData>();

    private boolean _up = false;
    private NodeDynamicInfo _info;
    private boolean _windows = false;
    
    private ShowIceLogDialog _showIceLogDialog;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUp;
    static private Icon _nodeDown;

    static private NodeEditor _editor;
    static private JPopupMenu _popup;
}
