// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
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

        public ValueTask SetAdapterDirectProxyAsync(
            string adapter,
            IObjectPrx? obj,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (obj != null)
                {
                    _adapters[adapter] = obj;
                }
                else
                {
                    _adapters.Remove(adapter);
                }
            }
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask SetReplicatedAdapterDirectProxyAsync(
            string adapter,
            string replica,
            IObjectPrx? obj,
            Current current,
            CancellationToken cancel)
        {
            lock (_mutex)
            {
                if (obj != null)
                {
                    _adapters[adapter] = obj;
                    _adapters[replica] = obj;
                }
                else
                {
                    _adapters.Remove(adapter);
                    _adapters.Remove(replica);
                }
            }
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask SetServerProcessProxyAsync(
            string id,
            IProcessPrx? proxy,
            Current current,
            CancellationToken cancel) => default;

        public void AddObject(IObjectPrx? obj, Current current, CancellationToken cancel)
        {
            TestHelper.Assert(obj != null);
            AddObject(obj);
        }

        public void AddObject(IObjectPrx obj)
        {
            lock (_mutex)
            {
                _objects[obj.Identity] = obj;
            }
        }

        public IObjectPrx GetAdapter(string adapter)
        {
            lock (_mutex)
            {
                if (!_adapters.TryGetValue(adapter, out IObjectPrx? obj))
                {
                    throw new AdapterNotFoundException();
                }
                return obj;
            }
        }

        public IObjectPrx GetObject(Identity id)
        {
            lock (_mutex)
            {
                if (!_objects.TryGetValue(id, out IObjectPrx? obj))
                {
                    throw new ObjectNotFoundException();
                }
                return obj;
            }
        }
    }
}
