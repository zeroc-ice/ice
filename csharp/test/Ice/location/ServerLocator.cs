//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;

namespace ZeroC.Ice.Test.Location
{
    public class ServerLocator : ITestLocator
    {
        public ServerLocator(ServerLocatorRegistry registry, ILocatorRegistryPrx registryPrx)
        {
            _registry = registry;
            _registryPrx = registryPrx;
            _requestCount = 0;
        }

        public ValueTask<IObjectPrx?> FindAdapterByIdAsync(string adapter, Current current)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return new ValueTask<IObjectPrx?>(_registry.GetAdapter(adapter).Clone(encoding: current.Encoding));
        }

        public ValueTask<IObjectPrx?> FindObjectByIdAsync(Identity id, Current current)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);
            return new ValueTask<IObjectPrx?>(_registry.GetObject(id).Clone(encoding: current.Encoding));
        }

        public ILocatorRegistryPrx GetRegistry(Current current) => _registryPrx;

        public int getRequestCount(Current current) => _requestCount;

        private readonly ServerLocatorRegistry _registry;
        private readonly ILocatorRegistryPrx _registryPrx;
        private int _requestCount;
    }
}
