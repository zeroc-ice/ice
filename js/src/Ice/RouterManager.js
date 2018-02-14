// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/HashMap", "../Ice/RouterInfo", "../Ice/Router"]);

var HashMap = Ice.HashMap;
var RouterInfo = Ice.RouterInfo;
var RouterPrx = Ice.RouterPrx;

var RouterManager = Ice.Class({
    __init__: function()
    {
        this._table = new HashMap(HashMap.compareEquals); // Map<Ice.RouterPrx, RouterInfo>
    },
    destroy: function()
    {
        for(var e = this._table.entries; e !== null; e = e.next)
        {
            e.value.destroy();
        }
        this._table.clear();
    },
    //
    // Returns router info for a given router. Automatically creates
    // the router info if it doesn't exist yet.
    //
    find: function(rtr)
    {
        if(rtr === null)
        {
            return null;
        }

        //
        // The router cannot be routed.
        //
        var router = RouterPrx.uncheckedCast(rtr.ice_router(null));

        var info = this._table.get(router);
        if(info === undefined)
        {
            info = new RouterInfo(router);
            this._table.set(router, info);
        }

        return info;
    },
    erase: function(rtr)
    {
        var info = null;
        if(rtr !== null)
        {
            // The router cannot be routed.
            var router = RouterPrx.uncheckedCast(rtr.ice_router(null));

            info = this._table.get(router);
            this._table.delete(router);
        }
        return info;
    }
});
Ice.RouterManager = RouterManager;
module.exports.Ice = Ice;
