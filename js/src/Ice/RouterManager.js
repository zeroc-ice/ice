// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

const Ice = require("../Ice/ModuleRegistry").Ice;
Ice._ModuleRegistry.require(module, ["../Ice/HashMap", "../Ice/RouterInfo", "../Ice/Router"]);

const HashMap = Ice.HashMap;
const RouterInfo = Ice.RouterInfo;
const RouterPrx = Ice.RouterPrx;

class RouterManager
{
    constructor()
    {
        this._table = new HashMap(HashMap.compareEquals); // Map<Ice.RouterPrx, RouterInfo>
    }

    destroy()
    {
        for(let router of this._table.values())
        {
            router.destroy();
        }
        this._table.clear();
    }

    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    find(rtr)
    {
        if(rtr === null)
        {
            return null;
        }

        //
        // The router cannot be routed.
        //
        const router = RouterPrx.uncheckedCast(rtr.ice_router(null));

        let info = this._table.get(router);
        if(info === undefined)
        {
            info = new RouterInfo(router);
            this._table.set(router, info);
        }

        return info;
    }

    erase(rtr)
    {
        let info = null;
        if(rtr !== null)
        {
            // The router cannot be routed.
            let router = RouterPrx.uncheckedCast(rtr.ice_router(null));

            info = this._table.get(router);
            this._table.delete(router);
        }
        return info;
    }
}
Ice.RouterManager = RouterManager;
module.exports.Ice = Ice;
