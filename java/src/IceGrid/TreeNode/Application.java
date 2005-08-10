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
import IceGrid.ApplicationUpdateDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.Model;
import IceGrid.ServerDynamicInfo;
import IceGrid.AdapterDynamicInfo;

class Application extends Parent
{
    //
    // Builds the application and all its subtrees
    //
    Application(ApplicationDescriptor descriptor, Model model)
    {
	super(descriptor.name, model);
	_descriptor = descriptor;

	_replicatedAdapters = new ReplicatedAdapters(_descriptor.replicatedAdapters,
						     _model);
	addChild(_replicatedAdapters);

	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates,
						 _model);	
	addChild(_serviceTemplates);

	_serverTemplates = new ServerTemplates(descriptor.serverTemplates,
					       this);
	addChild(_serverTemplates);
	
	_nodes = new Nodes(_descriptor.nodes, this);
	addChild(_nodes);
    }

    void update(ApplicationUpdateDescriptor desc)
    {
	//
	// Description
	//
	if(desc.description != null)
	{
	    _descriptor.description = desc.description.value;
	}

	//
	// Variables
	//
	for(int i = 0; i < desc.removeVariables.length; ++i)
	{
	    _descriptor.variables.remove(desc.removeVariables[i]);
	}
	_descriptor.variables.putAll(desc.variables);


	//
	// Replicated adapters
	//
	for(int i = 0; i < desc.removeReplicatedAdapters.length; ++i)
	{
	    _descriptor.replicatedAdapters.remove(desc.
						  removeReplicatedAdapters[i]);
	}
	_descriptor.replicatedAdapters.addAll(desc.replicatedAdapters);
	_replicatedAdapters.update(desc.replicatedAdapters, 
				   desc.removeReplicatedAdapters);


	//
	// Service templates
	//
	for(int i = 0; i < desc.removeServiceTemplates.length; ++i)
	{
	    _descriptor.serviceTemplates.remove(desc.
						removeServiceTemplates[i]);
	}
	_descriptor.serviceTemplates.putAll(desc.serviceTemplates);
	_serviceTemplates.update(desc.serviceTemplates, 
				 desc.removeServiceTemplates);

	//
	// Server templates
	//
	for(int i = 0; i < desc.removeServerTemplates.length; ++i)
	{
	    _descriptor.serverTemplates.remove(desc.removeServerTemplates[i]);
	}
	_descriptor.serverTemplates.putAll(desc.serverTemplates);
	_serverTemplates.update(desc.serverTemplates, 
				desc.removeServerTemplates);
	
	//
	// Nodes
	//
	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    _descriptor.nodes.remove(desc.removeNodes[i]);
	}
	//
	// Updates also _descriptor.nodes
	//
	_nodes.update(desc.nodes, desc.removeNodes);
    }

    ServerTemplate findServerTemplate(String id)
    {
	return (ServerTemplate)_serverTemplates.findChild(id);
    }

    TemplateDescriptor findServerTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serverTemplates.get(templateName);
    }

    TemplateDescriptor findServiceTemplateDescriptor(String templateName)
    {
	return (TemplateDescriptor)
	    _descriptor.serviceTemplates.get(templateName);
    }


    Node findNode(String id)
    {
	return (Node)_nodes.findChild(id);
    }
    
    ServerTemplates getServerTemplates()
    {
	return _serverTemplates;
    }

    //
    // Should only be used for reading
    //
    java.util.Map getVariables()
    {
	return _descriptor.variables;
    }

    void nodeUp(String nodeName)
    {
	_nodes.nodeUp(nodeName);
    }

    void nodeDown(String nodeName)
    {
	_nodes.nodeDown(nodeName);
    }
    
    public void cleanup()
    {
	_nodes.cleanup();
    }


    private ApplicationDescriptor _descriptor;

    //
    // Children
    //
    private ReplicatedAdapters _replicatedAdapters;
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private Nodes _nodes;
}
