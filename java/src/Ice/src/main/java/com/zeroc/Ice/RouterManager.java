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
    public RouterInfo get(RouterPrx router) {
        if (router == null) {
            return null;
        }

        //
        // The router cannot be routed.
        //
        router = RouterPrx.uncheckedCast(router.ice_router(null));

        synchronized (this) {
            RouterInfo info = _table.get(router);
            if (info == null) {
                info = new RouterInfo(router);
                _table.put(router, info);
            }

            return info;
        }
    }

    public synchronized void erase(RouterPrx router) {
        assert router.ice_getRouter() == null;
        _table.remove(router);
    }

    private java.util.HashMap<RouterPrx, RouterInfo> _table = new java.util.HashMap<>();
}
