// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

using System.Collections;
using System.Threading.Tasks;

namespace Ice
{
    namespace location
    {
        public class ServerLocatorRegistry : Test.TestLocatorRegistryDisp_
        {
            public ServerLocatorRegistry()
            {
                _adapters = new Hashtable();
                _objects = new Hashtable();
            }

            public override Task
            setAdapterDirectProxyAsync(string adapter, Ice.ObjectPrx obj, Ice.Current current)
            {
                if(obj != null)
                {
                    _adapters[adapter] = obj;
                }
                else
                {
                    _adapters.Remove(adapter);
                }
                return null;
            }

            public override Task
            setReplicatedAdapterDirectProxyAsync(string adapter, string replica, Ice.ObjectPrx obj,
                Ice.Current current)
            {
                if(obj != null)
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

            public override Task
            setServerProcessProxyAsync(string id, Ice.ProcessPrx proxy, Ice.Current current)
            {
                return null;
            }

            public override void addObject(Ice.ObjectPrx obj, Ice.Current current = null)
            {
                _objects[obj.ice_getIdentity()] = obj;
            }

            public virtual Ice.ObjectPrx getAdapter(string adapter)
            {
                object obj = _adapters[adapter];
                if(obj == null)
                {
                    throw new Ice.AdapterNotFoundException();
                }
                return(Ice.ObjectPrx)obj;
            }

            public virtual Ice.ObjectPrx getObject(Ice.Identity id)
            {
                object obj = _objects[id];
                if(obj == null)
                {
                    throw new Ice.ObjectNotFoundException();
                }
                return(Ice.ObjectPrx)obj;
            }

            private Hashtable _adapters;
            private Hashtable _objects;
        }
    }
}
