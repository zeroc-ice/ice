//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package com.zeroc.Ice;

final class RouterManager {
    RouterManager() {}

    synchronized void destroy() {
        for (RouterInfo info : _table.values()) {
            info.destroy();
        }
        _table.clear();
    }

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    public RouterInfo get(RouterPrx rtr) {
        if (rtr == null) {
            return null;
        }

        //
        // The router cannot be routed.
        //
        RouterPrx router = RouterPrx.uncheckedCast(rtr.ice_router(null));

        synchronized (this) {
            RouterInfo info = _table.get(router);
            if (info == null) {
                info = new RouterInfo(router);
                _table.put(router, info);
            }

            return info;
        }
    }

    public RouterInfo erase(RouterPrx rtr) {
        RouterInfo info = null;
        if (rtr != null) {
            // The router cannot be routed.
            RouterPrx router = RouterPrx.uncheckedCast(rtr.ice_router(null));

            synchronized (this) {
                info = _table.remove(router);
            }
        }
        return info;
    }

    private java.util.HashMap<RouterPrx, RouterInfo> _table = new java.util.HashMap<>();
}
