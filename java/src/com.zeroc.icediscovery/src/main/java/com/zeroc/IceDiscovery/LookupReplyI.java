// Copyright (c) ZeroC, Inc.

package com.zeroc.IceDiscovery;

import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;

class LookupReplyI implements LookupReply {
    public LookupReplyI(LookupI lookup) {
        _lookup = lookup;
    }

    @Override
    public void foundObjectById(
            Identity id,
            ObjectPrx proxy,
            Current current) {
        _lookup.foundObject(id, current.id.name, proxy);
    }

    @Override
    public void foundAdapterById(
            String adapterId,
            ObjectPrx proxy,
            boolean isReplicaGroup,
            Current current) {
        _lookup.foundAdapter(adapterId, current.id.name, proxy, isReplicaGroup);
    }

    private final LookupI _lookup;
}
