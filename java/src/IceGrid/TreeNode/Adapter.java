// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.Component;
import javax.swing.JTree;
import javax.swing.tree.DefaultTreeCellRenderer;

import com.jgoodies.uif_lite.panel.SimpleInternalFrame;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class Adapter extends Leaf
{
    //
    // Overridden to show tooltip
    //
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
	}
	
	_cellRenderer.setToolTipText(_toolTip);
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }

    public void displayProperties()
    {
	SimpleInternalFrame propertiesFrame = _model.getPropertiesFrame();
	
	propertiesFrame.setTitle("Properties for " + _id);
	if(_editor == null)
	{
	    _editor = new AdapterEditor(_model.getMainFrame());
	}
	
	_editor.show(this);
	propertiesFrame.setContent(_editor.getComponent());
	propertiesFrame.validate();
	propertiesFrame.repaint();
    }

    public boolean destroy()
    {
	if(isEphemeral() || isEditable() && _model.canUpdate())
	{
	    Adapters adapters = (Adapters)_parent;
	  
	    if(isEphemeral())
	    {
		adapters.removeChild(this, true);
	    }
	    else
	    {
		adapters.removeDescriptor(_descriptor);
		getEditable().markModified();
		getApplication().applySafeUpdate();
	    }
	    return true;
	}
	else
	{
	    return false;
	}
    }

    public void unregister()
    {
	if(_instanceId != null)
	{
	    getApplication().unregisterAdapter(_resolver.find("node"),
					       _instanceId,
					       this);
	}
    }

    public Object copy()
    {
	return _descriptor.clone();
    }


    Adapter(String adapterName, AdapterDescriptor descriptor,
	    Utils.Resolver resolver, Application application, Model model)
    {
	super(adapterName, model);
	_descriptor = descriptor;
	_resolver = resolver;
	_ephemeral = false;
	
	if(resolver != null)
	{
	    assert application != null;
	    //
	    // In a server instance
	    //
	    _instanceId 
		= new AdapterInstanceId(_resolver.find("server"),
					_resolver.substitute(_descriptor.id));
	    
	    _proxy = application.registerAdapter(_resolver.find("node"),
						 _instanceId,
						 this);
	    createToolTip();
	}
    }

    //
    // Fresh new temporary Adapter
    // Never becomes permanent; instead a new non-ephemeral Adapter is
    // created upon a successful "apply"
    //
    Adapter(String name, AdapterDescriptor descriptor, 
	    Utils.Resolver resolver, Model model)
    {
	super("*" + name, model);
	_resolver = resolver;
	_descriptor = descriptor;
	_ephemeral = true;
    }

    void updateProxy(Ice.ObjectPrx proxy)
    {
	assert _resolver != null;
	_proxy = proxy;
	createToolTip();
	fireNodeChangedEvent(this);
    }

    AdapterDescriptor getDescriptor()
    {
	return _descriptor;
    }

    Utils.Resolver getResolver()
    {
	return _resolver;
    }
    
    boolean isEditable()
    {
	return ((Adapters)_parent).isEditable();
    }

    boolean inIceBox()
    {
	return ((Adapters)_parent).inIceBox();
    }

    String getEndpoints()
    {
	PropertiesHolder ph = getParent().getParent().getPropertiesHolder();
	assert ph != null;
	return ph.get(_descriptor.name + ".Endpoints");
    }

    void setEndpoints(String newName, String newEndpoints)
    {
	PropertiesHolder ph = getParent().getParent().getPropertiesHolder();
	assert ph != null;
	ph.replace(_descriptor.name + ".Endpoints", newName + ".Endpoints",
		   newEndpoints);
    }

    AdapterInstanceId getInstanceId()
    {
	return _instanceId;
    }

    public boolean isEphemeral()
    {
	return _ephemeral;
    }

    private void createToolTip()
    {
	if(_proxy == null)
	{
	    _toolTip = null;
	}
	else
	{
	    _toolTip = "Proxy: " + _model.getCommunicator().proxyToString(_proxy);
	}
    }

    private boolean _ephemeral;
    private AdapterDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private AdapterInstanceId _instanceId;
    private Ice.ObjectPrx _proxy;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private AdapterEditor _editor;
}
