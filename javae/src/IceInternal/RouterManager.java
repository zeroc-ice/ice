// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
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
	java.util.Enumeration e = _table.elements();
        while(e.hasMoreElements())
        {
            RouterInfo info = (RouterInfo)e.nextElement();
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

    public RouterInfo
    erase(Ice.RouterPrx rtr)
    {
	RouterInfo info = null;
	if(rtr != null)
	{
	    // The router cannot be routed.
	    Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

	    synchronized(this)
	    {
		info = (RouterInfo)_table.remove(router);
	    }
	}
	return info;
    }

    private java.util.Hashtable _table = new java.util.Hashtable();
}
