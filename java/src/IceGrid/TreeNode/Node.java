// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

class Node extends Parent
{
    //
    // Node creation/deletion/renaming is done by starting/restarting
    // an IceGridNode process. Not through admin calls.
    //
    
    //
    // TODO: consider showing per-application node variables
    //

    Node(String name)
    {
	_name = name;
    }

    public String toString()
    {
	return _name;
    }

    private String _name;
}
