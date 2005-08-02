// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.AdapterDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class Adapters extends Parent
{
    //
    // In server or service template
    //
    Adapters(java.util.List descriptors, Model model)
    {
	this(descriptors, true, null, model);
    }
    
    //
    // In server or service instance
    //
    Adapters(java.util.List descriptors, boolean editable, 
	     java.util.Map[] variables,
	     Model model)
    {
	super("Adapters", model);
	_descriptors = descriptors;
	_editable = editable;
	_variables = variables;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    AdapterDescriptor descriptor = (AdapterDescriptor)p.next();
	    
	    String adapterName = descriptor.name;
	    if(_variables != null)
	    {
		adapterName = Utils.substituteVariables(adapterName, 
							_variables);
	    }

	    Adapter child = new Adapter(adapterName, descriptor, 
					_editable, _variables, 
					_model);
	    addChild(child);
	}
    }

    void unregisterAll()
    {
	java.util.Iterator p = _children.iterator();
	while(p.hasNext())
	{
	    Adapter adapter = (Adapter)p.next();
	    adapter.unregister();
	}
    }

    private java.util.List _descriptors;
    private boolean _editable;
    private java.util.Map[] _variables;
}
