// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package com.zeroc.IceDiscovery;

class LookupReplyI implements LookupReply
{
    public LookupReplyI(LookupI lookup)
    {
        _lookup = lookup;
    }

    @Override
    public void foundObjectById(com.zeroc.Ice.Identity id, com.zeroc.Ice.ObjectPrx proxy, com.zeroc.Ice.Current current)
    {
        _lookup.foundObject(id, proxy);
    }

    @Override
    public void foundAdapterById(String adapterId, com.zeroc.Ice.ObjectPrx proxy, boolean isReplicaGroup,
                                 com.zeroc.Ice.Current current)
    {
        _lookup.foundAdapter(adapterId, proxy, isReplicaGroup);
    }

    private final LookupI _lookup;
}
