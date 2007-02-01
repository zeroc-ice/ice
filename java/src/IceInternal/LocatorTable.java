// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
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
    getAdapterEndpoints(String adapter, int ttl)
    {
        if(ttl == 0) // Locator cache disabled.
        {
            return null;
        }

        EndpointTableEntry entry = (EndpointTableEntry)_adapterEndpointsTable.get(adapter);
        if(entry != null && checkTTL(entry.time, ttl))
        {
            return entry.endpoints;
        }
        return null;
    }

    synchronized void
    addAdapterEndpoints(String adapter, IceInternal.EndpointI[] endpoints)
    {
        _adapterEndpointsTable.put(adapter, new EndpointTableEntry(System.currentTimeMillis(), endpoints));
    }

    synchronized IceInternal.EndpointI[]
    removeAdapterEndpoints(String adapter)
    {
        EndpointTableEntry entry = (EndpointTableEntry)_adapterEndpointsTable.remove(adapter);
        return entry != null ? entry.endpoints : null;
    }

    synchronized Ice.ObjectPrx
    getProxy(Ice.Identity id, int ttl)
    {
        if(ttl == 0) // Locator cache disabled.
        {
            return null;
        }

        ProxyTableEntry entry = (ProxyTableEntry)_objectTable.get(id);
        if(entry != null && checkTTL(entry.time, ttl))
        {
            return entry.proxy;
        }
        return null;
    }

    synchronized void
    addProxy(Ice.Identity id, Ice.ObjectPrx proxy)
    {
        _objectTable.put(id, new ProxyTableEntry(System.currentTimeMillis(), proxy));
    }

    synchronized Ice.ObjectPrx
    removeProxy(Ice.Identity id)
    {
        ProxyTableEntry entry = (ProxyTableEntry)_objectTable.remove(id);
        return entry != null ? entry.proxy : null;
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
            return System.currentTimeMillis() - time <= ((long)ttl * 1000);
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
    };

    private static final class ProxyTableEntry
    {
        public ProxyTableEntry(long time, Ice.ObjectPrx proxy)
        {
            this.time = time;
            this.proxy = proxy;
        }

        final public long time;
        final public Ice.ObjectPrx proxy;
    };

    private java.util.HashMap _adapterEndpointsTable = new java.util.HashMap();
    private java.util.HashMap _objectTable = new java.util.HashMap();
}

