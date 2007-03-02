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
import javax.swing.JMenu;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

class Server extends ListArrayTreeNode
{
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

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

        return actions;
    }

    public void start()
    {
        final String prefix = "Starting server '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        AMI_Admin_startServer cb = new AMI_Admin_startServer()
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
                    amiFailure(prefix, "Failed to start " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to start " + _id, e.toString());
                }
            };
        
        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().startServer_async(cb, _id);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to start " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }
    
    public void stop()
    {
        final String prefix = "Stopping server '" + _id + "'...";
        getCoordinator().getStatusBar().setText(prefix);

        AMI_Admin_stopServer cb = new AMI_Admin_stopServer()
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
                    amiFailure(prefix, "Failed to stop " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to stop " + _id, e.toString());
                }
            };
        
        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
           getCoordinator().getAdmin().stopServer_async(cb, _id);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to stop " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
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
                    throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, DeploymentException
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
                    if(getRoot().getCoordinator().getCommunicator().getDefaultRouter() == null)
                    {
                        result = FileIteratorPrxHelper.uncheckedCast(
                            result.ice_endpoints(session.ice_getEndpoints()));
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
            for(int i = 0; i < _serverDescriptor.logs.length; ++i)
            {
                pathArray[i] = _resolver.substitute(_serverDescriptor.logs[i]);
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
                        FileIteratorPrx result = session.openServerLog(_id, fPath, count);

                        if(getRoot().getCoordinator().getCommunicator().getDefaultRouter() == null)
                        {
                            result = FileIteratorPrxHelper.uncheckedCast(
                                result.ice_endpoints(session.ice_getEndpoints()));
                        }
                        return result;
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

        AMI_Admin_sendSignal cb = new AMI_Admin_sendSignal()
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
                    amiFailure(prefix, "Failed to deliver signal " + s + " to " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to deliver signal " + s + " to " + _id, e.toString());
                }
            };
        
        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
            getCoordinator().getAdmin().sendSignal_async(cb, _id, s);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to deliver signal " + s + " to " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
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

        AMI_Admin_patchServer cb = new AMI_Admin_patchServer()
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
                    amiFailure(prefix, "Failed to patch " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to patch " + _id, e.toString());
                }
            };
        
        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
           getCoordinator().getAdmin().
               patchServer_async(cb, _id, 
                                 shutdown == JOptionPane.YES_OPTION);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to patch " + _id, e.toString());
        }
        finally
        {
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.DEFAULT_CURSOR));
        }
    }

    private void enableServer(boolean enable)
    {
        final String prefix = (enable ?
            "Enabling" : "Disabling") + " server '" + _id + "'...";
        
        final String action = enable ? "enable" : "disable";
        
        getCoordinator().getStatusBar().setText(prefix);

        AMI_Admin_enableServer cb = new AMI_Admin_enableServer()
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
                    amiFailure(prefix, "Failed to " + action + " " + _id, e);
                }

                public void ice_exception(Ice.LocalException e)
                {
                    amiFailure(prefix, "Failed to " + action + " " + _id, e.toString());
                }
            };
        
        try
        {   
            getCoordinator().getMainFrame().setCursor(
                Cursor.getPredefinedCursor(Cursor.WAIT_CURSOR));
           getCoordinator().getAdmin().enableServer_async(cb, _id, enable);
        }
        catch(Ice.LocalException e)
        {
            failure(prefix, "Failed to " + action + " " + _id, e.toString());
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
            _icons[ServerState.Inactive.value() + 1][0][0]
                = Utils.getIcon("/icons/16x16/server_inactive.png");
            _icons[ServerState.Activating.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_activating.png");
            _icons[ServerState.Active.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_active.png");
            _icons[ServerState.Deactivating.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_deactivating.png");
            _icons[ServerState.Destroying.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_destroying.png");
            _icons[ServerState.Destroyed.value() + 1][0][0] = 
                Utils.getIcon("/icons/16x16/server_destroyed.png");

            //
            // IceBox servers
            //
            _icons[0][1][0] = Utils.getIcon("/icons/16x16/icebox_server_unknown.png");
            _icons[ServerState.Inactive.value() + 1][1][0]
                = Utils.getIcon("/icons/16x16/icebox_server_inactive.png");
            _icons[ServerState.Activating.value() + 1][1][0] = 
                Utils.getIcon("/icons/16x16/icebox_server_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][1][0] = 
                Utils.getIcon("/icons/16x16/icebox_server_activating.png");
            _icons[ServerState.Active.value() + 1][1][0] = 
                Utils.getIcon("/icons/16x16/icebox_server_active.png");
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
            _icons[ServerState.Inactive.value() + 1][0][1]
                = Utils.getIcon("/icons/16x16/server_disabled_inactive.png");
            _icons[ServerState.Activating.value() + 1][0][1] = 
                Utils.getIcon("/icons/16x16/server_disabled_activating.png");
            _icons[ServerState.ActivationTimedOut.value() + 1][0][1] = 
                Utils.getIcon("/icons/16x16/server_disabled_activating.png");
            _icons[ServerState.Active.value() + 1][0][1] = 
                Utils.getIcon("/icons/16x16/server_disabled_active.png");
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
        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }


    Server(Node parent, String serverId, Utils.Resolver resolver,
           ServerInstanceDescriptor instanceDescriptor,
           ServerDescriptor serverDescriptor, ApplicationDescriptor application,
           ServerState state, int pid, boolean enabled)
    {
        super(parent, serverId, 3);
        _resolver = resolver;
        
        _instanceDescriptor = instanceDescriptor;
        _serverDescriptor = serverDescriptor;
        _application = application;
        
        _childrenArray[0] = _adapters;
        _childrenArray[1] = _dbEnvs;
        _childrenArray[2] = _services;

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

    void rebuild(Server server)
    {   
        _resolver = server._resolver;
        _instanceDescriptor = server._instanceDescriptor;
        _serverDescriptor = server._serverDescriptor;
        _application = server._application;
        _adapters = server._adapters;
        _dbEnvs = server._dbEnvs;

        _services = server._services;
        
        _childrenArray[0] = _adapters;
        _childrenArray[1] = _dbEnvs;
        _childrenArray[2] = _services;

        getRoot().getTreeModel().nodeStructureChanged(this);
    }

    void rebuild(Utils.Resolver resolver, boolean variablesChanged, java.util.Set serviceTemplates,
                 java.util.Set serverTemplates)
    {
        if(variablesChanged || 
           (_instanceDescriptor != null && serverTemplates != null && serverTemplates.contains(_instanceDescriptor.template)))
        {
            if(_instanceDescriptor != null)
            {
                TemplateDescriptor templateDescriptor = 
                    (TemplateDescriptor)_application.serverTemplates.get(_instanceDescriptor.template);
                assert templateDescriptor != null;
        
                _resolver.reset(resolver, _instanceDescriptor.parameterValues,
                                templateDescriptor.parameterDefaults);
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
            
            getRoot().getTreeModel().nodeStructureChanged(this);
        }
        else if(serviceTemplates != null && serviceTemplates.size() > 0 && _serverDescriptor instanceof IceBoxDescriptor)
        {
            _services.clear();
            _servicePropertySets.clear();
            createServices();
            getRoot().getTreeModel().nodeStructureChanged(this);
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

            if(fireEvent)
            {
                getRoot().getTreeModel().nodeChanged(this);
            }
        }
    }

    boolean updateAdapter(AdapterDynamicInfo info)
    {
        java.util.Iterator p = _adapters.iterator();
        while(p.hasNext())
        {
            Adapter adapter = (Adapter)p.next();
            if(adapter.update(info))
            {
                return true;
            }
        }

        //
        // Could be in one of the services as well
        //
        p = _services.iterator();
        while(p.hasNext())
        {
            Service service = (Service)p.next();
            if(service.updateAdapter(info))
            {
                return true;
            }
        }
        return false;
    }

    int updateAdapters(java.util.List infoList)
    {
        int result = 0;
        java.util.Iterator p = _adapters.iterator();
        while(p.hasNext() && result < infoList.size())
        {
            Adapter adapter = (Adapter)p.next();
            if(adapter.update(infoList))
            {
                result++;
            }
        }

        //
        // Could be in one of the services as well
        //
        p = _services.iterator();
        while(p.hasNext() && result < infoList.size())
        {
            Service service = (Service)p.next();
            result += service.updateAdapters(infoList);
        }
        return result;
    }

    void nodeDown()
    {
        update(null, 0, true, true);

        java.util.Iterator p = _adapters.iterator();
        while(p.hasNext())
        {
            Adapter adapter = (Adapter)p.next();
            adapter.update((AdapterDynamicInfo)null);
        }
        
        p = _services.iterator();
        while(p.hasNext())
        {
            Service service = (Service)p.next();
            service.nodeDown();
        }
    }

    java.util.SortedMap getProperties()
    {
        java.util.List psList = new java.util.LinkedList();
        Node node = (Node)_parent;

        
        psList.add(node.expand(_serverDescriptor.propertySet,
                               _application.name, _resolver));

        if(_instanceDescriptor != null)
        {
            psList.add(node.expand(_instanceDescriptor.propertySet, 
                                   _application.name, _resolver));
        }

        return Utils.propertySetsToMap(psList, _resolver);
    }

    private void createAdapters()
    {
        java.util.Iterator p = _serverDescriptor.adapters.iterator();
        while(p.hasNext())
        {
            AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
            String adapterName = Utils.substitute(descriptor.name, _resolver);
            String adapterId = Utils.substitute(descriptor.id, _resolver);
            Ice.ObjectPrx proxy = null;
            if(adapterId.length() > 0)
            {
                proxy = ((Node)_parent).getProxy(adapterId);
            }

            insertSortedChild(new Adapter(this, adapterName, 
                                          _resolver, adapterId, descriptor, proxy),
                              _adapters, null);
        }
    }
    
    private void createDbEnvs()
    {
        java.util.Iterator p = _serverDescriptor.dbEnvs.iterator();
        while(p.hasNext())
        {
            DbEnvDescriptor descriptor = (DbEnvDescriptor)p.next();
            String dbEnvName = Utils.substitute(descriptor.name, _resolver);
            
            insertSortedChild(new DbEnv(this, dbEnvName, _resolver, descriptor),
                              _dbEnvs, null);
        }
    }

    private void createServices()
    {
        if(_serverDescriptor instanceof IceBoxDescriptor)
        {
            if(_instanceDescriptor != null)
            {
                java.util.Iterator p = _instanceDescriptor.servicePropertySets.entrySet().iterator();
                while(p.hasNext())
                {
                    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
                    _servicePropertySets.put(_resolver.substitute((String)entry.getKey()), entry.getValue());
                }
            }

            IceBoxDescriptor iceBoxDescriptor = (IceBoxDescriptor)_serverDescriptor;
         
            java.util.Iterator p = iceBoxDescriptor.services.iterator();
            while(p.hasNext())
            {
                ServiceInstanceDescriptor descriptor = (ServiceInstanceDescriptor)p.next();
                createService(descriptor);
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

        PropertySetDescriptor serverInstancePSDescriptor = 
            (PropertySetDescriptor)_servicePropertySets.get(serviceName);

        _services.add(new Service(this, serviceName, serviceResolver,
                                  descriptor, serviceDescriptor, serverInstancePSDescriptor));
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
    private java.util.Map _servicePropertySets = new java.util.HashMap(); // with substituted names!

    private ServerDescriptor _serverDescriptor;
    private ApplicationDescriptor _application;

    private Utils.Resolver _resolver;
    private java.util.List _adapters = new java.util.LinkedList();
    private java.util.List _dbEnvs = new java.util.LinkedList();
    private java.util.List _services = new java.util.LinkedList();

    private ServerState _state;
    private boolean _enabled;
    private int _stateIconIndex;
    private int _pid;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[][][] _icons;

    static private ServerEditor _editor;
    static private JPopupMenu _popup;
    static private JMenu _signalMenu;
    static private WriteMessageDialog _writeMessageDialog;
}
