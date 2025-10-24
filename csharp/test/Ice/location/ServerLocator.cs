// Copyright (c) ZeroC, Inc.

using System.Diagnostics;

namespace Ice.location;

public class ServerLocator : Test.TestLocatorDisp_
{
    public ServerLocator(ServerLocatorRegistry registry, Ice.LocatorRegistryPrx registryPrx)
    {
        _registry = registry;
        _registryPrx = registryPrx;
        _requestCount = 0;
    }

    public override Ice.ObjectPrx findAdapterById(string id, Ice.Current current)
    {
        ++_requestCount;
        if (id == "TestAdapter10" || id == "TestAdapter10-2")
        {
            Debug.Assert(current.encoding.Equals(Ice.Util.Encoding_1_0));
            return _registry.getAdapter("TestAdapter");
        }
        else
        {
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return _registry.getAdapter(id);
        }
    }

    public override Ice.ObjectPrx findObjectById(Ice.Identity id, Ice.Current current)
    {
        ++_requestCount;
        // We add a small delay to make sure locator request queuing gets tested when
        // running the test on a fast machine
        System.Threading.Thread.Sleep(1);
        return _registry.getObject(id);
    }

    public override Ice.LocatorRegistryPrx getRegistry(Ice.Current current) => _registryPrx;

    public override int getRequestCount(Ice.Current current) => _requestCount;

    private readonly ServerLocatorRegistry _registry;
    private readonly Ice.LocatorRegistryPrx _registryPrx;
    private int _requestCount;
}
