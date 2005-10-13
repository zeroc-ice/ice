// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import javax.swing.JMenuItem;
import javax.swing.JPopupMenu;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class Adapters extends ListParent
{
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


    //
    // Actions
    //
    public boolean[] getAvailableActions()
    {
	boolean[] actions = new boolean[ACTION_COUNT];

	Object descriptor =  _model.getClipboard();
	if(descriptor != null)
	{
	    actions[PASTE] = canHaveNewChild() && 
		descriptor instanceof AdapterDescriptor;
	}

	actions[NEW_ADAPTER] = canHaveNewChild();
	return actions;
    }

    public JPopupMenu getPopupMenu()
    {
	if(_popup == null)
	{
	    _popup = new PopupMenu(_model);
	    JMenuItem item = new JMenuItem(_model.getActions()[NEW_ADAPTER]);
	    item.setText("New adapter");
	    _popup.add(item);
	}
	if(canHaveNewChild())
	{
	    return _popup;
	}
	else
	{
	    return null;
	}
    }
    
    public void newAdapter()
    {
	String name = makeNewChildId("NewAdapter");
	AdapterDescriptor descriptor = new AdapterDescriptor(
	    name,
	    "",
	    null,
	    "",
	    false,
	    true,
	    new java.util.LinkedList());   
	
	newAdapter(descriptor);
    }

    public void paste()
    {
	Object descriptor =  _model.getClipboard();
	assert canHaveNewChild() && 
	    descriptor instanceof AdapterDescriptor;

	AdapterDescriptor d = Adapter.copyDescriptor(
	    (AdapterDescriptor)descriptor);
	d.name = makeNewChildId(d.name);
	newAdapter(d);
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
	    addChild(createAdapter(descriptor));
	}
    }

    boolean isEditable()
    {
	return _isEditable;
    }
    
    CommonBase addNewChild(Object d) throws UpdateFailedException
    {
	AdapterDescriptor descriptor = (AdapterDescriptor)d;
       
	try
	{
	    Adapter adapter = createAdapter(descriptor);
	    addChild(adapter, true);
	    return adapter;
	}
	catch(UpdateFailedException e)
	{
	    e.addParent(this);
	    throw e;
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

    private void newAdapter(AdapterDescriptor descriptor)
    {
	Adapter adapter = new Adapter(descriptor.name, descriptor, _model);
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

    private Adapter createAdapter(AdapterDescriptor descriptor)
    {
	String adapterName = Utils.substitute(descriptor.name, _resolver);
	return new Adapter(adapterName, descriptor, _resolver, _model);
    }

    private Utils.Resolver _resolver;
    private boolean _isEditable;
    private boolean _inIceBox;
    
    static private JPopupMenu _popup;
}
