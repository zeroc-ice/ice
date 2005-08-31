// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

class DuplicateIdException extends Exception
{
    DuplicateIdException(Parent parent, String id)
    {
	_parent = parent;
	_id = id;
    }

    public String toString()
    {
	return _parent.getId() + " cannot have two children with '" 
	    + _id + "' as id";
    }

    private Parent _parent;
    private String _id;
}
