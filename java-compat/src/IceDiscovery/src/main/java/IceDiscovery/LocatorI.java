// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package IceDiscovery;

class LocatorI extends Ice._LocatorDisp
{
    public LocatorI(LookupI lookup, Ice.LocatorRegistryPrx registry)
    {
        _lookup = lookup;
        _registry = registry;
    }

    @Override
    public void
    findObjectById_async(Ice.AMD_Locator_findObjectById cb, Ice.Identity id, Ice.Current current)
    {
        _lookup.findObject(cb, id);
    }

    @Override
    public void
    findAdapterById_async(Ice.AMD_Locator_findAdapterById cb, String adapterId, Ice.Current current)
    {
        _lookup.findAdapter(cb, adapterId);
    }

    @Override
    public Ice.LocatorRegistryPrx
    getRegistry(Ice.Current current)
    {
        return _registry;
    }

    private final LookupI _lookup;
    private final Ice.LocatorRegistryPrx _registry;
}
