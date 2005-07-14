// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

import IceGrid.DbEnvDescriptor;
import IceGrid.Model;
import IceGrid.Utils;

class DbEnvs extends Parent
{
    //
    // In server or service template
    //
    DbEnvs(java.util.List descriptors, Model model)
    {
	this(descriptors, true, null, model);
    }

    //
    // In server or service instance
    //
    DbEnvs(java.util.List descriptors, 
	   boolean editable,
	   java.util.Map[] variables,
	   Model model)
    {
	super("DbEnvs", model);
	_descriptors = descriptors;
	_editable = editable;
	_variables = variables;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    DbEnvDescriptor descriptor = (DbEnvDescriptor)p.next();
	    
	    String dbEnvName = descriptor.name;
	    if(_variables != null)
	    {
		dbEnvName = Utils.substituteVariables(dbEnvName, _variables);
	    }
	    addChild(new DbEnv(dbEnvName, descriptor, _editable, _variables,
			       _model));
	}
    }

    private java.util.List _descriptors;
    private boolean _editable;
    private java.util.Map[] _variables;
  
}
