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
    DbEnvs(DbEnvDescriptor[] descriptors, boolean inTemplate)
    {
	_descriptors = descriptors;
	_inTemplate = inTemplate;

	for(int i = 0; i < _descriptors.length; ++i)
	{
	    addChild(new DbEnv(_descriptors[i], _inTemplate));
	}
    }
    
    public String toString()
    {
	return "DbEnvs";
    }

    private DbEnvDescriptor[] _descriptors;
    private boolean _inTemplate;
}
