// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JPopupMenu;
import javax.swing.JPanel;

import IceGrid.TreeModelI;
import IceGrid.IceBoxDescriptor;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.ServerState;
import IceGrid.TemplateDescriptor;

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

    //
    // Builds the server instance and all its sub-tree
    //
    ServerInstance(ServerInstanceDescriptor descriptor, 
		   NodeViewRoot nodeViewRoot,
		   boolean fireNodeViewEvent)
    {
	super(descriptor.descriptor.name);
	_state = ServerState.Inactive;
	_pid = 0;
	_nodeViewRoot = nodeViewRoot;
	
	rebuild(descriptor, fireNodeViewEvent);
    }

    //
    // Update the server instance and all its subtree
    //
    void rebuild(ServerInstanceDescriptor newDescriptor, 
		 boolean fireNodeViewEvent)
    {
	assert(newDescriptor != null);
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
	
	if(_descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    _iceBoxDescriptor = (IceBoxDescriptor)_descriptor.descriptor;
	    
	    _serviceInstances = new ServiceInstances(_iceBoxDescriptor.services);
	    addChild(_serviceInstances);
	    _serviceInstances.addParent(this); // no-op when newNode == true
	}
	else
	{
	    _iceBoxDescriptor = null;
	}

	_adapters = new Adapters(_descriptor.descriptor.adapters, false);
	addChild(_adapters);
	_adapters.addParent(this); // no-op when newNode == true

	_dbEnvs = new DbEnvs(_descriptor.descriptor.dbEnvs, false);
	addChild(_dbEnvs);
	_dbEnvs.addParent(this); // no-op when newNode == true

	if(newNode)
	{
	    Node node =  _nodeViewRoot.findNode(_descriptor.node);
	    addParent(node); // propagates to children
	    node.addChild(this, fireNodeViewEvent);
	}
    }
    
    void updateDynamicInfo(ServerState state, int pid)
    {
	if(state != _state)
	{
	    _state = state;
	    _pid = pid;
	    
	    //
	    // Change the node representation in all views
	    //
	    fireNodeChangedEvent(this);
	}
	else
	{
	    //
	    // We don't show the pid on the GUI, so no need
	    // to fire any event.
	    //
	    _pid = pid;
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


    private NodeViewRoot _nodeViewRoot;
    private ServerState _state;
    private int _pid;

    private ServerInstanceDescriptor _descriptor;
    private TemplateDescriptor _templateDescriptor;
    private IceBoxDescriptor _iceBoxDescriptor;
   
    //
    // Children
    //
    private ServiceInstances _serviceInstances;
    private Adapters _adapters;
    private DbEnvs _dbEnvs;
  
}
