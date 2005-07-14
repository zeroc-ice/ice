// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.Model;
import IceGrid.ServerInstanceDescriptor;
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;
import IceGrid.Utils;


class ServerInstances extends Parent
{
    ServerInstances(java.util.List descriptors, 
		    Application application,
		    boolean fireEvent)
    {
	super("Server instances", application.getModel());
	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    //
	    // The ServerInstance constructor inserts the new object in the 
	    // node view model
	    //
	    ServerInstanceDescriptor descriptor = 
		(ServerInstanceDescriptor)p.next();

	    String serverName = computeServerName(descriptor, application);
		
	    ServerInstance child = new ServerInstance(serverName,
						      descriptor,
						      application,
						      fireEvent);
	    addChild(child);
	}
    }

    void update(java.util.List updates, String[] removeServers)
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	Application application = (Application)getParent(TreeModelI.APPLICATION_VIEW);

	//
	// One big set of removes
	//
	for(int i = 0; i < removeServers.length; ++i)
	{
	    ServerInstance server = (ServerInstance)findChild(removeServers[i]);
	    server.removeFromNode();
	}
	removeChildren(removeServers);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.iterator();
	while(p.hasNext())
	{
	    ServerInstanceDescriptor descriptor = (ServerInstanceDescriptor)p.next();
	    
	    String serverName = computeServerName(descriptor, application);

	    ServerInstance child = (ServerInstance)findChild(serverName);
	    if(child == null)
	    {
		newChildren.add(new ServerInstance(serverName, descriptor, application, true));
	    }
	    else
	    {
		child.rebuild(application, descriptor, true);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray(new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }

    void removeFromNodes()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerInstance server = (ServerInstance)p.next();
	    server.removeFromNode();
	}
    }


    static String computeServerName(ServerInstanceDescriptor instanceDescriptor, 
				    Application application)
    {
	String nodeName = instanceDescriptor.node;

	if(instanceDescriptor.template.length() > 0)
	{
	    //
	    // Can't be null
	    //
	    TemplateDescriptor templateDescriptor = 
		application.findServerTemplateDescriptor(instanceDescriptor.template);
	    
	    java.util.Map parameters = 
		Utils.substituteVariables(instanceDescriptor.parameterValues,
					  application.getNodeVariables(nodeName),
					  application.getVariables());

	    return Utils.substituteVariables(templateDescriptor.descriptor.name,
					     parameters,
					     application.getNodeVariables(nodeName),
					     application.getVariables());
	}
	else
	{
	  
	    return Utils.substituteVariables(instanceDescriptor.descriptor.name,
					     application.getNodeVariables(nodeName),
					     application.getVariables());
	}
    } 

    private java.util.List _descriptors;
}
