// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Concurrent;
using System.Collections.Generic;
using System.Collections.Immutable;
using System.Threading;

namespace ZeroC.Ice.Test.Location
{
    public class ServerLocatorRegistry : ITestLocatorRegistry
    {
        private readonly IDictionary<string, IObjectPrx> _ice1Adapters =
            new ConcurrentDictionary<string, IObjectPrx>();

        private readonly IDictionary<(Identity, string), IObjectPrx> _ice1Objects =
            new ConcurrentDictionary<(Identity, string), IObjectPrx>();

        private readonly IDictionary<string, IReadOnlyList<EndpointData>> _ice2Adapters =
            new ConcurrentDictionary<string, IReadOnlyList<EndpointData>>();

        private readonly IDictionary<(Identity, string), (IReadOnlyList<EndpointData>, IReadOnlyList<string>)> _ice2Objects =
            new ConcurrentDictionary<(Identity, string), (IReadOnlyList<EndpointData>, IReadOnlyList<string>)>();

        public void AddObject(IObjectPrx obj, Current current, CancellationToken cancel)
        {
            AddObject(obj);
        }

        public void RegisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            EndpointData[] endpoints,
            Current current,
            CancellationToken cancel)
        {
            _ice2Adapters[adapterId] = endpoints;
            if (replicaGroupId.Length > 0)
            {
                _ice2Adapters[replicaGroupId] = endpoints;
            }
        }

        public void SetAdapterDirectProxy(
            string adapterId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel) =>
            SetReplicatedAdapterDirectProxy(adapterId, "", proxy, current, cancel);

        public void SetReplicatedAdapterDirectProxy(
            string adapterId,
            string replicaGroupId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel)
        {
            if (proxy != null)
            {
                _ice1Adapters[adapterId] = proxy;
                if (replicaGroupId.Length > 0)
                {
                    _ice1Adapters[replicaGroupId] = proxy;
                }
            }
            else
            {
                _ice1Adapters.Remove(adapterId);
                if (replicaGroupId.Length > 0)
                {
                    _ice1Adapters.Remove(replicaGroupId);
                }
            }
        }

        public void SetServerProcessProxy(
            string id,
            IProcessPrx? proxy,
            Current current,
            CancellationToken cancel)
        {
            // Ignored
        }

        public void UnregisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            Current current,
            CancellationToken cancel)
        {
            _ice2Adapters.Remove(adapterId);
            if (replicaGroupId.Length > 0)
            {
                _ice2Adapters.Remove(replicaGroupId);
            }
        }

        internal IObjectPrx? GetIce1Adapter(string adapter) =>
            _ice1Adapters.TryGetValue(adapter, out IObjectPrx? proxy) ? proxy : null;

        internal IObjectPrx? GetIce1Object(Identity id, string facet) =>
            _ice1Objects.TryGetValue((id, facet), out IObjectPrx? obj) ? obj : null;

        internal IReadOnlyList<EndpointData> GetIce2Adapter(string adapter) =>
            _ice2Adapters.TryGetValue(adapter, out IReadOnlyList<EndpointData>? endpoints) ? endpoints :
                ImmutableArray<EndpointData>.Empty;
        internal (IReadOnlyList<EndpointData>, IReadOnlyList<string>) GetIce2Object(Identity id, string facet) =>
            _ice2Objects.TryGetValue((id, facet), out var entry) ? entry :
                (ImmutableArray<EndpointData>.Empty, ImmutableArray<string>.Empty);

        internal void AddObject(IObjectPrx obj)
        {
            if (obj.Protocol == Protocol.Ice1)
            {
                _ice1Objects[(obj.Identity, obj.Facet)] = obj;
            }
            else
            {
                _ice2Objects[(obj.Identity, obj.Facet)] = (obj.Endpoints.ToEndpointDataList(), obj.Location);
            }
        }
    }
}
