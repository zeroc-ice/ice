// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceGridGUI.Application;

public class UpdateFailedException extends Exception
{
    UpdateFailedException(TreeNode parent, String id)
    {
        _message = parent.getFullId().toString() + " has already a child with '" + id + "' as ID.";
    }

    UpdateFailedException(String message)
    {
        _message = message;
    }
    
    @Override
    public String toString()
    {
        return _message;
    }

    private String _message;
}
