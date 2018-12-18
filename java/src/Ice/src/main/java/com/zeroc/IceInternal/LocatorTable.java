// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceInternal;

final class LocatorTable
{
    LocatorTable()
    {
    }

    synchronized void
    clear()
    {
        _adapterEndpointsTable.clear();
        _objectTable.clear();
    }

    synchronized EndpointI[]
    getAdapterEndpoints(String adapter, int ttl, Holder<Boolean> cached)
    {
        if(ttl == 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        EndpointTableEntry entry = _adapterEndpointsTable.get(adapter);
        if(entry != null)
        {
            cached.value = checkTTL(entry.time, ttl);
            return entry.endpoints;
        }
        cached.value = false;
        return null;
    }

    synchronized void
    addAdapterEndpoints(String adapter, EndpointI[] endpoints)
    {
        _adapterEndpointsTable.put(adapter,
                new EndpointTableEntry(Time.currentMonotonicTimeMillis(), endpoints));
    }

    synchronized EndpointI[]
    removeAdapterEndpoints(String adapter)
    {
        EndpointTableEntry entry = _adapterEndpointsTable.remove(adapter);
        return entry != null ? entry.endpoints : null;
    }

    synchronized Reference
    getObjectReference(com.zeroc.Ice.Identity id, int ttl, Holder<Boolean> cached)
    {
        if(ttl == 0) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        ReferenceTableEntry entry = _objectTable.get(id);
        if(entry != null)
        {
            cached.value = checkTTL(entry.time, ttl);
            return entry.reference;
        }
        cached.value = false;
        return null;
    }

    synchronized void
    addObjectReference(com.zeroc.Ice.Identity id, Reference ref)
    {
        _objectTable.put(id, new ReferenceTableEntry(Time.currentMonotonicTimeMillis(), ref));
    }

    synchronized Reference
    removeObjectReference(com.zeroc.Ice.Identity id)
    {
        ReferenceTableEntry entry = _objectTable.remove(id);
        return entry != null ? entry.reference : null;
    }

    private boolean
    checkTTL(long time, int ttl)
    {
        assert(ttl != 0);
        if(ttl < 0) // TTL = infinite
        {
            return true;
        }
        else
        {
            return Time.currentMonotonicTimeMillis() - time <= ((long)ttl * 1000);
        }
    }

    private static final class EndpointTableEntry
    {
        public EndpointTableEntry(long time, EndpointI[] endpoints)
        {
            this.time = time;
            this.endpoints = endpoints;
        }

        final public long time;
        final public EndpointI[] endpoints;
    }

    private static final class ReferenceTableEntry
    {
        public ReferenceTableEntry(long time, Reference reference)
        {
            this.time = time;
            this.reference = reference;
        }

        final public long time;
        final public Reference reference;
    }

    private java.util.Map<String, EndpointTableEntry> _adapterEndpointsTable = new java.util.HashMap<>();
    private java.util.Map<com.zeroc.Ice.Identity, ReferenceTableEntry> _objectTable = new java.util.HashMap<>();
}
