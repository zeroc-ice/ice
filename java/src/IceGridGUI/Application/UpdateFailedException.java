// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGridGUI.Application;

public class UpdateFailedException extends Exception
{
    UpdateFailedException(TreeNode parent, String id)
    {
        _parent = parent;
        _id = id;
    }
    
    public String toString()
    {
        return _parent.getFullId().toString() + " has already a child with '" + _id + "' as ID.";
    }
    
    private TreeNode _parent;
    private String _id;
}
