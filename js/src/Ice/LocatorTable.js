// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

var Ice = require("../Ice/ModuleRegistry").Ice;
Ice.__M.require(module, ["../Ice/Class", "../Ice/HashMap", "../Ice/Debug"]);

var HashMap = Ice.HashMap;
var Debug = Ice.Debug;

var LocatorTable = Ice.Class({
    __init__: function()
    {
        this._adapterEndpointsTable = new HashMap(); // Map<String, EndpointTableEntry>
        this._objectTable = new HashMap(HashMap.compareEquals); // Map<Ice.Identity, ReferenceTableEntry>
    },
    clear: function()
    {
        this._adapterEndpointsTable.clear();
        this._objectTable.clear();
    },
    getAdapterEndpoints: function(adapter, ttl, cached)
    {
        if(ttl === 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        var entry = this._adapterEndpointsTable.get(adapter);
        if(entry !== undefined)
        {
            cached.value = this.checkTTL(entry.time, ttl);
            return entry.endpoints;
        }
        cached.value = false;
        return null;
    },
    addAdapterEndpoints: function(adapter, endpoints)
    {
        this._adapterEndpointsTable.set(adapter, new EndpointTableEntry(Date.now(), endpoints));
    },
    removeAdapterEndpoints: function(adapter)
    {
        var entry = this._adapterEndpointsTable.get(adapter);
        this._adapterEndpointsTable.delete(adapter);
        return entry !== undefined ? entry.endpoints : null;
    },
    getObjectReference: function(id, ttl, cached)
    {
        if(ttl === 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        var entry = this._objectTable.get(id);
        if(entry !== undefined)
        {
            cached.value = this.checkTTL(entry.time, ttl);
            return entry.reference;
        }
        cached.value = false;
        return null;
    },
    addObjectReference: function(id, ref)
    {
        this._objectTable.set(id, new ReferenceTableEntry(Date.now(), ref));
    },
    removeObjectReference: function(id)
    {
        var entry = this._objectTable.get(id);
        this._objectTable.delete(id);
        return entry !== undefined ? entry.reference : null;
    },
    checkTTL: function(time, ttl)
    {
        Debug.assert(ttl !== 0);
        if(ttl < 0) // TTL = infinite
        {
            return true;
        }
        else
        {
            return Date.now() - time <= (ttl * 1000);
        }
    }
});

Ice.LocatorTable = LocatorTable;
module.exports.Ice = Ice;

var EndpointTableEntry = function(time, endpoints)
{
    this.time = time;
    this.endpoints = endpoints;
};

var ReferenceTableEntry = function(time, reference)
{
    this.time = time;
    this.reference = reference;
};
