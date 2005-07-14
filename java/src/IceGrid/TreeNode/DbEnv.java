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

class DbEnv extends Leaf
{
    DbEnv(String dbEnvName, DbEnvDescriptor descriptor, 
	  boolean editable, java.util.Map[] variables,
	  Model model)
    {
	super(dbEnvName, model);
	_descriptor = descriptor;
	_editable = editable;
	_variables = variables;
    }

    private DbEnvDescriptor _descriptor;
    private boolean _editable;
    private java.util.Map[] _variables;
}
