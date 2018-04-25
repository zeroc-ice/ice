// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;

import java.text.NumberFormat;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

class Node extends Communicator
{
    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];
        actions[SHUTDOWN_NODE] = _up;
        actions[RETRIEVE_ICE_LOG] = _up;
        actions[RETRIEVE_STDOUT] = _up;
        actions[RETRIEVE_STDERR] = _up;
        actions[START_ALL_SERVERS] = _up;
        actions[STOP_ALL_SERVERS] = _up;
        return actions;
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

        try
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));

            getCoordinator().getAdmin().shutdownNodeAsync(_id).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, "Failed to shutdown " + _id, ex);
                });
        }
        catch(com.zeroc.Ice.LocalException e)
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
    public void startAllServers()
    {
        for(Server server : _servers)
        {
            if(server.getAvailableActions()[START])
            {
                server.start();
            }
        }
    }

    @Override
    public void stopAllServers()
    {
        for(Server server : _servers)
        {
            if(server.getAvailableActions()[STOP])
            {
                server.stop();
            }
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
            _popup.add(la.get(START_ALL_SERVERS));
            _popup.add(la.get(STOP_ALL_SERVERS));
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

    //
    // Implement Communicator abstract methods
    //

    @Override
    protected java.util.concurrent.CompletableFuture<com.zeroc.Ice.ObjectPrx> getAdminAsync()
    {
        return getRoot().getCoordinator().getAdmin().getNodeAdminAsync(_id);
    }

    @Override
    protected String getServerDisplayName()
    {
        return "Node " + _id;
    }

    @Override
    protected String getDisplayName()
    {
        return "Node " + _id;
    }

    @Override
    protected String getDefaultFileName()
    {
        return "node-" + _id;
    }

    Node(Root parent, NodeDynamicInfo info)
    {
        super(parent, info.info.name, 2);
        _childrenArray[0] = _metrics;
        _childrenArray[1] = _servers;
        up(info, false);
    }

    Node(Root parent, ApplicationDescriptor appDesc, String nodeName, NodeDescriptor nodeDesc)
    {
        super(parent, nodeName, 2);
        _childrenArray[0] = _metrics;
        _childrenArray[1] = _servers;
        add(appDesc, nodeDesc);
    }

    Node(Root parent, ApplicationDescriptor appDesc, NodeUpdateDescriptor update)
    {
        super(parent, update.name, 2);
        _childrenArray[0] = _metrics;
        _childrenArray[1] = _servers;

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

        java.util.List<Server> toRemove = new java.util.LinkedList<>();
        int[] toRemoveIndices = new int[_servers.size()];
        int i = 0;

        for(Server server : _servers)
        {
            if(server.getApplication().name.equals(appName))
            {
                toRemove.add(server);
                toRemoveIndices[i++] = getIndex(server);
            }
        }
        toRemoveIndices = resize(toRemoveIndices, toRemove.size());
        _servers.removeAll(toRemove);
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
        java.util.Set<Server> freshServers = new java.util.HashSet<>();

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
                    _servers.remove(server);
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
                    oldServer.rebuild(server);
                    oldServer.updateMetrics();
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
                    oldServer.rebuild(server);
                    oldServer.updateMetrics();
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
            for(Server server : _servers)
            {
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
            updated = resolver.put("node.data", _info.info.dataDir) || updated;
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

                for(Server server : _servers)
                {
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
        java.util.Set<Server> updatedServers = new java.util.HashSet<>();
        for(ServerDynamicInfo sinfo : _info.servers)
        {
            Server server = findServer(sinfo.id);
            if(server != null)
            {
                server.update(sinfo.state, sinfo.pid, sinfo.enabled, true);
                updatedServers.add(server);
            }
        }
        for(Server server : _servers)
        {
            if(!updatedServers.contains(server))
            {
                server.update(ServerState.Inactive, 0, true, true);
            }
        }

        //
        // Tell adapters
        //
        java.util.Iterator<Server> p = _servers.iterator();
        int updateCount = 0;
        while(p.hasNext() && updateCount < _info.adapters.size())
        {
            Server server = p.next();
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

        if(_servers.isEmpty())
        {
            return true;
        }
        else
        {
            for(Server server : _servers)
            {
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
                    else
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

        for(Server server : _servers)
        {
            if(server.updateAdapter(updatedInfo))
            {
                break;
            }
        }
    }

    com.zeroc.Ice.ObjectPrx getProxy(String adapterId)
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
        java.util.SortedMap<String, String> result = new java.util.TreeMap<>();

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
        com.zeroc.IceGrid.AdminPrx admin = getCoordinator().getAdmin();
        if(admin == null)
        {
            _editor.setLoad("Unknown", this);
        }
        else
        {
            _editor.setLoad("", this);
            final String prefix = "Retrieving load for " + getDisplayName() + "...";
            getRoot().getCoordinator().getStatusBar().setText(prefix);

            try
            {
                admin.getNodeLoadAsync(_id).whenComplete((result, ex) ->
                    {
                        if(ex == null)
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
                                format.format(result.avg1) + " " +
                                format.format(result.avg5) + " " +
                                format.format(result.avg15);

                            SwingUtilities.invokeLater(() ->
                                {
                                    success(prefix);
                                    _editor.setLoad(load, Node.this);
                                });
                        }
                        else
                        {
                            amiFailure(prefix, "Failed to retrieve load for " + getDisplayName(), ex);
                        }
                    });
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                getRoot().getCoordinator().getStatusBar().setText(prefix + " " + e.toString() + ".");
            }
        }
    }

    private Server createServer(ApplicationDescriptor application, Utils.Resolver resolver,
                                ServerInstanceDescriptor instanceDescriptor)
    {
        //
        // Find template
        //
        TemplateDescriptor templateDescriptor = application.serverTemplates.get(instanceDescriptor.template);
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
        insertSortedChild(server, _servers, getRoot().getTreeModel());
    }

    private Server findServer(String id)
    {
        return (Server)find(id, _servers);
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

    public java.util.List<Server> getServers()
    {
        return new java.util.ArrayList<Server>(_servers);
    }

    //
    // Application name to ApplicationData
    //
    private final java.util.SortedMap<String, ApplicationData> _map = new java.util.TreeMap<>();

    private boolean _up = false;
    private NodeDynamicInfo _info;
    private boolean _windows = false;

    private java.util.LinkedList<Server> _servers = new java.util.LinkedList<>();

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon _nodeUp;
    static private Icon _nodeDown;

    static private NodeEditor _editor;
    static private JPopupMenu _popup;
}
