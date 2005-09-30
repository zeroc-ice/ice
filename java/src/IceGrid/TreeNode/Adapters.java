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

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class Adapters extends ListParent
{
    static class NewPopupMenu extends JPopupMenu
    {
	NewPopupMenu()
	{
	    _new = new AbstractAction("New adapter")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			_parent.newAdapter(null);
		    }
		};

	    add(_new);
	}

	void setParent(Adapters parent)
	{
	    _parent = parent;
	}

	private Adapters _parent;
	private Action _new;
    }

    public JPopupMenu getPopupMenu()
    {
	if(canHaveNewChild())
	{
	    if(_popup == null)
	    {
		_popup = new NewPopupMenu();
	    }
	    _popup.setParent(this);
	    return _popup;
	}
	else
	{
	    return null;
	}
    }
    
    static public java.util.LinkedList
    copyDescriptors(java.util.LinkedList descriptors)
    {
	java.util.LinkedList copy = new java.util.LinkedList();
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    copy.add(Adapter.copyDescriptor((AdapterDescriptor)p.next()));
	}
	return copy;
    }

    Adapters(java.util.List descriptors, boolean isEditable, 
	     boolean inIceBox, Utils.Resolver resolver, 
	     Model model)
	throws UpdateFailedException
    {
	super("Adapters", model);
	_descriptors = descriptors;
	_isEditable = isEditable;
	_inIceBox = inIceBox;
	_resolver = resolver;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    
	    String adapterName = Utils.substitute(descriptor.name, _resolver);
	    
	    addChild(new Adapter(adapterName, descriptor, 
				 _resolver, _model));
	}
    }

    boolean isEditable()
    {
	return _isEditable;
    }

    protected boolean validate(Object d)
    {
	AdapterDescriptor descriptor = (AdapterDescriptor)d;

	String newName = Utils.substitute(descriptor.name, _resolver);

	CommonBase child = findChild(newName);
	if(child != null && child.getDescriptor() != descriptor)
	{
	    JOptionPane.showMessageDialog(
		_model.getMainFrame(),
		_model.getRoot().identify(_parent.getPath()) 
		+ " has already an adapter named '" 
		+ newName + "'",
		"Duplicate adapter name error",
		JOptionPane.ERROR_MESSAGE);
	    return false;
	}
	return true;
    }

    protected void applyUpdate(Object d)
    {
	AdapterDescriptor descriptor = (AdapterDescriptor)d;

	CommonBase oldChild = findChildWithDescriptor(descriptor);
	if(oldChild != null)
	{
	    removeChild(oldChild, true);
	}

	String adapterName = Utils.substitute(descriptor.name, _resolver);
	Adapter newChild = new Adapter(adapterName, descriptor, 
				       _resolver, _model);

	try
	{
	    addChild(newChild, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
    }

  

    boolean inIceBox()
    {
	return _inIceBox;
    }

    boolean canHaveNewChild()
    {
	return _isEditable && !_inIceBox;
    }

    void newAdapter(AdapterDescriptor descriptor)
    {
	//
	// Generate a unique child name; ignore substitution for simplicity
	//
	String baseName = descriptor == null ? "NewAdapter" : descriptor.name;
	String name = makeNewChildId(baseName);

	if(descriptor == null)
	{
	    CommonBase parent = getParent();
	    String defaultId = (parent instanceof Service || 
				parent instanceof ServiceTemplate) ? 
		"${server}.${service}." + name : "${server}." + name;

	    descriptor = new AdapterDescriptor(
		name,
		defaultId,
		"", // TODO: ReplicaId
		false,
		true,
		new java.util.LinkedList());   
	}
	else
	{
	    descriptor.name = name;
	}

	Adapter adapter = new Adapter(name, descriptor, _model);
	try
	{
	    addChild(adapter, true);
	}
	catch(UpdateFailedException e)
	{
	    assert false;
	}
	_model.setSelectionPath(adapter.getPath());
    }

    public void paste(Object descriptor)
    {
	if(canHaveNewChild() && descriptor instanceof AdapterDescriptor)
	{
	    AdapterDescriptor d = (AdapterDescriptor)descriptor;
	    newAdapter(Adapter.copyDescriptor(d));
	}
    }

    private Utils.Resolver _resolver;
    private boolean _isEditable;
    private boolean _inIceBox;

    static private NewPopupMenu _popup;
}
