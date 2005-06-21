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
import IceGrid.TreeModelI;

public class ApplicationViewRoot extends Parent
{
    public ApplicationViewRoot(NodeViewRoot nodeViewRoot)
    {
	super(TreeModelI.APPLICATION_VIEW);
	_nodeViewRoot = nodeViewRoot;
    }

    //
    // The node view root must be (re-)initialized before the application view root
    // 
    public void init(ApplicationDescriptor[] descriptors)
    {
	assert(_children.size() == 0);

	_descriptors = descriptors;

	for(int i = 0; i <  descriptors.length; ++i)
	{
	    Application child = new  Application(descriptors[i], _nodeViewRoot);
	    addChild(child);
	    child.addParent(this);
	}

	//
	// Fire structure change for both application and node views
	//
	fireStructureChanged(this);
	_nodeViewRoot.fireStructureChanged(this);
    }
    
    public void clear()
    {
	clearChildren();
	_descriptors = null;
	fireStructureChanged(this);
    }

    public String toString()
    {
	return "Applications";
    }

    private ApplicationDescriptor[] _descriptors;
    private NodeViewRoot _nodeViewRoot;
}
