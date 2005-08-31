// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package IceGrid.TreeNode;

class AdapterInstanceId
{
    AdapterInstanceId(String serverId, String adapterId)
    {
	this.serverId = serverId;
	this.adapterId = adapterId;
    }
    
    public boolean equals(Object obj)
    {
	try
	{
	    AdapterInstanceId other = (AdapterInstanceId)obj;
	    return serverId.equals(other.serverId) 
		&& adapterId.equals(other.adapterId);
	}
	catch(ClassCastException e)
	{
	    return false;
	}
    }
    
    public int hashCode()
    {
	return serverId.hashCode() ^ adapterId.hashCode();
    }
    
    String serverId;
    String adapterId;
};
