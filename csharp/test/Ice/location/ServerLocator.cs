// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;

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
            Debug.Assert(current.Protocol == Protocol.Ice1);
            return ResolveLocation(new string[] { adapter }, Protocol.Ice1, current, cancel);
        }

        public IObjectPrx? FindObjectById(Identity id, Current current, CancellationToken cancel)
        {
            Debug.Assert(current.Protocol == Protocol.Ice1);
            return ResolveWellKnownProxy(id, Protocol.Ice1, current, cancel);
        }

        public ILocatorRegistryPrx GetRegistry(Current current, CancellationToken cancel) => _registryPrx;

        public int GetRequestCount(Current current, CancellationToken cancel) => _requestCount;

        public IObjectPrx? ResolveLocation(
            string[] location,
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);

            return _registry.GetAdapter(location[0]);
        }

        public IObjectPrx? ResolveWellKnownProxy(
            Identity identity,
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            ++_requestCount;
            // We add a small delay to make sure locator request queuing gets tested when
            // running the test on a fast machine
            System.Threading.Thread.Sleep(1);

            return _registry.GetObject(identity);
        }
    }
}
