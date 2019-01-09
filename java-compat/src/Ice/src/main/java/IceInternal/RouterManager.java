// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
        for(RouterInfo info : _table.values())
        {
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

        //
        // The router cannot be routed.
        //
        Ice.RouterPrx router = Ice.RouterPrxHelper.uncheckedCast(rtr.ice_router(null));

        synchronized(this)
        {
            RouterInfo info = _table.get(router);
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
                info = _table.remove(router);
            }
        }
        return info;
    }

    private java.util.HashMap<Ice.RouterPrx, RouterInfo> _table = new java.util.HashMap<Ice.RouterPrx, RouterInfo>();
}
