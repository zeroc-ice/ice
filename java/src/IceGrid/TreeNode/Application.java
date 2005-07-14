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

	_nodeVars = new NodeVars(_descriptor.nodes, _model);
	addChild(_nodeVars);

	//
	// _serviceTemplates must be created before _serverTemplates
	// and _serverInstances, since they may refer to them
	//
	_serviceTemplates = new ServiceTemplates(_descriptor.serviceTemplates, _model);
						
	addChild(_serviceTemplates);

	_serverTemplates = new ServerTemplates(descriptor.serverTemplates, this);
	addChild(_serverTemplates);
	
	//
	// _serverInstances must be processed last, after the templates
	// and the node variables have been created.
	//
	_serverInstances = new ServerInstances(_descriptor.servers, this, fireEvent);
	addChild(_serverInstances);
    }

    void removeFromNodes()
    {
	_serverInstances.removeFromNodes();
    }

    void update(ApplicationUpdateDescriptor desc)
    {
	//
	// Variables
	//
	for(int i = 0; i < desc.removeVariables.length; ++i)
	{
	    _descriptor.variables.remove(desc.removeVariables[i]);
	}
	_descriptor.variables.putAll(desc.variables);

	//
	// Targets and comment
	//

	if(desc.targets != null)
	{
	    _descriptor.targets = desc.targets.value;
	}

	if(desc.comment != null)
	{
	    _descriptor.comment = desc.comment.value;
	}

	//
	// Server templates
	//
	for(int i = 0; i < desc.removeServerTemplates.length; ++i)
	{
	    _descriptor.serverTemplates.remove(desc.removeServerTemplates[i]);
	}
	_descriptor.serverTemplates.putAll(desc.serverTemplates);
	_serverTemplates.update(desc.serverTemplates, desc.removeServerTemplates);
	
	//
	// Service templates
	//
	for(int i = 0; i < desc.removeServiceTemplates.length; ++i)
	{
	    _descriptor.serviceTemplates.remove(desc.removeServiceTemplates[i]);
	}
	_descriptor.serviceTemplates.putAll(desc.serviceTemplates);
	_serviceTemplates.update(desc.serviceTemplates, desc.removeServiceTemplates);

	//
	// Nodes
	//
	for(int i = 0; i < desc.removeNodes.length; ++i)
	{
	    _descriptor.nodes.remove(desc.removeNodes[i]);
	}
	_descriptor.nodes.addAll(desc.nodes);
	_nodeVars.update(desc.nodes, desc.removeNodes);

	//
	// Servers
	//
	for(int i = 0; i < desc.removeServers.length; ++i)
	{
	    _descriptor.servers.remove(desc.removeServers[i]);
	}
	_descriptor.servers.addAll(desc.servers);
	_serverInstances.update(desc.servers, desc.removeServers);
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


    NodeVar findNodeVar(String id)
    {
	return (NodeVar)_nodeVars.findChild(id);
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
	NodeVar nodeVar = findNodeVar(id);
	if(nodeVar != null)
	{
	    return nodeVar.getVariables();
	}
	else
	{
	    return null;
	}
    }

    private ApplicationDescriptor _descriptor;

    //
    // Children
    //
    private ServerTemplates _serverTemplates;
    private ServiceTemplates _serviceTemplates;
    private NodeVars _nodeVars;
    private ServerInstances _serverInstances;
  
}
