// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.ServerInstanceDescriptor;
import IceGrid.TemplateDescriptor;

class ServerInstances extends Parent
{
    ServerInstances(ServerInstanceDescriptor[] descriptors, 
		    java.util.Map serverTemplates,
		    java.util.Map serviceTemplates,
		    NodeViewRoot nodeViewRoot)
    {
	_descriptors = descriptors;
	_nodeViewRoot = nodeViewRoot;

	for(int i = 0; i < _descriptors.length; ++i)
	{
	    TemplateDescriptor templateDescriptor =
		(TemplateDescriptor)serverTemplates.
		get(_descriptors[i].template);

	    //
	    // The ServerInstance constructor inserts the new object in the 
	    // node view model
	    //
	    ServerInstance child = new ServerInstance(_descriptors[i],
						      templateDescriptor,
						      serviceTemplates,
						      _nodeViewRoot);
	    addChild(child);
	}
    }

    public String toString()
    {
	return "Server instances";
    }

    private ServerInstanceDescriptor[] _descriptors;
    private NodeViewRoot _nodeViewRoot;
}
