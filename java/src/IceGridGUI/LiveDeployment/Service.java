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
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

class Service extends ListArrayTreeNode
{
    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
        boolean[] actions = new boolean[ACTION_COUNT];

        if(((Server)_parent).getState() != null)
        {
            actions[RETRIEVE_LOG] = _serviceDescriptor.logs.length > 0;
        }
        return actions;
    }

    public void retrieveLog()
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
            for(int i = 0; i < _serviceDescriptor.logs.length; ++i)
            {
                pathArray[i] = _resolver.substitute(_serviceDescriptor.logs[i]);
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
          
            getRoot().openShowLogDialog(new ShowLogDialog.FileIteratorFactory()
                {
                    public FileIteratorPrx open(int count)
                        throws FileNotAvailableException, ServerNotExistException, NodeUnreachableException, 
                        DeploymentException
                    {
                        AdminSessionPrx session = getRoot().getCoordinator().getSession();
                        FileIteratorPrx result = session.openServerLog(_parent.getId(), fPath, count);
                        if(getRoot().getCoordinator().getCommunicator().getDefaultRouter() == null)
                        {
                            result = FileIteratorPrxHelper.uncheckedCast(
                                result.ice_endpoints(session.ice_getEndpoints()));
                        }
                        return result;
                    }
                    
                    public String getTitle()
                    {
                        return "Service " + _parent.getId() + "/" + _id + " " + new java.io.File(fPath).getName();
                    }
                    
                    public String getDefaultFilename()
                    {
                        return new java.io.File(fPath).getName();
                    }
                });
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
            _cellRenderer = new DefaultTreeCellRenderer();
            _cellRenderer.setOpenIcon(
                Utils.getIcon("/icons/16x16/service.png"));

            _cellRenderer.setClosedIcon(
                Utils.getIcon("/icons/16x16/service.png"));
        }

        return _cellRenderer.getTreeCellRendererComponent(
            tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public Editor getEditor()
    {
        if(_editor == null)
        {
            _editor = new ServiceEditor();
        }
        _editor.show(this);
        return _editor;
    }

    public JPopupMenu getPopupMenu()
    {
        LiveActions la = getCoordinator().getLiveActionsForPopup();

        if(_popup == null)
        {
            _popup = new JPopupMenu();
            _popup.add(la.get(RETRIEVE_LOG));
        }
        
        la.setTarget(this);
        return _popup;
    }


    Service(Server parent, String serviceName, Utils.Resolver resolver,
            ServiceInstanceDescriptor descriptor, 
            ServiceDescriptor serviceDescriptor,
            PropertySetDescriptor serverInstancePSDescriptor)
    {
        super(parent, serviceName, 2);
        _resolver = resolver;
        
        _instanceDescriptor = descriptor;
        _serviceDescriptor = serviceDescriptor;
        _serverInstancePSDescriptor = serverInstancePSDescriptor;
        
        _childrenArray[0] = _adapters;
        _childrenArray[1] = _dbEnvs;

        createAdapters();
        createDbEnvs();
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
        return result;
    }

    void nodeDown()
    {
        java.util.Iterator p = _adapters.iterator();
        while(p.hasNext())
        {
            Adapter adapter = (Adapter)p.next();
            adapter.update((AdapterDynamicInfo)null);
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

    java.util.SortedMap getProperties()
    {
        java.util.List psList = new java.util.LinkedList();
        Node node = (Node)_parent.getParent();

        String applicationName = ((Server)_parent).getApplication().name;

        psList.add(node.expand(_serviceDescriptor.propertySet,
                               applicationName, _resolver));

        if(_instanceDescriptor != null)
        {
            psList.add(node.expand(_instanceDescriptor.propertySet, 
                                   applicationName, _resolver));
        }          

        if(_serverInstancePSDescriptor != null)
        {
            psList.add(node.expand(_serverInstancePSDescriptor, 
                                   applicationName, _resolver));

        }

        return Utils.propertySetsToMap(psList, _resolver);
    }

    private void createAdapters()
    {
        java.util.Iterator p = _serviceDescriptor.adapters.iterator();
        while(p.hasNext())
        {
            AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
            String adapterName = Utils.substitute(descriptor.name, _resolver);
            
            String adapterId = Utils.substitute(descriptor.id, _resolver);
            Ice.ObjectPrx proxy = null;
            if(adapterId.length() > 0)
            {
                proxy = ((Node)_parent.getParent()).getProxy(adapterId);
            }
            
            insertSortedChild(
                new Adapter(this, adapterName, 
                            _resolver, adapterId, descriptor, proxy),
                _adapters, null);
        }
    }
    
    private void createDbEnvs()
    {
        java.util.Iterator p = _serviceDescriptor.dbEnvs.iterator();
        while(p.hasNext())
        {
            DbEnvDescriptor descriptor = (DbEnvDescriptor)p.next();
            String dbEnvName = Utils.substitute(descriptor.name, _resolver);
            insertSortedChild(
                new DbEnv(this, dbEnvName, _resolver, descriptor), _dbEnvs, null);
        }
    }    

    private final ServiceInstanceDescriptor _instanceDescriptor;
    private final ServiceDescriptor _serviceDescriptor;
    private final PropertySetDescriptor _serverInstancePSDescriptor;
    private final Utils.Resolver _resolver;

    private final java.util.List _adapters = new java.util.LinkedList();
    private final java.util.List _dbEnvs = new java.util.LinkedList();

    static private ServiceEditor _editor;
    static private DefaultTreeCellRenderer _cellRenderer;
    static private JPopupMenu _popup;
}
