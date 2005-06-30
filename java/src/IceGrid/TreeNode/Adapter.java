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

import IceGrid.AdapterDescriptor;
import IceGrid.Model;

class Adapter extends Leaf
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
	if(_node == null)
	{
	    return null;
	}
	else
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
    }


    Adapter(AdapterDescriptor descriptor, Model model, Node node)
    {
	super(descriptor.name, model);
	_descriptor = descriptor;
	_node = node;

	if(_node != null)
	{
	    _proxy = node.registerAdapter(_descriptor.id, this);
	    createToolTip();
	}
    }

    void unregister()
    {
	if(_node != null)
	{
	    _node.unregisterAdapter(_descriptor.id);
	    _node = null;
	}
    }

 
    void updateProxy(Ice.ObjectPrx proxy)
    {
	assert(_node != null);
	_proxy = proxy;
	createToolTip();
	fireNodeChangedEvent(this);
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

    private AdapterDescriptor _descriptor;
    private Node _node;
    private Ice.ObjectPrx _proxy;
    private String _toolTip;

    static private DefaultTreeCellRenderer _cellRenderer;
}
