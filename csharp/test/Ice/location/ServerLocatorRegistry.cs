//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections;
using System.Threading.Tasks;

namespace Ice
{
    namespace location
    {
        public class ServerLocatorRegistry : Test.TestLocatorRegistry
        {
            public ServerLocatorRegistry()
            {
                _adapters = new Hashtable();
                _objects = new Hashtable();
            }

            public Task
            setAdapterDirectProxyAsync(string adapter, Ice.IObjectPrx obj, Ice.Current current)
            {
                if (obj != null)
                {
                    _adapters[adapter] = obj;
                }
                else
                {
                    _adapters.Remove(adapter);
                }
                return null;
            }

            public Task
            setReplicatedAdapterDirectProxyAsync(string adapter, string replica, Ice.IObjectPrx obj,
                Ice.Current current)
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
                return null;
            }

            public Task
            setServerProcessProxyAsync(string id, Ice.ProcessPrx proxy, Ice.Current current)
            {
                return null;
            }

            public void addObject(Ice.IObjectPrx obj, Ice.Current current)
            {
                addObject(obj);
            }
            public void addObject(Ice.IObjectPrx obj)
            {
                _objects[obj.Identity] = obj;
            }

            public Ice.IObjectPrx getAdapter(string adapter)
            {
                object obj = _adapters[adapter];
                if (obj == null)
                {
                    throw new Ice.AdapterNotFoundException();
                }
                return (Ice.ObjectPrx)obj;
            }

            public Ice.IObjectPrx getObject(Ice.Identity id)
            {
                object obj = _objects[id];
                if (obj == null)
                {
                    throw new Ice.ObjectNotFoundException();
                }
                return (Ice.ObjectPrx)obj;
            }

            private Hashtable _adapters;
            private Hashtable _objects;
        }
    }
}
