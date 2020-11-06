// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;

namespace ZeroC.Ice.Test.Location
{
    public class ServerLocator : ITestLocator
    {
        private readonly ServerLocatorRegistry _registry;
        private readonly ILocatorRegistryPrx _registryPrx;
        private int _requestCount;

        public ServerLocator(ServerLocatorRegistry registry, ILocatorRegistryPrx registryPrx)
        {
            _registry = registry;
            _registryPrx = registryPrx;
            _requestCount = 0;
        }

        public IObjectPrx? FindAdapterById(string adapter, Current current, CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            Thread.Sleep(1);

            return _registry.GetIce1Adapter(adapter);
        }

        public IObjectPrx? FindObjectById(Identity id, string? facet, Current current, CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            Thread.Sleep(1);

            return _registry.GetIce1Object(id, facet ?? "");
        }

        public ILocatorRegistryPrx GetRegistry(Current current, CancellationToken cancel) => _registryPrx;

        public int GetRequestCount(Current current, CancellationToken cancel) => _requestCount;

        public IEnumerable<EndpointData> ResolveLocation(string[] location, Current current, CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);

            return _registry.GetIce2Adapter(location[0]);
        }

        public (IEnumerable<EndpointData>, IEnumerable<string>) ResolveWellKnownProxy(
            Identity identity,
            string facet,
            Current current,
            CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            Thread.Sleep(1);

            return _registry.GetIce2Object(identity, facet);
        }
    }
}
