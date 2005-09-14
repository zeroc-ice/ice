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
import javax.swing.JPopupMenu;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;


class Adapters extends Parent
{
    static class PopupMenu extends JPopupMenu
    {
	PopupMenu()
	{
	    _new = new AbstractAction("New")
		{
		    public void actionPerformed(ActionEvent e) 
		    {
			_adapters.newAdapter(null);
		    }
		};

	    add(_new);
	}

	void setAdapters(Adapters adapters)
	{
	    _adapters = adapters;
	}

	private Adapters _adapters;
	private Action _new;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_isEditable && !_inIceBox)
	{
	    if(_popup == null)
	    {
		_popup = new PopupMenu();
	    }
	    _popup.setAdapters(this);
	    return _popup;
	}
	else
	{
	    return null;
	}
    }

    public void unregister()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.unregister();
	}
    }


    Adapters(java.util.List descriptors, boolean isEditable, 
	     boolean inIceBox, Utils.Resolver resolver, 
	     Application application, Model model)
	throws DuplicateIdException
    {
	super("Adapters", model);
	_descriptors = descriptors;
	_isEditable = isEditable;
	_inIceBox = inIceBox;
	_resolver = resolver;
	_popup = new PopupMenu();

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    
	    String adapterName = Utils.substitute(descriptor.name,
						  _resolver);
	    
	    addChild(new Adapter(adapterName, descriptor, 
				 _resolver, application, _model));
	}
    }
    
    boolean isEditable()
    {
	return _isEditable;
    }

    boolean inIceBox()
    {
	return _inIceBox;
    }

    void newAdapter(AdapterDescriptor descriptor)
    {
	//
	// Generate a unique child name; ignore substitution for simplicity
	//
	String baseName = descriptor == null ? "NewInstance" : descriptor.name;
	String name = baseName;

	int i = 0;
	while(findChild(name) != null || findChild("*" + name) != null)
	{
	    name = baseName + "-" + (++i);
	}

	if(descriptor == null)
	{
	    CommonBase parent = getParent();
	    String defaultId = (parent instanceof Service || parent instanceof ServiceTemplate) ? 
		"${server}.${service}." + name : "${server}." + name;

	    descriptor = new AdapterDescriptor(
		"NewAdapter",
		defaultId,
		false,
		true,
		new java.util.LinkedList());   
	}
	else
	{
	    descriptor.name = name;
	}

	Adapter adapter = new Adapter(name, descriptor, _resolver, _model);
	try
	{
	    addChild(adapter, true);
	}
	catch(DuplicateIdException e)
	{
	    assert false;
	}
	adapter.setParent(this);
	_model.setSelectionPath(adapter.getPath());
    }

    public void paste(Object descriptor)
    {
	if(_isEditable && !_inIceBox && descriptor instanceof AdapterDescriptor)
	{
	    AdapterDescriptor d = (AdapterDescriptor)descriptor;
	    try
	    {
		newAdapter((AdapterDescriptor)d.clone());
	    }
	    catch(CloneNotSupportedException e)
	    {
		assert false;
	    }
	}
    }

    void addDescriptor(AdapterDescriptor descriptor)
    {
	_descriptors.add(descriptor);
    }

    void removeDescriptor(AdapterDescriptor descriptor)
    {
	//
	// A straight remove uses equals(), which is not the desired behavior
	//
	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    if(descriptor == p.next())
	    {
		p.remove();
		break;
	    }
	}
    }

    private java.util.List _descriptors;
    private Utils.Resolver _resolver;
    private boolean _isEditable;
    private boolean _inIceBox;
    static private PopupMenu _popup;
}
