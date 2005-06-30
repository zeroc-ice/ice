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
import IceGrid.TreeModelI;
import IceGrid.Model;

public class ApplicationViewRoot extends Parent
{
    public ApplicationViewRoot(Model model)
    {
	super("Applications", model, TreeModelI.APPLICATION_VIEW);
    }

    //
    // The node view root must be (re-)initialized before the application view root
    // 
    public void init(java.util.List descriptors)
    {
	assert(_children.size() == 0);
	
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    ApplicationDescriptor descriptor = (ApplicationDescriptor)p.next();
	    Application child = new Application(descriptor, _model, false);
	    addChild(child);
	    child.addParent(this);
	}

	//
	// Fire structure change for both application and node views
	//
	fireStructureChangedEvent(this);
	_model.getNodeViewRoot().fireStructureChangedEvent(this);
    }
    
    public void clear()
    {
	clearChildren();
	fireStructureChangedEvent(this);
    }

    public void applicationAdded(ApplicationDescriptor desc)
    {
	applicationAdded(desc, true);
    }

    private Application applicationAdded(ApplicationDescriptor desc, boolean fireEvent)
    {
	//
	// This always fires insert events on the node view for the new server
	// instances
	//
	Application child = new Application(desc, _model, true); 
	child.addParent(this);
	addChild(child, fireEvent);
	return child;
    }

    public void applicationRemoved(String name)
    {
	applicationRemoved(name, true);
    }
    
    private void applicationRemoved(String name, boolean fireEvent)
    {
	Application application = (Application)findChild(name);
	if(application != null)
	{
	    application.removeFromNodes();
	}
	
	removeChild(name, fireEvent);
    }

    public void applicationSynced(ApplicationDescriptor desc)
    {
	applicationRemoved(desc.name, false);
	Application child = applicationAdded(desc, false);
	child.fireStructureChangedEvent(this);
    }
    
    public void applicationUpdated(ApplicationUpdateDescriptor desc)
    {
	Application application = (Application)findChild(desc.name);
	application.update(desc);
    }
}
