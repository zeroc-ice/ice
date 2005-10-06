// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JOptionPane;

import IceGrid.Actions;
import IceGrid.IceBoxDescriptor;
import IceGrid.Model;
import IceGrid.TemplateDescriptor;
import IceGrid.TreeModelI;

class ServerTemplates extends Templates
{
    public Actions getActions()
    {
	if(_actions == null)
	{
	    _actions = new ServerTemplatesActions(_model);
	}
	_actions.reset(this);
	return _actions;
    }

    ServerTemplates(java.util.Map descriptors, Application application)
	throws UpdateFailedException
    {
	super("Server templates", application.getModel());

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServerTemplate(false, (String)entry.getKey(),
					(TemplateDescriptor)entry.getValue(),
					application));
	}
    }

    ServerTemplates(ServerTemplates o)
    {
	super(o);
	_descriptors = o._descriptors;

	//
	// Deep-copy children
	//
	java.util.Iterator p = o._children.iterator();
	while(p.hasNext())
	{
	    try
	    {
		addChild(new ServerTemplate((ServerTemplate)p.next()));
	    }
	    catch(UpdateFailedException e)
	    {
		assert false;
	    }
	}
    }

    void getUpdates(java.util.Map updates)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerTemplate t = (ServerTemplate)p.next();
	    if(t.isNew() || t.isModified())
	    {
		updates.put(t.getId(), t.getDescriptor());
	    }
	}
    }

    void update() throws UpdateFailedException
    {
	//
	// The only template-descriptor update going through the
	// update() calls is the update or removal of one template;
	// template addition does not require a complex validation.
	//

	Application application = getApplication();

	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String templateId = (String)entry.getKey();
	    TemplateDescriptor d = (TemplateDescriptor)entry.getValue();

	    ServerTemplate t = (ServerTemplate)findChild(templateId);
	    if(t != null)
	    {
		t.rebuild(d, application);
	    }
	}
	purgeChildren(_descriptors.keySet());
	fireStructureChangedEvent(this);
    }

    java.util.List findServiceInstances(String template)
    {
	java.util.List result = new java.util.LinkedList();
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerTemplate t = (ServerTemplate)p.next();
	    result.addAll(t.findServiceInstances(template));
	}
	return result;
    }

    void removeServiceInstances(String template)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServerTemplate t = (ServerTemplate)p.next();
	    t.removeServiceInstances(template);
	}
    }
    
    void newServerTemplate()
    {
	newServerTemplate(new TemplateDescriptor(
			      Server.newServerDescriptor(), 
			      new java.util.LinkedList(),
			      new java.util.TreeMap()));
    }

    void newIceBoxTemplate()
    {
	newServerTemplate(new TemplateDescriptor(
			      Server.newIceBoxDescriptor(), 
			      new java.util.LinkedList(),
			      new java.util.TreeMap()));
    }


    void newServerTemplate(TemplateDescriptor descriptor)
    {
	String id;
	if(descriptor.descriptor instanceof IceBoxDescriptor)
	{
	    id = makeNewChildId("NewIceBoxTemplate");
	}
	else
	{
	    id = makeNewChildId("NewServerTemplate");
	}
	
	ServerTemplate t = new ServerTemplate(id, descriptor, getApplication());
	try
	{
	    addChild(t, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(t.getPath());
    }

    void paste()
    {
	Object descriptor =  _model.getClipboard();
	TemplateDescriptor td = (TemplateDescriptor)descriptor;
	newServerTemplate(td);
    }

    boolean tryAdd(String newId, TemplateDescriptor descriptor)
    {
	if(findChild(newId) != null)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		"There is already a server template with the same id.",
		"Duplicate id",
		JOptionPane.INFORMATION_MESSAGE);
	    return false;
	}
	_descriptors.put(newId, descriptor);
	
	try
	{
	    addChild(new ServerTemplate(true, newId, descriptor, 
					getApplication()), true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
	return true;
    }

    protected java.util.List findAllTemplateInstances(String templateId)
    {
	return getApplication().findServerInstances(templateId);
    }

    void update(java.util.Map updates, String[] removeTemplates)
	throws UpdateFailedException
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	Application application = getApplication();

	//
	// One big set of removes
	//
	removeChildren(removeTemplates);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = updates.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    TemplateDescriptor templateDescriptor 
		= (TemplateDescriptor)entry.getValue();
	    ServerTemplate child = (ServerTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServerTemplate(false, name, templateDescriptor,
						   application));
	    }
	    else
	    {
		child.rebuild(templateDescriptor, application);
		updatedChildren.add(child);
	    }
	}
	
	updateChildren((CommonBaseI[])updatedChildren.toArray
		       (new CommonBaseI[0]));
	addChildren((CommonBaseI[])newChildren.toArray(new CommonBaseI[0]));
    }
    
    void removeDescriptor(String id)
    {
	_descriptors.remove(id);
    }

    private java.util.Map _descriptors;
    static private ServerTemplatesActions _actions;
}
