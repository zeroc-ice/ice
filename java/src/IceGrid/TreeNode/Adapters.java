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
			_adapters.newAdapter();
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
	if(_isEditable)
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
	     Utils.Resolver resolver, Application application, 
	     Model model)
	throws DuplicateIdException
    {
	super("Adapters", model);
	_descriptors = descriptors;
	_isEditable = isEditable;
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

    void newAdapter()
    {
	String name = "NewAdapter";
	int i = 0;
	while(findChild(name) != null)
	{
	    name = "NewAdapter-" + (++i);
	}
	Adapter adapter = new Adapter(name, _resolver, _model);
	adapter.setParent(this);
	_model.setSelectionPath(getPath());
	adapter.displayProperties();
    }

    void addDescriptor(AdapterDescriptor descriptor)
    {
	_descriptors.add(descriptor);
    }

    void popDescriptor()
    {
	_descriptors.remove(_descriptors.size() - 1);
    }

    private java.util.List _descriptors;
    private Utils.Resolver _resolver;
    private boolean _isEditable;
    static private PopupMenu _popup;
}
