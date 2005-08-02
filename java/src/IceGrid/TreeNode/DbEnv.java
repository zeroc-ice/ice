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

class DbEnv extends Leaf
{
    DbEnv(String dbEnvName, DbEnvDescriptor descriptor, 
	  boolean editable, Utils.Resolver resolver,
	  Model model)
    {
	super(dbEnvName, model);
	_descriptor = descriptor;
	_editable = editable;
	_resolver = resolver;
    }

    private DbEnvDescriptor _descriptor;
    private boolean _editable;
    private Utils.Resolver _resolver;
}
