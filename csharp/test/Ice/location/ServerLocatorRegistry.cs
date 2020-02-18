//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Threading.Tasks;

namespace Ice.location
{
    public class ServerLocatorRegistry : Test.ITestLocatorRegistry
    {
        public ServerLocatorRegistry()
        {
            _adapters = new Hashtable();
            _objects = new Hashtable();
        }

        public ValueTask SetAdapterDirectProxyAsync(string adapter, Ice.IObjectPrx obj, Ice.Current current)
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

        public ValueTask SetReplicatedAdapterDirectProxyAsync(string adapter, string replica, IObjectPrx obj,
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

        public ValueTask SetServerProcessProxyAsync(string id, IProcessPrx proxy, Current current)
            => new ValueTask(Task.CompletedTask);

        public void addObject(IObjectPrx obj, Current current) => addObject(obj);
        public void addObject(IObjectPrx obj) => _objects[obj.Identity] = obj;

        public IObjectPrx getAdapter(string adapter)
        {
            object obj = _adapters[adapter];
            if (obj == null)
            {
                throw new AdapterNotFoundException();
            }
            return (ObjectPrx)obj;
        }

        public IObjectPrx getObject(Ice.Identity id)
        {
            object obj = _objects[id];
            if (obj == null)
            {
                throw new Ice.ObjectNotFoundException();
            }
            return (ObjectPrx)obj;
        }

        private Hashtable _adapters;
        private Hashtable _objects;
    }
}
