// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ApplicationDescriptor;

class Application extends Parent
{
    //
    // Builds the application and all its subtrees
    //
    Application(ApplicationDescriptor descriptor, NodeViewRoot nodeViewRoot)
    {
	_nodeViewRoot = nodeViewRoot;
	update(descriptor, false);
    }

    void update(ApplicationDescriptor newDescriptor, boolean fireEvent)
    {
	assert(newDescriptor != null);
	clearChildren();
	
	_descriptor = newDescriptor;

	_serverTemplates = new ServerTemplates(_descriptor.serverTemplates, _descriptor.serviceTemplates);
	addChild(_serverTemplates);
	_serverTemplates.addParent(this); // no-op when this is not yet attached to its root
	                                  // (i.e. during construction)
	
	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates);
	addChild(_serviceTemplates);
	_serviceTemplates.addParent(this);

	_nodeVars = new NodeVars(_descriptor.nodes);
	addChild(_nodeVars);
	_nodeVars.addParent(this);
	
	_serverInstances = new ServerInstances(_descriptor.servers,
					       _descriptor.serverTemplates, 
					       _descriptor.serviceTemplates,
					       _nodeViewRoot);
	addChild(_serverInstances);
	_serverInstances.addParent(this);

	if(fireEvent)
	{
	    //    fireStructureChanged(this);
	}
    }
 
    public String toString()
    {
	return _descriptor.name;
    }

    
    private ApplicationDescriptor _descriptor;
    private NodeViewRoot _nodeViewRoot;

    //
    // Children
    //
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private NodeVars _nodeVars;
    private ServerInstances _serverInstances;
  
}
