// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceDiscovery;

class LookupReplyI extends _LookupReplyDisp
{
    public LookupReplyI(LookupI lookup)
    {
        _lookup = lookup;
    }

    @Override
    public void
    foundObjectById(Ice.Identity id, Ice.ObjectPrx proxy, Ice.Current current)
    {
        _lookup.foundObject(id, proxy);
    }

    @Override
    public void
    foundAdapterById(String adapterId, Ice.ObjectPrx proxy, boolean isReplicaGroup, Ice.Current current)
    {
        _lookup.foundAdapter(adapterId, proxy, isReplicaGroup);
    }

    private final LookupI _lookup;
}

