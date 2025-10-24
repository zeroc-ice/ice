// Copyright (c) ZeroC, Inc.

using System.Collections;

namespace Ice.location;

public class ServerLocatorRegistry : Test.TestLocatorRegistryDisp_
{
    public ServerLocatorRegistry()
    {
        _adapters = new Hashtable();
        _objects = new Hashtable();
    }

    public override void setAdapterDirectProxy(string id, ObjectPrx proxy, Current current)
    {
        if (proxy != null)
        {
            _adapters[id] = proxy;
        }
        else
        {
            _adapters.Remove(id);
        }
    }

    public override void setReplicatedAdapterDirectProxy(
        string adapterId,
        string replicaGroupId,
        ObjectPrx proxy,
        Current current)
    {
        if (proxy != null)
        {
            _adapters[adapterId] = proxy;
            _adapters[replicaGroupId] = proxy;
        }
        else
        {
            _adapters.Remove(adapterId);
            _adapters.Remove(replicaGroupId);
        }
    }

    public override void setServerProcessProxy(string id, Ice.ProcessPrx proxy, Ice.Current current)
    {
    }

    public override void addObject(Ice.ObjectPrx obj, Ice.Current current) => addObject(obj);

    internal void addObject(Ice.ObjectPrx obj) => _objects[obj.ice_getIdentity()] = obj;

    internal Ice.ObjectPrx getAdapter(string adapter)
    {
        object obj = _adapters[adapter] ?? throw new Ice.AdapterNotFoundException();
        return (Ice.ObjectPrx)obj;
    }

    internal Ice.ObjectPrx getObject(Ice.Identity id)
    {
        object obj = _objects[id] ?? throw new Ice.ObjectNotFoundException();
        return (Ice.ObjectPrx)obj;
    }

    private readonly Hashtable _adapters;
    private readonly Hashtable _objects;
}
