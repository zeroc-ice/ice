// **********************************************************************
//
// Copyright (c) 2002
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

package IceInternal;

public final class RouterManager
{
    RouterManager()
    {
    }

    synchronized void
    destroy()
    {
	java.util.Iterator i = _table.values().iterator();
        while(i.hasNext())
        {
            RouterInfo info = (RouterInfo)i.next();
            info.destroy();
        }
        _table.clear();
    }

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    public RouterInfo
    get(Ice.RouterPrx rtr)
    {
        if(rtr == null)
        {
            return null;
        }

        Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null)); // The router cannot be routed.

        synchronized(this)
        {
            RouterInfo info = (RouterInfo)_table.get(router);
            if(info == null)
            {
                info = new RouterInfo(router);
                _table.put(router, info);
            }

            return info;
        }
    }

    private java.util.HashMap _table = new java.util.HashMap();
}
