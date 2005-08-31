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
    DbEnvs(java.util.List descriptors, 
	   boolean isEditable,
	   Utils.Resolver resolver,
	   Model model)
	throws DuplicateIdException
    {
	super("DbEnvs", model);
	_isEditable = isEditable;
	java.util.Iterator p = descriptors.iterator();
	while(p.hasNext())
	{
	    DbEnvDescriptor descriptor = (DbEnvDescriptor)p.next();
	    
	    String dbEnvName = Utils.substitute(descriptor.name, resolver);
	    addChild(new DbEnv(dbEnvName, descriptor, resolver, _model));
	}
    }

    private boolean _isEditable;
}
