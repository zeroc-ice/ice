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

import IceGrid.SimpleInternalFrame;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class Adapter extends Leaf
{
    static public AdapterDescriptor copyDescriptor(AdapterDescriptor d)
    {
	return (AdapterDescriptor)d.clone();
    }

    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];
	actions[COPY] = true;
	
	if(_parent.getAvailableActions()[PASTE])
	{
	    actions[PASTE] = true;
	}
	if(isEditable())
	{
	    actions[DELETE] = true;
	}
	
	if(_resolver != null && !_ephemeral)
	{
	    actions[SUBSTITUTE_VARS] = true;
	}
	return actions;
    }

    public void copy()
    {
	_model.setClipboard(copyDescriptor(_descriptor));
	if(_parent.getAvailableActions()[PASTE])
	{
	    _model.getActions()[PASTE].setEnabled(true);
	}
    }
    public void paste()
    {
	_parent.paste();
    }

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
	return _parent == null ? false : 
	    ((ListParent)_parent).destroyChild(this);
    }

    public void setParent(CommonBase parent)
    {
	if(_resolver != null)
	{
	    //
	    // In a server instance
	    //
	    _instanceId 
		= new AdapterInstanceId(_resolver.find("server"),
					_resolver.substitute(_descriptor.id));
	    
	    _proxy = _model.getRoot().registerAdapter(_resolver.find("node"),
						      _instanceId,
						      this);
	    createToolTip();
	}

	super.setParent(parent);
    }

    public void clearParent()
    {
	if(_parent != null)
	{
	    if(_instanceId != null)
	    {
		_model.getRoot().unregisterAdapter(_resolver.find("node"),
						   _instanceId, this);
	    }
	    super.clearParent();
	}
    }

    public Object getDescriptor()
    {
	return _descriptor;
    }

    public Object saveDescriptor()
    {
	return copyDescriptor(_descriptor);
    }

    public void restoreDescriptor(Object savedDescriptor)
    {
	AdapterDescriptor ad = (AdapterDescriptor)savedDescriptor;
	
	_descriptor.name = ad.name;
	_descriptor.id = ad.id;
	_descriptor.registerProcess = ad.registerProcess;
	_descriptor.waitForActivation = ad.waitForActivation;
	_descriptor.objects = ad.objects;
    }
    
   
    Adapter(String adapterName, AdapterDescriptor descriptor,
	    Utils.Resolver resolver, Model model)
    {
	super(adapterName, model);
	_descriptor = descriptor;
	_resolver = resolver;
	_ephemeral = false;
    }

    //
    // Fresh new temporary Adapter
    // Never becomes permanent; instead a new non-ephemeral Adapter is
    // created upon a successful "apply"
    //
    Adapter(String name, AdapterDescriptor descriptor, Model model)
    {
	super(name, model);
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

    
    Utils.Resolver getResolver()
    {
	return _resolver;
    }
    
    boolean isEditable()
    {
	if(_parent == null)
	{
	    return false;
	}
	else
	{
	    return ((Adapters)_parent).isEditable();
	}
	
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

    private final boolean _ephemeral;
    private AdapterDescriptor _descriptor;
    private Utils.Resolver _resolver;

    private AdapterInstanceId _instanceId;
    private Ice.ObjectPrx _proxy;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private AdapterEditor _editor;
}
