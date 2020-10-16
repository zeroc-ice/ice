// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Diagnostics;
using System.Threading;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Location
{
    public class ServerLocatorRegistry : ITestLocatorRegistry
    {
        private readonly Dictionary<string, IObjectPrx> _adapters = new Dictionary<string, IObjectPrx>();
        private readonly object _mutex = new object();
        private readonly Dictionary<Identity, IObjectPrx> _objects = new Dictionary<Identity, IObjectPrx>();

        public void RegisterAdapterEndpoints(
            string adapterId,
            string replicaGroupId,
            IObjectPrx endpoints,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                _adapters[adapterId] = endpoints;
                if (replicaGroupId.Length > 0)
                {
                    _adapters[replicaGroupId] = endpoints;
                }
            }
        }

        public void SetAdapterDirectProxy(
            string adapterId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel)
        {
            Debug.Assert(current.Protocol == Protocol.Ice1);

            if (proxy == null)
            {
                UnregisterAdapterEndpoints(adapterId, replicaGroupId: "", Protocol.Ice1, current, cancel);
            }
            else
            {
                RegisterAdapterEndpoints(adapterId, "", proxy, current, cancel);
            }
        }

        public void SetReplicatedAdapterDirectProxy(
            string adapterId,
            string replicaGroupId,
            IObjectPrx? proxy,
            Current current,
            CancellationToken cancel)
        {
            Debug.Assert(current.Protocol == Protocol.Ice1);

            if (proxy == null)
            {
                UnregisterAdapterEndpoints(adapterId, replicaGroupId, Protocol.Ice1, current, cancel);
            }
            else
            {
                RegisterAdapterEndpoints(adapterId, replicaGroupId, proxy, current, cancel);
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
            Protocol protocol,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                _adapters.Remove(adapterId);
                if (replicaGroupId.Length > 0)
                {
                    _adapters.Remove(replicaGroupId);
                }
            }
        }

        public void AddObject(IObjectPrx obj, Current current, CancellationToken cancel)
        {
            AddObject(obj);
        }

        internal void AddObject(IObjectPrx obj)
        {
            lock (_mutex)
            {
                _objects[obj.Identity] = obj;
            }
        }

        internal IObjectPrx? GetAdapter(string adapter)
        {
            lock (_mutex)
            {
                if (_adapters.TryGetValue(adapter, out IObjectPrx? obj))
                {
                    return obj;
                }
                else
                {
                    return null;
                }
            }
        }

        internal IObjectPrx? GetObject(Identity id)
        {
            lock (_mutex)
            {
                if (_objects.TryGetValue(id, out IObjectPrx? obj))
                {
                    return obj;
                }
                else
                {
                    return null;
                }
            }
        }
    }
}
