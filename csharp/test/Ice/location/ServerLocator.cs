//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Threading.Tasks;

namespace Ice.location
{
    public class ServerLocator : Test.ITestLocator
    {
        public ServerLocator(ServerLocatorRegistry registry, ILocatorRegistryPrx registryPrx)
        {
            _registry = registry;
            _registryPrx = registryPrx;
            _requestCount = 0;
        }

        public ValueTask<IObjectPrx?>
        FindAdapterByIdAsync(string adapter, Current current)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return new ValueTask<IObjectPrx?>(_registry.getAdapter(adapter));
        }

        public ValueTask<IObjectPrx?>
        FindObjectByIdAsync(Identity id, Current current)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return new ValueTask<IObjectPrx?>(_registry.getObject(id));
        }

        public ILocatorRegistryPrx GetRegistry(Current current) => _registryPrx;

        public int getRequestCount(Current current) => _requestCount;

        private ServerLocatorRegistry _registry;
        private ILocatorRegistryPrx _registryPrx;
        private int _requestCount;
    }
}
