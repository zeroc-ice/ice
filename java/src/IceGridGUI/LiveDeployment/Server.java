// **********************************************************************
//
// Copyright (c) 2003-2013 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceGridGUI.LiveDeployment;

import java.awt.Component;
import java.awt.Cursor;

import javax.swing.Icon;
import javax.swing.JMenu;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

public class Server extends ListArrayTreeNode
{
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];

        if(_state != null)
        {
            actions[START] = _state == ServerState.Inactive && _enabled
                && !_resolver.substitute(_serverDescriptor.activation).equals("session");

            actions[STOP] = _state != ServerState.Inactive;
            actions[ENABLE] = !_enabled;
            actions[DISABLE] = _enabled;
            actions[WRITE_MESSAGE] = _state != ServerState.Inactive;
            actions[RETRIEVE_STDOUT] = true;
            actions[RETRIEVE_STDERR] = true;
            actions[RETRIEVE_LOG] = _serverDescriptor.logs.length > 0;

            actions[PATCH_SERVER] =
                !_serverDescriptor.distrib.icepatch.equals("");

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

    public void start()
    {
        final String prefix = "Starting server '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_startServer cb = new Callback_Admin_startServer()
            {
                //
                // Called by another thread!
                //
                public void response()
                {
                    amiSuccess(prefix);
                }

                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to start " + _id, e);
                }

                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to start " + _id, e.toString());
                }
            };

        try
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().begin_startServer(_id, cb);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to start " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    public void stop()
    {
        final String prefix = "Stopping server '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_stopServer cb = new Callback_Admin_stopServer()
            {
                //
                // Called by another thread!
                //
                public void response()
                {
                    amiSuccess(prefix);
                    rebuild(Server.this, false);
                }

                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to stop " + _id, e);
                }

                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to stop " + _id, e.toString());
                }
            };

        try
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().begin_stopServer(_id, cb);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to stop " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    public void enable()
    {
        enableServer(true);
    }

    public void disable()
    {
        enableServer(false);
    }

    public void writeMessage()
    {
        if(_writeMessageDialog == null)
        {
            _writeMessageDialog = new WriteMessageDialog(getRoot());
        }
        _writeMessageDialog.showDialog(_id);
    }

    public void retrieveOutput(final boolean stdout)
    {
        getRoot().openShowLogDialog(new ShowLogDialog.FileIteratorFactory()
            {
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

                public String getTitle()
                {
                    return "Server " + _id + " " + (stdout ? "stdout" : "stderr");
                }

                public String getDefaultFilename()
                {
                    return _id + (stdout ? ".out" : ".err");
                }
            });
    }

    public void retrieveLog()
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

            path  = (String)JOptionPane.showInputDialog(
                getCoordinator().getMainFrame(),
                "Which log file do you want to retrieve?",
                "Retrieve Log File",
                JOptionPane.QUESTION_MESSAGE, null,
                pathArray, pathArray[0]);
        }

        if(path != null)
        {
            final String fPath = path;

            getRoot().openShowLogDialog(new ShowLogDialog.FileIteratorFactory()
                {
                    public FileIteratorPrx open(int count)
                        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException,
                        DeploymentException
                    {
                        AdminSessionPrx session = getRoot().getCoordinator().getSession();
                        return session.openServerLog(_id, fPath, count);
                    }

                    public String getTitle()
                    {
                        return "Server " + _id + " " + new java.io.File(fPath).getName();
                    }

                    public String getDefaultFilename()
                    {
                        return new java.io.File(fPath).getName();
                    }
                });
        }
    }

    public void signal(final String s)
    {
        final String prefix = "Sending '" + s + "' to server '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_sendSignal cb = new Callback_Admin_sendSignal()
            {
                //
                // Called by another thread!
                //
                public void response()
                {
                    amiSuccess(prefix);
                }

                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to deliver signal " + s + " to " + _id, e);
                }

                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to deliver signal " + s + " to " + _id, e.toString());
                }
            };

        try
        {
            getCoordinator().getAdmin().begin_sendSignal(_id, s, cb);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to deliver signal " + s + " to " + _id, e.toString());
        }
    }

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
        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_patchServer cb = new Callback_Admin_patchServer()
            {
                //
                // Called by another thread!
                //
                public void response()
                {
                    amiSuccess(prefix);
                }

                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to patch " + _id, e);
                }

                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to patch " + _id, e.toString());
                }
            };

        try
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().begin_patchServer(_id, shutdown == JOptionPane.YES_OPTION, cb);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to patch " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    private void enableServer(boolean enable)
    {
        final String prefix = (enable ?  "Enabling" : "Disabling") + " server '" + _id + "'...";

        final String action = enable ? "enable" : "disable";

        getCoordinator().getStatusBar().setText(prefix);

        Callback_Admin_enableServer cb = new Callback_Admin_enableServer()
            {
                //
                // Called by another thread!
                //
                public void response()
                {
                    amiSuccess(prefix);
                }

                public void exception(Ice.UserException e)
                {
                    amiFailure(prefix, "Failed to " + action + " " + _id, e);
                }

                public void exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to " + action + " " + _id, e.toString());
                }
            };

        try
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().begin_enableServer(_id, enable, cb);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to " + action + " " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    public void fetchMetricsViewNames()
    {
        if(_metricsRetrieved)
        {
            return; // Already loaded.
        }

        Ice.ObjectPrx admin = getServerAdmin();
        if(admin == null)
        {
            return;
        }
        _metricsRetrieved = true;
        final IceMX.MetricsAdminPrx metricsAdmin =
                IceMX.MetricsAdminPrxHelper.uncheckedCast(admin.ice_facet("Metrics"));
        IceMX.Callback_MetricsAdmin_getMetricsViewNames cb = new IceMX.Callback_MetricsAdmin_getMetricsViewNames()
            {
                public void response(final String[] enabledViews, final String[] disabledViews)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            public void run()
                            {
                                for(String name : enabledViews)
                                {
                                    insertSortedChild(new MetricsView(Server.this, name, metricsAdmin, true), _metrics, null);
                                }
                                for(String name : disabledViews)
                                {
                                    insertSortedChild(new MetricsView(Server.this, name, metricsAdmin, false), _metrics, null);
                                }
                                rebuild(Server.this, false);
                            }
                        });
                }

                public void exception(final Ice.LocalException e)
                {
                    SwingUtilities.invokeLater(new Runnable()
                        {
                            public void run()
                            {
                                _metricsRetrieved = false;
                                if(e instanceof Ice.ObjectNotExistException)
                                {
                                    // Server is down.
                                }
                                else if(e instanceof Ice.FacetNotExistException)
                                {
                                    // MetricsAdmin facet not present. Old server version?
                                }
                                else
                                {
                                    e.printStackTrace();
                                    JOptionPane.showMessageDialog(getCoordinator().getMainFrame(),
                                                                  "Error: " + e.toString(), "Error",
                                                                  JOptionPane.ERROR_MESSAGE);
                                }
                            }
                        });
                }
            };
        try
        {
            metricsAdmin.begin_getMetricsViewNames(cb);
        }
        catch(Ice.LocalException e)
        {
            _metricsRetrieved = false;
            JOptionPane.showMessageDialog(getCoordinator().getMainFrame(), "Error: " + e.toString(), "Error",
                                          JOptionPane.ERROR_MESSAGE);
        }
    }

    void showRuntimeProperties()
    {
        Ice.ObjectPrx serverAdmin = getServerAdmin();

        if(serverAdmin == null)
        {
            _editor.setBuildId("", this);
        }
        else
        {
            Ice.Callback_PropertiesAdmin_getPropertiesForPrefix cb = new Ice.Callback_PropertiesAdmin_getPropertiesForPrefix()
                {
                    public void response(final java.util.Map<String, String> properties)
                    {
                        SwingUtilities.invokeLater(new Runnable()
                            {
                                public void run()
                                {
                                    _editor.setRuntimeProperties((java.util.SortedMap<String, String>)properties,
                                                                 Server.this);
                                }
                            });
                    }

                    public void exception(final Ice.LocalException e)
                    {
                        SwingUtilities.invokeLater(new Runnable()
                            {
                                public void run()
                                {
                                    if(e instanceof Ice.ObjectNotExistException)
                                    {
                                        _editor.setBuildId("Error: can't reach this server's Admin object",
                                                           Server.this);
                                    }
                                    else if(e instanceof Ice.FacetNotExistException)
                                    {
                                        _editor.setBuildId("Error: this server's Admin object does not provide a " +
                                                           "'Properties' facet", Server.this);
                                    }
                                    else
                                    {
                                        e.printStackTrace();
                                        _editor.setBuildId("Error: " + e.toString(), Server.this);
                                    }
                                }
                            });
                    }
                };


            try
            {
                Ice.PropertiesAdminPrx propAdmin =
                    Ice.PropertiesAdminPrxHelper.uncheckedCast(serverAdmin.ice_facet("Properties"));
                propAdmin.begin_getPropertiesForPrefix("", cb);
            }
            catch(Ice.LocalException e)
            {
                _editor.setBuildId("Error: " + e.toString(), this);
            }
        }
    }

    public void openDefinition()
    {
        ApplicationPane app = getCoordinator().openLiveApplication(_application.name);
        if(app != null)
        {
            app.getRoot().selectServer(((Node)_parent).getId(), _id);
        }
    }

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
            _popup.add(la.get(RETRIEVE_STDOUT));
            _popup.add(la.get(RETRIEVE_STDERR));
            _popup.add(la.get(RETRIEVE_LOG));
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

    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new ServerEditor(getCoordinator());
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

        _childrenArray[0] = _adapters;
        _childrenArray[1] = _dbEnvs;
        _childrenArray[2] = _services;
        _childrenArray[3] = _metrics;

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

    void updateMetrics()
    {
        _metricsRetrieved = false;
        if(getRoot().getTree().isExpanded(getPath()))
        {
            fetchMetricsViewNames();
        }
    }

    void rebuild(Server server, boolean fetchMetrics)
    {
        _resolver = server._resolver;
        _instanceDescriptor = server._instanceDescriptor;
        _serverDescriptor = server._serverDescriptor;
        _application = server._application;

        _adapters = server._adapters;
        _dbEnvs = server._dbEnvs;
        _services = server._services;
        _metrics = server._metrics;

        _childrenArray[0] = _adapters;
        _childrenArray[1] = _dbEnvs;
        _childrenArray[2] = _services;
        _childrenArray[3] = _metrics;

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

        if(fetchMetrics)
        {
            updateMetrics();
        }
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
                    rebuild(this, false);
                }
            }

            if(_serverDescriptor instanceof IceBoxDescriptor)
            {
                if(_state == ServerState.Active)
                {
                    if(_serviceObserver == null)
                    {
                        _serviceObserver = IceBox.ServiceObserverPrxHelper.uncheckedCast(
                            getCoordinator().retrieveCallback(_id, "IceBox.ServiceManager"));

                        if(_serviceObserver == null)
                        {
                            IceBox.ServiceObserver servant = new IceBox._ServiceObserverDisp()
                                {
                                    public void servicesStarted(final String[] services, Ice.Current current)
                                    {
                                        final java.util.Set<String> serviceSet =
                                            new java.util.HashSet<String>(java.util.Arrays.asList(services));

                                        SwingUtilities.invokeLater(new Runnable()
                                            {
                                                public void run()
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
                                                }
                                            });
                                    }

                                    public void servicesStopped(final String[] services, Ice.Current current)
                                    {
                                        final java.util.Set<String> serviceSet =
                                            new java.util.HashSet<String>(java.util.Arrays.asList(services));

                                        SwingUtilities.invokeLater(new Runnable()
                                            {
                                                public void run()
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
                                                }
                                            });
                                    }

                                };

                            _serviceObserver = IceBox.ServiceObserverPrxHelper.uncheckedCast(
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
                        Ice.ObjectPrx serverAdmin = getServerAdmin();
                        if(serverAdmin != null)
                        {
                            IceBox.ServiceManagerPrx serviceManager =
                                IceBox.ServiceManagerPrxHelper.uncheckedCast(
                                    serverAdmin.ice_facet("IceBox.ServiceManager"));

                            try
                            {
                                //
                                // Ignore failures to register the service observers. Failures can occur if
                                // there's an incompatibility between IceGrid nodes & registries (it's the
                                // case for instance between 3.5 and 3.7).
                                //
                                serviceManager.begin_addObserver(_serviceObserver);
                            }
                            catch(Ice.LocalException ex)
                            {
                                // Ignore.
                            }
                        }
                    }
                }
                else if(_state == ServerState.Inactive)
                {
                    for(Service service: _services)
                    {
                        service.stopped();
                    }
                }
            }
            else if(_state == null || _state == ServerState.Inactive)
            {
                for(Service service: _services)
                {
                    service.stopped();
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
        java.util.List<Utils.ExpandedPropertySet> psList = new java.util.LinkedList<Utils.ExpandedPropertySet>();
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
            Ice.ObjectPrx proxy = null;
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
            TemplateDescriptor templateDescriptor
                = (TemplateDescriptor)_application.serviceTemplates.get(descriptor.template);

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

        PropertySetDescriptor serverInstancePSDescriptor = (PropertySetDescriptor)_servicePropertySets.get(serviceName);

        _services.add(new Service(this, serviceName, serviceResolver, descriptor, serviceDescriptor,
                                  serverInstancePSDescriptor));
    }

    Ice.ObjectPrx getServerAdmin()
    {
        if(_state != ServerState.Active)
        {
            return null;
        }
        AdminPrx admin = getCoordinator().getAdmin();
        if(admin == null)
        {
            return null;
        }
        else
        {
            Ice.Identity adminId = new Ice.Identity(_id, getCoordinator().getServerAdminCategory());
            return admin.ice_identity(adminId);
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

    public java.util.List<MetricsView>
    getMetrics()
    {
        return new java.util.ArrayList<MetricsView>(_metrics);
    }

    private ServerInstanceDescriptor _instanceDescriptor;
    private java.util.Map<String, PropertySetDescriptor> _servicePropertySets =
        new java.util.HashMap<String, PropertySetDescriptor>(); // with substituted names!

    private ServerDescriptor _serverDescriptor;
    private ApplicationDescriptor _application;

    private Utils.Resolver _resolver;
    private java.util.List<Adapter> _adapters = new java.util.LinkedList<Adapter>();
    private java.util.List<DbEnv> _dbEnvs = new java.util.LinkedList<DbEnv>();
    private java.util.List<Service> _services = new java.util.LinkedList<Service>();
    private java.util.List<MetricsView> _metrics = new java.util.LinkedList<MetricsView>();

    private java.util.Set<String> _startedServices = new java.util.HashSet<String>();

    private ServerState _state;
    private boolean _enabled;
    private int _stateIconIndex;
    private int _pid;
    private String _toolTip;
    private boolean _metricsRetrieved = false;

    private IceBox.ServiceObserverPrx _serviceObserver;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[][][] _icons;

    static private ServerEditor _editor;
    static private JPopupMenu _popup;
    static private JMenu _signalMenu;
    static private WriteMessageDialog _writeMessageDialog;
}
