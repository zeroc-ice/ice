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

namespace IceInternal
{

    using System.Collections;

    public sealed class RouterManager
    {
	internal RouterManager()
	{
	    _table = new Hashtable();
	}
	
	internal void destroy()
	{
	    lock(this)
	    {
		foreach(RouterInfo i in _table.Values)
		{
		    i.destroy();
		}
		_table.Clear();
	    }
	}
	
	//
	// Returns router info for a given router. Automatically creates
	// the router info if it doesn't exist yet.
	//
	public RouterInfo get(Ice.RouterPrx rtr)
	{
	    if(rtr == null)
	    {
		return null;
	    }
	    
	    Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null)); // The router cannot be routed.
	    
	    lock(this)
	    {
		RouterInfo info = (RouterInfo)_table[router];
		if(info == null)
		{
		    info = new RouterInfo(router);
		    _table[router] = info;
		}
		
		return info;
	    }
	}
	
	private Hashtable _table;
    }

}
