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
import javax.swing.JPopupMenu;
import javax.swing.JPanel;
import javax.swing.JTree;
import javax.swing.Icon;
import javax.swing.tree.DefaultTreeCellRenderer;

import IceGrid.TreeModelI;
import IceGrid.IceBoxDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.TemplateDescriptor;
import IceGrid.Utils;
import IceGrid.Model;
import IceGrid.ServerDynamicInfo;

class ServerInstance extends Parent
{
    public JPopupMenu getPopupMenu()
    {
	return null;
    }

    public JPanel getProperties(int view)
    {
	return null;
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
	    _icons = new Icon[7];
	    _icons[0] = Utils.getIcon("/icons/unknown.png");
	    _icons[ServerState.Inactive.value() + 1] = Utils.getIcon("/icons/inactive.png");
	    _icons[ServerState.Activating.value() + 1] = Utils.getIcon("/icons/activating.png");
	    _icons[ServerState.Active.value() + 1] = Utils.getIcon("/icons/active.png");
	    _icons[ServerState.Deactivating.value() + 1] = Utils.getIcon("/icons/deactivating.png");
	    _icons[ServerState.Destroying.value() + 1] = Utils.getIcon("/icons/destroying.png");
	    _icons[ServerState.Destroyed.value() + 1] = Utils.getIcon("/icons/destroyed.png");
	}

	//
	// TODO: separate icons for open and close
	//
	if(expanded)
	{
	    _cellRenderer.setOpenIcon(_icons[_stateIconIndex]);
	}
	else
	{
	    _cellRenderer.setClosedIcon(_icons[_stateIconIndex]);
	}

	_cellRenderer.setToolTipText(_toolTip);
	return _cellRenderer.getTreeCellRendererComponent(
	    tree, value, sel, expanded, leaf, row, hasFocus);
    }


    //
    // Builds the server instance and all its sub-tree
    //
    ServerInstance(ServerInstanceDescriptor descriptor, 
		   Model model,
		   boolean fireNodeViewEvent)
    {
	super(descriptor.descriptor.name, model);
	
	rebuild(descriptor, fireNodeViewEvent);
    }


    void removeFromNode()
    {
	removeFromNode(true);
    }

    private void removeFromNode(boolean fireNodeViewEvent)
    {
	if(_serviceInstances != null)
	{
	    _serviceInstances.unregisterAdapters();
	}
	if(_adapters != null)
	{
	    _adapters.unregisterAll();
	}

	Node node = (Node)getParent(TreeModelI.NODE_VIEW);
	if(node != null)
	{
	    node.removeChild(_id, fireNodeViewEvent);
	}
    }

    //
    // Update the server instance and all its subtree
    //
    void rebuild(ServerInstanceDescriptor newDescriptor, 
		 boolean fireNodeViewEvent)
    {
	assert(newDescriptor != null);
	removeFromNode(fireNodeViewEvent);
	clearChildren();
	
	//
	// First check if my node changed [node view]
	// For the application view, the only way a server instance can change application 
	// is by being removed and then re-added (i.e. not with an update)
	//
	boolean newNode = false;
	
	if(_descriptor == null)
	{
	    newNode = true;
	}
	else if(_descriptor.node != newDescriptor.node)
	{
	    newNode = true;
	    //
	    // Remove from existing node
	    //
	    CommonBase parent = _parents[TreeModelI.NODE_VIEW];
	    assert(parent != null); // must be connected
	    removeParent(parent);
	    ((Parent)parent).removeChild(this);  
	}
	_descriptor = newDescriptor;
	
	Node node = _model.getNodeViewRoot().findNode(_descriptor.node);

	if(_descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_descriptor.descriptor;
	    
	    //
	    // We need to pass the node to register the adapters
	    // 
	    _serviceInstances = new ServiceInstances(_iceBoxDescriptor.services, 
						     _model, node);
	    addChild(_serviceInstances);
	    _serviceInstances.addParent(this); // no-op when newNode == true
	}
	else
	{
	    _iceBoxDescriptor = null;
	}

	_adapters = new Adapters(_descriptor.descriptor.adapters, _model, node);
	addChild(_adapters);
	_adapters.addParent(this); // no-op when newNode == true

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, _model, false);
	addChild(_dbEnvs);
	_dbEnvs.addParent(this); // no-op when newNode == true

	if(newNode)
	{
	    updateDynamicInfo(node.getServerDynamicInfo(_id), false);
	    addParent(node); // propagates to children
	    node.addChild(this, fireNodeViewEvent);
	}
    }

    void updateDynamicInfo(ServerDynamicInfo info)
    {
	updateDynamicInfo(info, true);
    }
    
    private void updateDynamicInfo(ServerDynamicInfo info, boolean fireEvent)
    {
	if(info.state != _state || info.pid != _pid)
	{
	    _state = info.state;
	    _pid = info.pid;
	    _toolTip = toolTip(info.state, info.pid);
	    _stateIconIndex = (_state == null ? 0 : _state.value() + 1); 

	    if(fireEvent)
	    {
		//
		// Change the node representation in all views
		//
		fireNodeChangedEvent(this);
	    }
	}
    }
   
    public String toString()
    {
	String result = _descriptor.descriptor.name;

	if(!_descriptor.template.equals(""))
	{
	    
	    result += ": " + templateLabel(_descriptor.template, 
					   _descriptor.parameterValues.values());
	}
	return result;
    }

    private static String toolTip(ServerState state, int pid)
    {
	String result = (state == null ? "Unknown" : state.toString());

	if(pid != 0)
	{
	    result += ", pid: " + pid;
	}
	return result;
    }

    private ServerState _state = null;
    private int _stateIconIndex = 0;
    private int _pid = 0;
    private String _toolTip = toolTip(_state, _pid);

    private ServerInstanceDescriptor _descriptor;
    private TemplateDescriptor _templateDescriptor;
    private IceBoxDescriptor _iceBoxDescriptor;
   
    //
    // Children
    //
    private ServiceInstances _serviceInstances;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;

    static private DefaultTreeCellRenderer _cellRenderer;
    static private Icon[] _icons;
  
}
