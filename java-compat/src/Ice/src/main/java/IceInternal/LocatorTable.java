// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceInternal;

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

    synchronized IceInternal.EndpointI[]
    getAdapterEndpoints(String adapter, int ttl, Ice.Holder<Boolean> cached)
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
    addAdapterEndpoints(String adapter, IceInternal.EndpointI[] endpoints)
    {
        _adapterEndpointsTable.put(adapter,
                new EndpointTableEntry(IceInternal.Time.currentMonotonicTimeMillis(), endpoints));
    }

    synchronized IceInternal.EndpointI[]
    removeAdapterEndpoints(String adapter)
    {
        EndpointTableEntry entry = _adapterEndpointsTable.remove(adapter);
        return entry != null ? entry.endpoints : null;
    }

    synchronized Reference
    getObjectReference(Ice.Identity id, int ttl, Ice.Holder<Boolean> cached)
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
    addObjectReference(Ice.Identity id, Reference ref)
    {
        _objectTable.put(id, new ReferenceTableEntry(IceInternal.Time.currentMonotonicTimeMillis(), ref));
    }

    synchronized Reference
    removeObjectReference(Ice.Identity id)
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
            return IceInternal.Time.currentMonotonicTimeMillis() - time <= ((long)ttl * 1000);
        }
    }

    private static final class EndpointTableEntry
    {
        public EndpointTableEntry(long time, IceInternal.EndpointI[] endpoints)
        {
            this.time = time;
            this.endpoints = endpoints;
        }

        final public long time;
        final public IceInternal.EndpointI[] endpoints;
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

    private java.util.Map<String, EndpointTableEntry> _adapterEndpointsTable =
        new java.util.HashMap<String, EndpointTableEntry>();
    private java.util.Map<Ice.Identity, ReferenceTableEntry> _objectTable =
        new java.util.HashMap<Ice.Identity, ReferenceTableEntry>();
}
