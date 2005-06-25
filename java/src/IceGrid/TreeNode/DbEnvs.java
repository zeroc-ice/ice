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

class DbEnvs extends Parent
{
    DbEnvs(java.util.List descriptors, boolean inTemplate)
    {
	super("DbEnvs");
	_descriptors = descriptors;
	_inTemplate = inTemplate;

	java.util.Iterator p = _descriptors.iterator();
	while(p.hasNext())
	{
	    DbEnvDescriptor descriptor = (DbEnvDescriptor)p.next();
	    addChild(new DbEnv(descriptor, _inTemplate));
	}
    }

    private java.util.List _descriptors;
    private boolean _inTemplate;
}
