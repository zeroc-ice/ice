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

import javax.swing.Icon;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.SwingUtilities;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.zeroc.IceGrid.*;
import com.zeroc.IceGridGUI.*;

public class Service extends Communicator
{
    //
    // Actions
    //
    @Override
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[com.zeroc.IceGridGUI.LiveDeployment.TreeNode.ACTION_COUNT];

        ServerState serverState = ((Server)_parent).getState();

        if(serverState != null)
        {
            actions[RETRIEVE_LOG_FILE] = _serviceDescriptor.logs.length > 0;
        }
        if(serverState == ServerState.Active)
        {
            actions[RETRIEVE_ICE_LOG] = serverState == ServerState.Active;
            if(((Server)_parent).hasServiceObserver())
            {
                actions[START] = !_started;
                actions[STOP] = _started;
            }
            else
            {
                actions[START] = true;
                actions[STOP] = true;
            }
        }

        return actions;
    }

    @Override
    public void start()
    {
        com.zeroc.IceBox.ServiceManagerPrx serviceManager = com.zeroc.IceBox.ServiceManagerPrx.uncheckedCast(
            ((Server)_parent).getAdminFacet("IceBox.ServiceManager"));

        if(serviceManager != null)
        {
            final String prefix = "Starting service '" + _id + "'...";
            getCoordinator().getStatusBar().setText(prefix);

            try
            {
                serviceManager.startServiceAsync(_id).whenComplete((result, ex) ->
                    {
                        if(ex == null || ex instanceof com.zeroc.IceBox.AlreadyStartedException)
                        {
                            amiSuccess(prefix);
                        }
                        else
                        {
                            amiFailure(prefix, "Failed to start service " + _id, ex.toString());
                        }
                    });
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                failure(prefix, "Failed to start service " + _id, e.toString());
            }
        }
    }

    @Override
    public void stop()
    {
        com.zeroc.IceBox.ServiceManagerPrx serviceManager = com.zeroc.IceBox.ServiceManagerPrx.uncheckedCast(
            ((Server)_parent).getAdminFacet("IceBox.ServiceManager"));

        if(serviceManager != null)
        {
            final String prefix = "Stopping service '" + _id + "'...";
            getCoordinator().getStatusBar().setText(prefix);

            try
            {
                serviceManager.stopServiceAsync(_id).whenComplete((result, ex) ->
                    {
                        if(ex == null || ex instanceof com.zeroc.IceBox.AlreadyStoppedException)
                        {
                            amiSuccess(prefix);
                        }
                        else
                        {
                            amiFailure(prefix, "Failed to stop service " + _id, ex.toString());
                        }
                    });
            }
            catch(com.zeroc.Ice.LocalException e)
            {
                failure(prefix, "Failed to stop service " + _id, e.toString());
            }
        }
    }

    @Override
    public void retrieveLogFile()
    {
        assert _serviceDescriptor.logs.length > 0;

        String path = null;

        if(_serviceDescriptor.logs.length == 1)
        {
            path = _resolver.substitute(_serviceDescriptor.logs[0]);
        }
        else
        {
            Object[] pathArray = new Object[_serviceDescriptor.logs.length];
            int i = 0;
            for(String log : _serviceDescriptor.logs)
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
                        return session.openServerLog(_parent.getId(), fPath, count);
                    }

                    @Override
                    public String getTitle()
                    {
                        return "Service " + _parent.getId() + "/" + _id + " " + new java.io.File(fPath).getName();
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
            _cellRenderer = new DefaultTreeCellRenderer();

            _startedIcon = Utils.getIcon("/icons/16x16/service_running.png");
            _stoppedIcon = Utils.getIcon("/icons/16x16/service.png");
        }

        Icon icon = _started ?  _startedIcon : _stoppedIcon;

        if(expanded)
        {
            _cellRenderer.setOpenIcon(icon);
        }
        else
        {
            _cellRenderer.setClosedIcon(icon);
        }

        return _cellRenderer.getTreeCellRendererComponent(tree, value, sel, expanded, leaf, row, hasFocus);
    }

    @Override
    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new ServiceEditor(getCoordinator());
        }
        _editor.show(this);
        return _editor;
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
            _popup.add(la.get(RETRIEVE_ICE_LOG));
            _popup.add(la.get(RETRIEVE_LOG_FILE));
        }

        la.setTarget(this);
        return _popup;
    }

    //
    // Implement Communicator abstract methods
    //

    @Override
    protected java.util.concurrent.CompletableFuture<com.zeroc.Ice.ObjectPrx> getAdminAsync()
    {
        return java.util.concurrent.CompletableFuture.completedFuture(((Server)_parent).getAdmin());
    }

    @Override
    protected String getServerDisplayName()
    {
        return "Server " + _parent.getId();
    }

    @Override
    protected String getDisplayName()
    {
        return "Service " + _parent.getId() + "/" + _id;
    }

    @Override
    protected String getDefaultFileName()
    {
        return _parent.getId() + "-" + _id;
    }

    com.zeroc.Ice.ObjectPrx getAdminFacet(String facet)
    {
        String facetName = "IceBox.Service." + _id + "." + facet;

        try
        {
            if(Integer.valueOf(((Server)_parent).getProperties().get("IceBox.UseSharedCommunicator." + _id)) > 0)
            {
                facetName = "IceBox.SharedCommunicator." + facet;
            }
        }
        catch(NumberFormatException ex)
        {
        }

        return ((Server)_parent).getAdminFacet(facetName);
    }

    Service(Server parent, String serviceName, Utils.Resolver resolver, ServiceInstanceDescriptor descriptor,
            ServiceDescriptor serviceDescriptor, PropertySetDescriptor serverInstancePSDescriptor)
    {
        super(parent, serviceName, 3);
        _resolver = resolver;

        _instanceDescriptor = descriptor;
        _serviceDescriptor = serviceDescriptor;
        _serverInstancePSDescriptor = serverInstancePSDescriptor;

        _childrenArray[0] = _metrics;
        _childrenArray[1] = _adapters;
        _childrenArray[2] = _dbEnvs;

        createAdapters();
        createDbEnvs();
    }

    void stopShowIceLogDialog()
    {
        if(_showIceLogDialog != null)
        {
            _showIceLogDialog.stopped();
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
        return false;
    }

    int updateAdapters(java.util.List<AdapterDynamicInfo> infoList)
    {
        int result = 0;
        java.util.Iterator<Adapter> p = _adapters.iterator();
        while(p.hasNext() && result < infoList.size())
        {
            Adapter adapter = p.next();
            if(adapter.update(infoList))
            {
                result++;
            }
        }
        return result;
    }

    void nodeDown()
    {
        for(Adapter p : _adapters)
        {
            p.update((AdapterDynamicInfo)null);
        }
    }

    boolean isStarted()
    {
        return _started;
    }

    void started()
    {
        if(!_started)
        {
            _started = true;
            if(getRoot().getTree().isExpanded(getPath()))
            {
                fetchMetricsViewNames();
            }
            getRoot().getTreeModel().nodeChanged(this);
        }
    }

    void stopped()
    {
        if(_started)
        {
            _started = false;
            _metricsRetrieved = false;
            if(!_metrics.isEmpty())
            {
                _metrics.clear();
                rebuild(this);
            }
            getRoot().getTreeModel().nodeChanged(this);
        }
    }

    Utils.Resolver getResolver()
    {
        return _resolver;
    }

    ServiceDescriptor getServiceDescriptor()
    {
        return _serviceDescriptor;
    }

    ServiceInstanceDescriptor getInstanceDescriptor()
    {
        return _instanceDescriptor;
    }

    java.util.SortedMap<String, String> getProperties()
    {
        java.util.List<Utils.ExpandedPropertySet> psList = new java.util.LinkedList<>();
        Node node = (Node)_parent.getParent();

        String applicationName = ((Server)_parent).getApplication().name;

        psList.add(node.expand(_serviceDescriptor.propertySet, applicationName, _resolver));

        if(_instanceDescriptor != null)
        {
            psList.add(node.expand(_instanceDescriptor.propertySet, applicationName, _resolver));
        }

        if(_serverInstancePSDescriptor != null)
        {
            psList.add(node.expand(_serverInstancePSDescriptor, applicationName, _resolver));

        }

        return Utils.propertySetsToMap(psList, _resolver);
    }

    private void createAdapters()
    {
        for(AdapterDescriptor p : _serviceDescriptor.adapters)
        {
            String adapterName = Utils.substitute(p.name, _resolver);

            String adapterId = Utils.substitute(p.id, _resolver);
            com.zeroc.Ice.ObjectPrx proxy = null;
            if(adapterId.length() > 0)
            {
                proxy = ((Node)_parent.getParent()).getProxy(adapterId);
            }

            insertSortedChild(new Adapter(this, adapterName, _resolver, adapterId, p, proxy), _adapters, null);
        }
    }

    private void createDbEnvs()
    {
        for(DbEnvDescriptor p : _serviceDescriptor.dbEnvs)
        {
            String dbEnvName = Utils.substitute(p.name, _resolver);
            insertSortedChild(new DbEnv(this, dbEnvName, _resolver, p), _dbEnvs, null);
        }
    }

    void rebuild(Service service)
    {
        _adapters = service._adapters;
        _dbEnvs = service._dbEnvs;
        _metrics = service._metrics;

        _childrenArray[0] = _metrics;
        _childrenArray[1] = _adapters;
        _childrenArray[2] = _dbEnvs;

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

        for(MetricsView metrics: _metrics)
        {
            metrics.reparent(this);
        }
        getRoot().getTreeModel().nodeStructureChanged(this);
    }

    private final ServiceInstanceDescriptor _instanceDescriptor;
    private final ServiceDescriptor _serviceDescriptor;
    private final PropertySetDescriptor _serverInstancePSDescriptor;
    private final Utils.Resolver _resolver;

    private java.util.List<Adapter> _adapters = new java.util.LinkedList<>();
    private java.util.List<DbEnv> _dbEnvs = new java.util.LinkedList<>();

    private boolean _started = false;

    static private ServiceEditor _editor;
    static private DefaultTreeCellRenderer _cellRenderer;
    static private JPopupMenu _popup;
    static private Icon _startedIcon;
    static private Icon _stoppedIcon;
}
