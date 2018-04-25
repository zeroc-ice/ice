// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Diagnostics;
using System.Threading.Tasks;

public class ServerLocator : Test.TestLocatorDisp_
{
    public ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    public override Task<Ice.ObjectPrx>
    findAdapterByIdAsync(string adapter,  Ice.Current current)
    {
        ++_requestCount;
        if(adapter.Equals("TestAdapter10") || adapter.Equals("TestAdapter10-2"))
        {
            Debug.Assert(current.encoding.Equals(Ice.Util.Encoding_1_0));
            return Task.FromResult<Ice.ObjectPrx>(_registry.getAdapter("TestAdapter"));
        }
        else
        {
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return Task.FromResult<Ice.ObjectPrx>(_registry.getAdapter(adapter));
        }
    }

    public override Task<Ice.ObjectPrx>
    findObjectByIdAsync(Ice.Identity id,  Ice.Current current)
    {
        ++_requestCount;
        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        System.Threading.Thread.Sleep(1);
        return Task.FromResult<Ice.ObjectPrx>(_registry.getObject(id));
    }

    public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current)
    {
        return _registryPrx;
    }

    public override int getRequestCount(Ice.Current current)
    {
        return _requestCount;
    }

    private ServerLocatorRegistry _registry;
    private Ice.LocatorRegistryPrx _registryPrx;
    private int _requestCount;
}
