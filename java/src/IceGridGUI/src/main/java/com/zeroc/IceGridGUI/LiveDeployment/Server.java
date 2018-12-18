// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package com.zeroc.IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JMenu;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

public class Server extends Communicator
{
    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];

        if(_state != null)
        {
            actions[START] = _state == ServerState.Inactive && _enabled
                && !_resolver.substitute(_serverDescriptor.activation).equals("session");

            actions[STOP] = _state != ServerState.Inactive;
            actions[ENABLE] = !_enabled;
            actions[DISABLE] = _enabled;
            actions[WRITE_MESSAGE] = _state == ServerState.Active;
            actions[RETRIEVE_ICE_LOG] = _state == ServerState.Active;
            actions[RETRIEVE_STDOUT] = true;
            actions[RETRIEVE_STDERR] = true;
            actions[RETRIEVE_LOG_FILE] = _serverDescriptor.logs.length > 0;

            actions[PATCH_SERVER] = !_serverDescriptor.distrib.icepatch.equals("");

            if(_state != ServerState.Inactive)
            {
                Node node = (Node)_parent;
                if(!node.isRunningWindows())
                {
                    for(int i = SIGHUP; i <= SIGTERM; ++i)
                    {
                        actions[i] = true;
                    }
                }
            }
        }

        actions[OPEN_DEFINITION] = true;

        return actions;
    }

    @Override
    public void start()
    {
        final String prefix = "Starting server '" + _id + "'...";
        final String errorTitle = "Failed to start " + _id;

        getCoordinator().getStatusBar().setText(prefix);
        try
        {
            final AdminPrx admin = getCoordinator().getAdmin();
            admin.startServerAsync(_id).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, errorTitle, ex);
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    @Override
    public void stop()
    {
        final String prefix = "Stopping server '" + _id + "'...";
        final String errorTitle = "Failed to stop " + _id;
        getCoordinator().getStatusBar().setText(prefix);
        try
        {
            final AdminPrx admin = getCoordinator().getAdmin();
            admin.stopServerAsync(_id).whenComplete((result, ex) ->
                {
                    if(ex == null)
                    {
                        amiSuccess(prefix);
                        SwingUtilities.invokeLater(() -> rebuild(Server.this));
                    }
                    else if(ex instanceof com.zeroc.Ice.UserException)
                    {
                        amiFailure(prefix, errorTitle, (com.zeroc.Ice.UserException)ex);
                    }
                    else
                    {
                        amiFailure(prefix, errorTitle, ex.toString());
                    }
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    @Override
    public void enable()
    {
        enableServer(true);
    }

    @Override
    public void disable()
    {
        enableServer(false);
    }

    @Override
    public void writeMessage()
    {
        if(_writeMessageDialog == null)
        {
            _writeMessageDialog = new WriteMessageDialog(getRoot());
        }
        _writeMessageDialog.showDialog(_id);
    }

    @Override
    public void retrieveOutput(final boolean stdout)
    {
        getRoot().openShowLogFileDialog(new ShowLogFileDialog.FileIteratorFactory()
            {
                @Override
                public FileIteratorPrx open(int count)
                    throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException,
                           DeploymentException
                {
                    AdminSessionPrx session = getRoot().getCoordinator().getSession();
                    FileIteratorPrx result;
                    if(stdout)
                    {
                        result = session.openServerStdOut(_id, count);
                    }
                    else
                    {
                        result = session.openServerStdErr(_id, count);
                    }
                    return result;
                }

                @Override
                public String getTitle()
                {
                    return "Server " + _id + " " + (stdout ? "stdout" : "stderr");
                }

                @Override
                public String getDefaultFilename()
                {
                    return _id + (stdout ? ".out" : ".err");
                }
            });
    }

    @Override
    public void retrieveLogFile()
    {
        assert _serverDescriptor.logs.length > 0;

        String path = null;

        if(_serverDescriptor.logs.length == 1)
        {
            path = _resolver.substitute(_serverDescriptor.logs[0]);
        }
        else
        {
            Object[] pathArray = new Object[_serverDescriptor.logs.length];
            int i = 0;
            for(String log : _serverDescriptor.logs)
            {
                pathArray[i++] = _resolver.substitute(log);
            }

            path = (String)JOptionPane.showInputDialog(
                getCoordinator().getMainFrame(),
                "Which log file do you want to retrieve?",
                "Retrieve Log File",
                JOptionPane.QUESTION_MESSAGE, null,
                pathArray, pathArray[0]);
        }

        if(path != null)
        {
            final String fPath = path;

            getRoot().openShowLogFileDialog(new ShowLogFileDialog.FileIteratorFactory()
                {
                    @Override
                    public FileIteratorPrx open(int count)
                        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException,
                        DeploymentException
                    {
                        AdminSessionPrx session = getRoot().getCoordinator().getSession();
                        return session.openServerLog(_id, fPath, count);
                    }

                    @Override
                    public String getTitle()
                    {
                        return "Server " + _id + " " + new java.io.File(fPath).getName();
                    }

                    @Override
                    public String getDefaultFilename()
                    {
                        return new java.io.File(fPath).getName();
                    }
                });
        }
    }

    @Override
    public void signal(final String s)
    {
        final String prefix = "Sending '" + s + "' to server '" + _id + "'...";
        final String errorTitle = "Failed to deliver signal " + s + " to " + _id;
        getCoordinator().getStatusBar().setText(prefix);
        try
        {
            final AdminPrx admin = getCoordinator().getAdmin();
            admin.sendSignalAsync(_id, s).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, errorTitle, ex);
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    @Override
    public void patchServer()
    {
        String message = _serverDescriptor.applicationDistrib ?
            "You are about to install or refresh your"
            + " server distribution and your application distribution onto this node.\n"
            + "Do you want shut down all servers affected by this update?" :
            "You are about to install or refresh the distribution for this server.\n"
            + "Do you want to shut down the server for this update?";

        int shutdown = JOptionPane.showConfirmDialog(
            getCoordinator().getMainFrame(),
            message,
            "Patch Confirmation",
            JOptionPane.YES_NO_CANCEL_OPTION);

        if(shutdown == JOptionPane.CANCEL_OPTION)
        {
            return;
        }

        final String prefix = "Patching server '" + _id + "'...";
        final String errorTitle = "Failed to patch " + _id;

        getCoordinator().getStatusBar().setText(prefix);
        try
        {
            final AdminPrx admin = getCoordinator().getAdmin();
            admin.patchServerAsync(_id, shutdown == JOptionPane.YES_OPTION).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, errorTitle, ex);
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    private void enableServer(boolean enable)
    {
        final String prefix = (enable ?  "Enabling" : "Disabling") + " server '" + _id + "'...";
        final String errorTitle = "Failed to " + (enable ? "enable" : "disable") + " " + _id;
        getCoordinator().getStatusBar().setText(prefix);
        try
        {
            final AdminPrx admin = getCoordinator().getAdmin();
            admin.enableServerAsync(_id, enable).whenComplete((result, ex) ->
                {
                    amiComplete(prefix, errorTitle, ex);
                });
        }
        catch(com.zeroc.Ice.LocalException ex)
        {
            failure(prefix, errorTitle, ex.toString());
        }
    }

    @Override
    public void openDefinition()
    {
        ApplicationPane app = getCoordinator().openLiveApplication(_application.name);
        if(app != null)
        {
            app.getRoot().selectServer(((Node)_parent).getId(), _id);
        }
    }

    @Override
    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();

            _popup.add(la.get(START));
            _popup.add(la.get(STOP));
            _popup.addSeparator();
            _popup.add(la.get(ENABLE));
            _popup.add(la.get(DISABLE));
            _popup.addSeparator();
            _popup.add(la.get(PATCH_SERVER));
            _popup.addSeparator();
            _popup.add(la.get(WRITE_MESSAGE));
            _popup.add(la.get(RETRIEVE_ICE_LOG));
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.add(la.get(RETRIEVE_LOG_FILE));
            _popup.addSeparator();

            _signalMenu = new JMenu("Send Signal");
            _popup.add(_signalMenu);

            _signalMenu.add(la.get(SIGHUP));
            _signalMenu.add(la.get(SIGINT));
            _signalMenu.add(la.get(SIGQUIT));
            _signalMenu.add(la.get(SIGKILL));
            _signalMenu.add(la.get(SIGUSR1));
            _signalMenu.add(la.get(SIGUSR2));
            _signalMenu.add(la.get(SIGTERM));
            _popup.addSeparator();
            _popup.add(la.get(OPEN_DEFINITION));
        }

        la.setTarget(this);
        _signalMenu.setEnabled(la.get(SIGHUP).isEnabled());
        return _popup;
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new ServerEditor(getCoordinator());
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

            //
            // Regular servers
            //
            _icons = new Icon[8][2][2];
            _icons[0][0][0] = Utils.getIcon("/icons/16x16/server_unknown.png");
            _icons[ServerState.Inactive.value() + 1][0][0] = Utils.getIcon("/icons/16x16/server_inactive.png");
            _icons[ServerState.Activating.value() + 1][0][0] = Utils.getIcon("/icons/16x16/server_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][0][0] =
                Utils.getIcon("/icons/16x16/server_activating.png");
            _icons[ServerState.Active.value() + 1][0][0] = Utils.getIcon("/icons/16x16/server_active.png");
            _icons[ServerState.Deactivating.value() + 1][0][0] =
                Utils.getIcon("/icons/16x16/server_deactivating.png");
            _icons[ServerState.Destroying.value() + 1][0][0] = Utils.getIcon("/icons/16x16/server_destroying.png");
            _icons[ServerState.Destroyed.value() + 1][0][0] = Utils.getIcon("/icons/16x16/server_destroyed.png");

            //
            // IceBox servers
            //
            _icons[0][1][0] = Utils.getIcon("/icons/16x16/icebox_server_unknown.png");
            _icons[ServerState.Inactive.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_inactive.png");
            _icons[ServerState.Activating.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_activating.png");
            _icons[ServerState.Active.value() + 1][1][0] = Utils.getIcon("/icons/16x16/icebox_server_active.png");
            _icons[ServerState.Deactivating.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_deactivating.png");
            _icons[ServerState.Destroying.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_destroying.png");
            _icons[ServerState.Destroyed.value() + 1][1][0] =
                Utils.getIcon("/icons/16x16/icebox_server_destroyed.png");

            //
            // Regular servers (disabled)
            //
            _icons[0][0][1] = Utils.getIcon("/icons/16x16/server_unknown.png");
            _icons[ServerState.Inactive.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_inactive.png");
            _icons[ServerState.Activating.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_activating.png");
            _icons[ServerState.Active.value() + 1][0][1] = Utils.getIcon("/icons/16x16/server_disabled_active.png");
            _icons[ServerState.Deactivating.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_deactivating.png");
            _icons[ServerState.Destroying.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_destroying.png");
            _icons[ServerState.Destroyed.value() + 1][0][1] =
                Utils.getIcon("/icons/16x16/server_disabled_destroyed.png");

            //
            // IceBox servers (disabled)
            //
            _icons[0][1][1] = Utils.getIcon("/icons/16x16/icebox_server_unknown.png");
            _icons[ServerState.Inactive.value() + 1][1][1]
                = Utils.getIcon("/icons/16x16/icebox_server_disabled_inactive.png");
            _icons[ServerState.Activating.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_activating.png");
            _icons[ServerState.Active.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_active.png");
            _icons[ServerState.Deactivating.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_deactivating.png");
            _icons[ServerState.Destroying.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_destroying.png");
            _icons[ServerState.Destroyed.value() + 1][1][1] =
                Utils.getIcon("/icons/16x16/icebox_server_disabled_destroyed.png");
        }

        int icebox = _serverDescriptor instanceof IceBoxDescriptor ? 1 : 0;
        int disabled = _enabled ? 0 : 1;

        if(expanded)
        {
            _cellRenderer.setOpenIcon(_icons[_stateIconIndex][icebox][disabled]);
        }
        else
        {
            _cellRenderer.setClosedIcon(_icons[_stateIconIndex][icebox][disabled]);
        }

        _cellRenderer.setToolTipText(_toolTip);
        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    Server(Node parent, String serverId, Utils.Resolver resolver, ServerInstanceDescriptor instanceDescriptor,
           ServerDescriptor serverDescriptor, ApplicationDescriptor application, ServerState state, int pid,
           boolean enabled)
    {
        super(parent, serverId, 4);
        _resolver = resolver;

        _instanceDescriptor = instanceDescriptor;
        _serverDescriptor = serverDescriptor;
        _application = application;

        _childrenArray[0] = _metrics;
        _childrenArray[1] = _adapters;
        _childrenArray[2] = _dbEnvs;
        _childrenArray[3] = _services;

        update(state, pid, enabled, false);

        createAdapters();
        createDbEnvs();
        createServices();
    }

    ApplicationDescriptor getApplication()
    {
        return _application;
    }

    ServerInstanceDescriptor getInstanceDescriptor()
    {
        return _instanceDescriptor;
    }

    ServerDescriptor getServerDescriptor()
    {
        return _serverDescriptor;
    }

    ServerState getState()
    {
        return _state;
    }

    boolean hasServiceObserver()
    {
        return _serviceObserver != null;
    }

    int getPid()
    {
        return _pid;
    }

    boolean isEnabled()
    {
        return _enabled;
    }

    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    void removeCallbacks()
    {
        if(_serviceObserver != null)
        {
            getCoordinator().removeCallback(_serviceObserver.ice_getIdentity().name, _serviceObserver.ice_getFacet());
            _serviceObserver = null;
        }
    }

    void updateServices()
    {
        for(Service service: _services)
        {
            if(_startedServices.contains(service.getId()))
            {
                service.started();
            }
            else
            {
                service.stopped();
            }
        }
    }

    void rebuild(Server server)
    {
        _resolver = server._resolver;
        _instanceDescriptor = server._instanceDescriptor;
        _serverDescriptor = server._serverDescriptor;
        _application = server._application;

        _metrics = server._metrics;
        _adapters = server._adapters;
        _dbEnvs = server._dbEnvs;
        _services = server._services;

        _childrenArray[0] = _metrics;
        _childrenArray[1] = _adapters;
        _childrenArray[2] = _dbEnvs;
        _childrenArray[3] = _services;

        //
        // Need to re-parent all the children
        //
        for(Adapter adapter: _adapters)
        {
            adapter.reparent(this);
        }

        for(DbEnv dbEnv: _dbEnvs)
        {
            dbEnv.reparent(this);
        }

        for(Service service: _services)
        {
            service.reparent(this);
        }

        for(MetricsView metrics: _metrics)
        {
            metrics.reparent(this);
        }

        updateServices();

        getRoot().getTreeModel().nodeStructureChanged(this);
    }

    void rebuild(Utils.Resolver resolver, boolean variablesChanged, java.util.Set<String> serviceTemplates,
                 java.util.Set<String> serverTemplates)
    {
        if(variablesChanged ||
           (_instanceDescriptor != null && serverTemplates != null &&
            serverTemplates.contains(_instanceDescriptor.template)))
        {
            if(_instanceDescriptor != null)
            {
                TemplateDescriptor templateDescriptor = _application.serverTemplates.get(_instanceDescriptor.template);
                assert templateDescriptor != null;

                _resolver.reset(resolver, _instanceDescriptor.parameterValues, templateDescriptor.parameterDefaults);
                _resolver.put("server", _id);
                _serverDescriptor = (ServerDescriptor)templateDescriptor.descriptor;
            }
            else
            {
                _resolver.reset(resolver);
                _resolver.put("server", _id);
            }

            _adapters.clear();
            createAdapters();
            _dbEnvs.clear();
            createDbEnvs();
            _services.clear();
            _servicePropertySets.clear();
            createServices();
            updateServices();

            _metrics.clear();

            getRoot().getTreeModel().nodeStructureChanged(this);
            updateMetrics();
        }
        else if(serviceTemplates != null && serviceTemplates.size() > 0 &&
                _serverDescriptor instanceof IceBoxDescriptor)
        {
            _metrics.clear();
            _services.clear();
            _servicePropertySets.clear();
            createServices();
            updateServices();

            getRoot().getTreeModel().nodeStructureChanged(this);
            updateMetrics();
        }
    }

    void update(ServerState state, int pid, boolean enabled, boolean fireEvent)
    {
        if(state != _state || pid != _pid || enabled != _enabled)
        {
            _state = state;
            _pid = pid;
            _enabled = enabled;

            _toolTip = toolTip(_state, _pid, _enabled);
            if(_state == null)
            {
                _stateIconIndex = 0;
            }
            else
            {
                _stateIconIndex = _state.value() + 1;
            }

            if(_state == ServerState.Active && getRoot().getTree().isExpanded(getPath()))
            {
                fetchMetricsViewNames();
            }
            else
            {
                _metricsRetrieved = false;
                if(!_metrics.isEmpty())
                {
                    _metrics.clear();
                    rebuild(this);
                }
            }

            if(_state == ServerState.Inactive)
            {
                if(_showIceLogDialog != null)
                {
                    _showIceLogDialog.stopped();
                }
            }

            if(_serverDescriptor instanceof IceBoxDescriptor)
            {
                if(_state == ServerState.Active)
                {
                    if(_serviceObserver == null)
                    {
                        _serviceObserver = com.zeroc.IceBox.ServiceObserverPrx.uncheckedCast(
                            getCoordinator().retrieveCallback(_id, "IceBox.ServiceManager"));

                        if(_serviceObserver == null)
                        {
                            com.zeroc.IceBox.ServiceObserver servant = new com.zeroc.IceBox.ServiceObserver()
                                {
                                    @Override
                                    public void servicesStarted(final String[] services, com.zeroc.Ice.Current current)
                                    {
                                        final java.util.Set<String> serviceSet =
                                            new java.util.HashSet<>(java.util.Arrays.asList(services));

                                        SwingUtilities.invokeLater(() ->
                                            {
                                                for(Service service: _services)
                                                {
                                                    if(serviceSet.contains(service.getId()))
                                                    {
                                                        service.started();
                                                    }
                                                }
                                                _startedServices.addAll(serviceSet);
                                                getCoordinator().getLiveDeploymentPane().refresh();
                                            });
                                    }

                                    @Override
                                    public void servicesStopped(final String[] services, com.zeroc.Ice.Current current)
                                    {
                                        final java.util.Set<String> serviceSet =
                                            new java.util.HashSet<>(java.util.Arrays.asList(services));

                                        SwingUtilities.invokeLater(() ->
                                            {
                                                for(Service service: _services)
                                                {
                                                    if(serviceSet.contains(service.getId()))
                                                    {
                                                        service.stopped();
                                                    }
                                                }
                                                _startedServices.removeAll(serviceSet);
                                                getCoordinator().getLiveDeploymentPane().refresh();
                                            });
                                    }

                                };

                            _serviceObserver = com.zeroc.IceBox.ServiceObserverPrx.uncheckedCast(
                                getCoordinator().addCallback(servant, _id, "IceBox.ServiceManager"));

                            if(_serviceObserver == null)
                            {
                                JOptionPane.showMessageDialog(
                                    getCoordinator().getMainFrame(),
                                    "Could not create servant for service-manager observer",
                                    "Observer creation error",
                                    JOptionPane.ERROR_MESSAGE);
                            }
                        }
                    }

                    if(_serviceObserver != null)
                    {
                        //
                        // Add observer to service manager using AMI call
                        // Note that duplicate registrations are ignored
                        //

                        com.zeroc.IceBox.ServiceManagerPrx serviceManager =
                            com.zeroc.IceBox.ServiceManagerPrx.uncheckedCast(getAdminFacet("IceBox.ServiceManager"));

                        if(serviceManager != null)
                        {
                            try
                            {
                                serviceManager.addObserverAsync(_serviceObserver).whenComplete((result, ex) ->
                                    {
                                        if(ex != null)
                                        {
                                            JOptionPane.showMessageDialog(
                                                getCoordinator().getMainFrame(),
                                                "Failed to register service-manager observer: " + ex.toString(),
                                                "Observer registration error",
                                                JOptionPane.ERROR_MESSAGE);
                                        }
                                    });
                            }
                            catch(com.zeroc.Ice.LocalException ex)
                            {
                                JOptionPane.showMessageDialog(
                                    getCoordinator().getMainFrame(),
                                    "Failed to contact service-manager: " + ex.toString(),
                                    "Observer communication error",
                                    JOptionPane.ERROR_MESSAGE);
                            }
                        }
                    }
                }
                else if(_state == ServerState.Inactive)
                {
                    for(Service service: _services)
                    {
                        service.stopShowIceLogDialog();
                        service.stopped();
                    }
                }
            }

            if(fireEvent)
            {
                getRoot().getTreeModel().nodeChanged(this);
            }
        }
    }

    boolean updateAdapter(AdapterDynamicInfo info)
    {
        for(Adapter p : _adapters)
        {
            if(p.update(info))
            {
                return true;
            }
        }

        //
        // Could be in one of the services as well
        //
        for(Service p : _services)
        {
            if(p.updateAdapter(info))
            {
                return true;
            }
        }

        return false;
    }

    int updateAdapters(java.util.List<AdapterDynamicInfo> infoList)
    {
        int result = 0;
        {
            java.util.Iterator<Adapter> p = _adapters.iterator();
            while(p.hasNext() && result < infoList.size())
            {
                Adapter adapter = p.next();
                if(adapter.update(infoList))
                {
                    result++;
                }
            }
        }

        //
        // Could be in one of the services as well
        //
        {
            java.util.Iterator<Service> p = _services.iterator();
            while(p.hasNext() && result < infoList.size())
            {
                Service service = p.next();
                result += service.updateAdapters(infoList);
            }
        }

        return result;
    }

    void nodeDown()
    {
        update(null, 0, true, true);

        for(Adapter p : _adapters)
        {
            p.update((AdapterDynamicInfo)null);
        }

        for(Service p : _services)
        {
            p.nodeDown();
        }
    }

    java.util.SortedMap<String, String> getProperties()
    {
        java.util.List<Utils.ExpandedPropertySet> psList = new java.util.LinkedList<>();
        Node node = (Node)_parent;

        psList.add(node.expand(_serverDescriptor.propertySet, _application.name, _resolver));

        if(_instanceDescriptor != null)
        {
            psList.add(node.expand(_instanceDescriptor.propertySet, _application.name, _resolver));
        }

        return Utils.propertySetsToMap(psList, _resolver);
    }

    int getIceVersion()
    {
        return Utils.getIntVersion(Utils.substitute(_serverDescriptor.iceVersion, _resolver));
    }

    private void createAdapters()
    {
        for(AdapterDescriptor p : _serverDescriptor.adapters)
        {
            String adapterName = Utils.substitute(p.name, _resolver);
            String adapterId = Utils.substitute(p.id, _resolver);
            com.zeroc.Ice.ObjectPrx proxy = null;
            if(adapterId.length() > 0)
            {
                proxy = ((Node)_parent).getProxy(adapterId);
            }

            insertSortedChild(new Adapter(this, adapterName, _resolver, adapterId, p, proxy), _adapters, null);
        }
    }

    private void createDbEnvs()
    {
        for(DbEnvDescriptor p : _serverDescriptor.dbEnvs)
        {
            String dbEnvName = Utils.substitute(p.name, _resolver);
            insertSortedChild(new DbEnv(this, dbEnvName, _resolver, p), _dbEnvs, null);
        }
    }

    private void createServices()
    {
        if(_serverDescriptor instanceof IceBoxDescriptor)
        {
            if(_instanceDescriptor != null)
            {
                for(java.util.Map.Entry<String, PropertySetDescriptor> p :
                    _instanceDescriptor.servicePropertySets.entrySet())
                {
                    _servicePropertySets.put(_resolver.substitute(p.getKey()), p.getValue());
                }
            }

            IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)_serverDescriptor;

            for(ServiceInstanceDescriptor p : iceBoxDescriptor.services)
            {
                createService(p);
            }
        }
    }

    private void createService(ServiceInstanceDescriptor descriptor)
    {
        ServiceDescriptor serviceDescriptor = null;
        String serviceName = null;
        Utils.Resolver serviceResolver = null;

        if(descriptor.template.length() > 0)
        {
            TemplateDescriptor templateDescriptor = _application.serviceTemplates.get(descriptor.template);

            assert templateDescriptor != null;

            serviceDescriptor = (ServiceDescriptor)templateDescriptor.descriptor;
            assert serviceDescriptor != null;

            serviceResolver = new Utils.Resolver(_resolver,
                                                 descriptor.parameterValues,
                                                 templateDescriptor.parameterDefaults);
            serviceName = serviceResolver.substitute(serviceDescriptor.name);
            serviceResolver.put("service", serviceName);
        }
        else
        {
            serviceDescriptor = descriptor.descriptor;
            assert serviceDescriptor != null;

            serviceResolver = new Utils.Resolver(_resolver);
            serviceName = _resolver.substitute(serviceDescriptor.name);
            serviceResolver.put("service", serviceName);
        }

        PropertySetDescriptor serverInstancePSDescriptor = _servicePropertySets.get(serviceName);

        _services.add(new Service(this, serviceName, serviceResolver, descriptor, serviceDescriptor,
                                  serverInstancePSDescriptor));
    }

    //
    // Implement Communicator abstract methods
    //

    @Override
    protected java.util.concurrent.CompletableFuture<com.zeroc.Ice.ObjectPrx> getAdminAsync()
    {
        return java.util.concurrent.CompletableFuture.completedFuture(getAdmin());
    }

    @Override
    protected String getDisplayName()
    {
        return "Server " + _id;
    }

    @Override
    protected String getDefaultFileName()
    {
        return _id;
    }

    com.zeroc.Ice.ObjectPrx getAdmin()
    {
        if(_state == ServerState.Active)
        {
            AdminPrx gridAdmin = getCoordinator().getAdmin();
            if(gridAdmin != null)
            {
                return gridAdmin.ice_identity(new com.zeroc.Ice.Identity(_id, getCoordinator().getServerAdminCategory()));
            }
        }
        return null;
    }

    com.zeroc.Ice.ObjectPrx getAdminFacet(String facet)
    {
        com.zeroc.Ice.ObjectPrx admin = getAdmin();
        if(admin != null)
        {
            return admin.ice_facet(facet);
        }
        else
        {
            return null;
        }
    }

    static private String toolTip(ServerState state, int pid, boolean enabled)
    {
        String result = (state == null ? "Unknown" : state.toString());

        if(!enabled)
        {
            result += ", disabled";
        }

        if(pid != 0)
        {
            result += ", pid: " + pid;
        }
        return result;
    }

    private ServerInstanceDescriptor _instanceDescriptor;
    private java.util.Map<String, PropertySetDescriptor> _servicePropertySets =
        new java.util.HashMap<>(); // with substituted names!

    private ServerDescriptor _serverDescriptor;
    private ApplicationDescriptor _application;

    private Utils.Resolver _resolver;
    private java.util.List<Adapter> _adapters = new java.util.LinkedList<>();
    private java.util.List<DbEnv> _dbEnvs = new java.util.LinkedList<>();
    private java.util.List<Service> _services = new java.util.LinkedList<>();

    private java.util.Set<String> _startedServices = new java.util.HashSet<>();

    private ServerState _state;
    private boolean _enabled;
    private int _stateIconIndex;
    private int _pid;
    private String _toolTip;

    private com.zeroc.IceBox.ServiceObserverPrx _serviceObserver;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[][][] _icons;

    static private ServerEditor _editor;
    static private JPopupMenu _popup;
    static private JMenu _signalMenu;
    static private WriteMessageDialog _writeMessageDialog;
}
