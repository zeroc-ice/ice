// **********************************************************************
//
// Copyright (c) 2003
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************
namespace Ice
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
	public bool add(ObjectPrx prx)
	{
	    if(prx == null)
	    {
		return false;
	    }
	    
	    ObjectPrx proxy = prx.ice_default(); // We insert the proxy in it's default form into the routing table.
	    
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
	
	//
	// Returns null if no Proxy exists for the given identity.
	//
	public ObjectPrx get(Identity ident)
	{
	    if(ident.name.Length == 0)
	    {
		return null;
	    }
	    
	    lock(this)
	    {
		return (ObjectPrx)_table[ident];
	    }
	}
	
	private Hashtable _table;
    }

}
