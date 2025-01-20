// Copyright (c) ZeroC, Inc.

package com.zeroc.Ice;

import java.time.Duration;

final class LocatorTable {
    LocatorTable() {}

    synchronized void clear() {
        _adapterEndpointsTable.clear();
        _objectTable.clear();
    }

    synchronized EndpointI[] getAdapterEndpoints(
            String adapter, Duration ttl, Holder<Boolean> cached) {
        if (ttl.isZero()) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        EndpointTableEntry entry = _adapterEndpointsTable.get(adapter);
        if (entry != null) {
            cached.value = checkTTL(entry.time, ttl);
            return entry.endpoints;
        }
        cached.value = false;
        return null;
    }

    synchronized void addAdapterEndpoints(String adapter, EndpointI[] endpoints) {
        _adapterEndpointsTable.put(
                adapter, new EndpointTableEntry(Time.currentMonotonicTimeMillis(), endpoints));
    }

    synchronized EndpointI[] removeAdapterEndpoints(String adapter) {
        EndpointTableEntry entry = _adapterEndpointsTable.remove(adapter);
        return entry != null ? entry.endpoints : null;
    }

    synchronized Reference getObjectReference(Identity id, Duration ttl, Holder<Boolean> cached) {
        if (ttl.isZero()) // Locator cache disabled.
        {
            cached.value = false;
            return null;
        }

        ReferenceTableEntry entry = _objectTable.get(id);
        if (entry != null) {
            cached.value = checkTTL(entry.time, ttl);
            return entry.reference;
        }
        cached.value = false;
        return null;
    }

    synchronized void addObjectReference(Identity id, Reference ref) {
        _objectTable.put(id, new ReferenceTableEntry(Time.currentMonotonicTimeMillis(), ref));
    }

    synchronized Reference removeObjectReference(Identity id) {
        ReferenceTableEntry entry = _objectTable.remove(id);
        return entry != null ? entry.reference : null;
    }

    private boolean checkTTL(long time, Duration ttl) {
        assert (!ttl.isZero());
        if (ttl.compareTo(Duration.ZERO) < 0) // TTL = infinite
        {
            return true;
        } else {
            return Time.currentMonotonicTimeMillis() - time <= ttl.toMillis();
        }
    }

    private static final class EndpointTableEntry {
        public EndpointTableEntry(long time, EndpointI[] endpoints) {
            this.time = time;
            this.endpoints = endpoints;
        }

        public final long time;
        public final EndpointI[] endpoints;
    }

    private static final class ReferenceTableEntry {
        public ReferenceTableEntry(long time, Reference reference) {
            this.time = time;
            this.reference = reference;
        }

        public final long time;
        public final Reference reference;
    }

    private java.util.Map<String, EndpointTableEntry> _adapterEndpointsTable =
            new java.util.HashMap<>();
    private java.util.Map<Identity, ReferenceTableEntry> _objectTable = new java.util.HashMap<>();
}
