//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

const Ice = require("../Ice/ModuleRegistry").Ice;

require("../Ice/Debug");
require("../Ice/HashMap");
require("../Ice/IdentityUtil");

const Debug = Ice.Debug;
const HashMap = Ice.HashMap;

class EndpointTableEntry
{
    constructor(time, endpoints)
    {
        this.time = time;
        this.endpoints = endpoints;
    }
}

class ReferenceTableEntry
{
    constructor(time, reference)
    {
        this.time = time;
        this.reference = reference;
    }
}

class LocatorTable
{
    constructor()
    {
        this._adapterEndpointsTable = new Map(); // Map<String, EndpointTableEntry>
        this._objectTable = new HashMap(HashMap.compareEquals); // Map<Ice.Identity, ReferenceTableEntry>
    }

    clear()
    {
        this._adapterEndpointsTable.clear();
        this._objectTable.clear();
    }

    getAdapterEndpoints(adapter, ttl, cached)
    {
        if(ttl === 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        const entry = this._adapterEndpointsTable.get(adapter);
        if(entry !== undefined)
        {
            cached.value = this.checkTTL(entry.time, ttl);
            return entry.endpoints;
        }
        cached.value = false;
        return null;
    }

    addAdapterEndpoints(adapter, endpoints)
    {
        this._adapterEndpointsTable.set(adapter, new EndpointTableEntry(Date.now(), endpoints));
    }

    removeAdapterEndpoints(adapter)
    {
        const entry = this._adapterEndpointsTable.get(adapter);
        this._adapterEndpointsTable.delete(adapter);
        return entry !== undefined ? entry.endpoints : null;
    }

    getObjectReference(id, ttl, cached)
    {
        if(ttl === 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        const entry = this._objectTable.get(id);
        if(entry !== undefined)
        {
            cached.value = this.checkTTL(entry.time, ttl);
            return entry.reference;
        }
        cached.value = false;
        return null;
    }

    addObjectReference(id, ref)
    {
        this._objectTable.set(id, new ReferenceTableEntry(Date.now(), ref));
    }

    removeObjectReference(id)
    {
        const entry = this._objectTable.get(id);
        this._objectTable.delete(id);
        return entry !== undefined ? entry.reference : null;
    }

    checkTTL(time, ttl)
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
}

Ice.LocatorTable = LocatorTable;
module.exports.Ice = Ice;
