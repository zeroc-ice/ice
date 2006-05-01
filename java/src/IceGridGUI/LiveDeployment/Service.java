// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.LiveDeployment;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;
import javax.swing.tree.TreeModel;
import javax.swing.tree.TreePath;
import java.util.Enumeration;

import IceGrid.*;
import IceGridGUI.*;

class Service extends ListArrayTreeNode
{
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


    Service(Server parent, String serviceName, Utils.Resolver resolver,
	    ServiceInstanceDescriptor descriptor, 
	    ServiceDescriptor serviceDescriptor)
    {
	super(parent, serviceName, 2);
	_resolver = resolver;
	
	_instanceDescriptor = descriptor;
	_serviceDescriptor = serviceDescriptor;
	
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

    private ServiceInstanceDescriptor _instanceDescriptor;
    private ServiceDescriptor _serviceDescriptor;
    private Utils.Resolver _resolver;

    private java.util.List _adapters = new java.util.LinkedList();
    private java.util.List _dbEnvs = new java.util.LinkedList();

    static private ServiceEditor _editor;
    static private DefaultTreeCellRenderer _cellRenderer;   
}
