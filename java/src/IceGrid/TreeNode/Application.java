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
    Application(ApplicationDescriptor descriptor, Model model, 
		boolean fireEvent)
    {
	super(descriptor.name, model);
	_descriptor = descriptor;

	//
	// TODO: add replicated adapter
	//

	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates,
						 _model);	
	addChild(_serviceTemplates);

	_serverTemplates = new ServerTemplates(descriptor.serverTemplates,
					       this);
	addChild(_serverTemplates);
	
	_nodes = new Nodes(_descriptor.nodes, _model);
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
	_descriptor.nodes.putAll(_nodes.update(desc.nodes, 
					       desc.removeNodes));
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

    java.util.Map getNodeVariables(String id)
    {
	Node node = findNode(id);
	if(node != null)
	{
	    return node.getVariables();
	}
	else
	{
	    return null;
	}
    }

    void nodeUp(String nodeName, java.util.Map serverMap, 
		       java.util.Map adapterMap)
    {
	_nodes.nodeUp(nodeName, serverMap, adapterMap);
    }

    void nodeDown(String nodeName)
    {
	_nodes.nodeDown(nodeName);
    }
    
    void updateServer(String nodeName, ServerDynamicInfo updatedInfo)
    {
	_nodes.updateServer(nodeName, updatedInfo);
    }
    
    void updateAdapter(String nodeName, AdapterDynamicInfo updatedInfo)
    {
	_nodes.updateAdapter(nodeName, updatedInfo);
    }
    

    private ApplicationDescriptor _descriptor;

    //
    // Children
    //
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private Nodes _nodes;
}
