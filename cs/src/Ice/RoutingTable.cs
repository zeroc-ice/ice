// **********************************************************************
//
// Copyright (c) 2003-2005 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceInternal
{

    using System.Collections;
	    
    public sealed class RoutingTable
    {
	public RoutingTable()
	{
	    _table = new Hashtable();
	}
	
	//
	// Clear the contents of the routing table.
	//
	public void clear()
	{
	    lock(this)
	    {
		_table.Clear();
	    }
	}
	
	//
	// Returns false if the Proxy exists already.
	//
	public bool add(Ice.ObjectPrx prx)
	{
	    if(prx == null)
	    {
		return false;
	    }
	    
	    //
	    // We insert the proxy in its default form into the routing table.
	    //
	    Ice.ObjectPrx proxy = prx.ice_twoway().ice_secure(false);
	    
	    lock(this)
	    {
		if(!_table.Contains(proxy.ice_getIdentity()))
		{
		    _table[proxy.ice_getIdentity()] = proxy;
		    return true;
		}
		else
		{
		    return false;
		}
	    }
	}

	private Hashtable _table;
    }

}
