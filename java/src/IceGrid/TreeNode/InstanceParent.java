// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

interface InstanceParent
{
    Object rebuild(CommonBase child, java.util.List editables) 
	throws UpdateFailedException;

    void restore(CommonBase child, Object backup);
}
