//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Location
{
    public class ServerLocatorRegistry : ITestLocatorRegistry
    {
        public ValueTask SetAdapterDirectProxyAsync(string adapter, IObjectPrx? obj, Current current)
        {
            if (obj != null)
            {
                _adapters[adapter] = obj;
            }
            else
            {
                _adapters.Remove(adapter);
            }
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask SetReplicatedAdapterDirectProxyAsync(string adapter, string replica, IObjectPrx? obj,
            Current current)
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
            return new ValueTask(Task.CompletedTask);
        }

        public ValueTask SetServerProcessProxyAsync(string id, IProcessPrx? proxy, Current current) =>
            new ValueTask(Task.CompletedTask);

        public void AddObject(IObjectPrx? obj, Current current)
        {
            TestHelper.Assert(obj != null);
            AddObject(obj);
        }

        public void AddObject(IObjectPrx obj) => _objects[obj.Identity] = obj;

        public IObjectPrx GetAdapter(string adapter)
        {
            if (!_adapters.TryGetValue(adapter, out IObjectPrx? obj))
            {
                throw new AdapterNotFoundException();
            }
            return obj;
        }

        public IObjectPrx GetObject(Identity id)
        {
            if (!_objects.TryGetValue(id, out IObjectPrx? obj))
            {
                throw new ObjectNotFoundException();
            }
            return obj;
        }

        private readonly Dictionary<string, IObjectPrx> _adapters = new Dictionary<string, IObjectPrx>();
        private readonly Dictionary<Identity, IObjectPrx> _objects = new Dictionary<Identity, IObjectPrx>();
    }
}
