// Copyright (c) ZeroC, Inc.

import { HashMap } from "./HashMap.js";
import { Ice as Ice_Router } from "./Router.js";
const { RouterPrx } = Ice_Router;
import { RouterInfo } from "./RouterInfo.js";

export class RouterManager {
    constructor() {
        this._table = new HashMap(HashMap.compareEquals); // Map<Ice.RouterPrx, RouterInfo>
    }

    destroy() {
        for (const router of this._table.values()) {
            router.destroy();
        }
        this._table.clear();
    }

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    find(router) {
        if (router === null) {
            return null;
        }

        // The router cannot be routed.
        router = RouterPrx.uncheckedCast(router.ice_router(null));

        let info = this._table.get(router);
        if (info === undefined) {
            info = new RouterInfo(router);
            this._table.set(router, info);
        }

        return info;
    }

    erase(router) {
        DEV: console.assert(router.ice_getRouter() == null); // The router cannot be routed.
        this._table.delete(router);
    }
}
