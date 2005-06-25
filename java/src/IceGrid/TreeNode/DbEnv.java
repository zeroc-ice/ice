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

class DbEnv extends Leaf
{
    DbEnv(DbEnvDescriptor descriptor, boolean inTemplate)
    {
	super(descriptor.name);

	_descriptor = descriptor;
	_inTemplate = inTemplate;
    }

    private DbEnvDescriptor _descriptor;
    private boolean _inTemplate;
}
