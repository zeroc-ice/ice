// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import java.awt.event.ActionEvent;

import javax.swing.AbstractAction;
import javax.swing.Action;
import javax.swing.JOptionPane;
import javax.swing.JPopupMenu;


import IceGrid.Model;
import IceGrid.ServiceDescriptor;
import IceGrid.TemplateDescriptor;

class ServiceTemplates extends Templates
{
    static class NewPopupMenu extends JPopupMenu
    {
	NewPopupMenu()
	{
	    _new = new AbstractAction("New service template")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			_parent.newServiceTemplate();
		    }
		};

	    add(_new);
	}

	void setParent(ServiceTemplates parent)
	{
	    _parent = parent;
	}

	private ServiceTemplates _parent;
	private Action _new;
    }
    
    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new NewPopupMenu();
	}
	_popup.setParent(this);
	return _popup;
    }

    ServiceTemplates(java.util.Map descriptors, Model model)
	throws UpdateFailedException
    {
	super("Service templates", model);

	_descriptors = descriptors;

	java.util.Iterator p = _descriptors.entrySet().iterator();
	
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    addChild(new ServiceTemplate(false, (String)entry.getKey(),
					 (TemplateDescriptor)entry.getValue(),
					 _model));
	}
    }

    ServiceTemplates(ServiceTemplates o)
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
		addChild(new ServiceTemplate((ServiceTemplate)p.next()));
	    }
	    catch(UpdateFailedException e)
	    {
		assert false;
	    }
	}
    }

    void newServiceTemplate()
    {
	ServiceDescriptor sd = new ServiceDescriptor(
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    new java.util.LinkedList(),
	    "",
	    "",
	    "");
	    
	newServiceTemplate(new TemplateDescriptor(
			       sd, new java.util.LinkedList()));
    }

    void newServiceTemplate(TemplateDescriptor descriptor)
    {
	String id = makeNewChildId("NewServiceTemplate");
	
	ServiceTemplate t = new ServiceTemplate(id, descriptor, _model);
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

    public void paste(Object descriptor)
    {
	if(descriptor instanceof TemplateDescriptor)
	{
	    TemplateDescriptor td = (TemplateDescriptor)descriptor;
	    if(td.descriptor instanceof ServiceDescriptor)
	    {
		newServiceTemplate(td);
	    }
	}
    }

    boolean tryAdd(String newId, TemplateDescriptor descriptor)
    {
	if(findChild(newId) != null)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		"There is already a service template with the same id.",
		"Duplicate id",
		JOptionPane.INFORMATION_MESSAGE);
	    return false;
	}
	_descriptors.put(newId, descriptor);
	
	try
	{
	    addChild(new ServiceTemplate(true, newId, descriptor, _model), true);
	}
	catch(UpdateFailedException e)
	{
	    assert false; // impossible
	}
	return true;
    }

    protected java.util.List findAllTemplateInstances(String templateId)
    {
	return getApplication().findServiceInstances(templateId);
    }
    

    void getUpdates(java.util.Map updates)
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    ServiceTemplate t = (ServiceTemplate)p.next();
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
	java.util.Iterator p = _descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String templateId = (String)entry.getKey();
	    TemplateDescriptor d = (TemplateDescriptor)entry.getValue();

	    ServiceTemplate t = (ServiceTemplate)findChild(templateId);
	    if(t != null)
	    {
		t.rebuild(d);
	    }
	}
	purgeChildren(_descriptors.keySet());
	fireStructureChangedEvent(this);
    }


    void update(java.util.Map descriptors, String[] removeTemplates)
	throws UpdateFailedException
    {
	//
	// Note: _descriptors is updated by Application
	//
	
	//
	// One big set of removes
	//
	removeChildren(removeTemplates);

	//
	// One big set of updates, followed by inserts
	//
	java.util.Vector newChildren = new java.util.Vector();
	java.util.Vector updatedChildren = new java.util.Vector();
	
	java.util.Iterator p = descriptors.entrySet().iterator();
	while(p.hasNext())
	{
	    java.util.Map.Entry entry = (java.util.Map.Entry)p.next();
	    String name = (String)entry.getKey();
	    TemplateDescriptor templateDescriptor 
		= (TemplateDescriptor)entry.getValue();
	    ServiceTemplate child = (ServiceTemplate)findChild(name);
	    if(child == null)
	    {
		newChildren.add(new ServiceTemplate(false, name, 
						    templateDescriptor, _model));
	    }
	    else
	    {
		child.rebuild(templateDescriptor);
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

    static private NewPopupMenu _popup;
}
