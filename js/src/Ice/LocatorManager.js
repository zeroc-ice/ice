// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module,
    [
        "../Ice/Class", 
        "../Ice/HashMap", 
        "../Ice/LocatorInfo", 
        "../Ice/LocatorTable", 
        "../Ice/Locator"
    ]);

var HashMap = Ice.HashMap;
var LocatorInfo = Ice.LocatorInfo;
var LocatorTable = Ice.LocatorTable;
var LocatorPrx = Ice.LocatorPrx;

var LocatorManager = Ice.Class({
    __init__: function(properties)
    {
        this._background = properties.getPropertyAsInt("Ice.BackgroundLocatorCacheUpdates") > 0;

        this._table = new HashMap(HashMap.compareEquals); // Map<Ice.LocatorPrx, LocatorInfo>
        this._locatorTables = new HashMap(HashMap.compareEquals); // Map<Ice.Identity, LocatorTable>
    },
    destroy: function()
    {
        for(var e = this._table.entries; e !== null; e = e.next)
        {
            e.value.destroy();
        }
        this._table.clear();
        this._locatorTables.clear();
    },
    //
    // Returns locator info for a given locator. Automatically creates
    // the locator info if it doesn't exist yet.
    //
    find: function(loc)
    {
        if(loc === null)
        {
            return null;
        }

        //
        // The locator can't be located.
        //
        var locator = LocatorPrx.uncheckedCast(loc.ice_locator(null));

        //
        // TODO: reap unused locator info objects?
        //

        var info = this._table.get(locator);
        if(info === undefined)
        {
            //
            // Rely on locator identity for the adapter table. We want to
            // have only one table per locator (not one per locator
            // proxy).
            //
            var table = this._locatorTables.get(locator.ice_getIdentity());
            if(table === undefined)
            {
                table = new LocatorTable();
                this._locatorTables.set(locator.ice_getIdentity(), table);
            }

            info = new LocatorInfo(locator, table, this._background);
            this._table.set(locator, info);
        }

        return info;
    }
});

Ice.LocatorManager = LocatorManager;
module.exports.Ice = Ice;
