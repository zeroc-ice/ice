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

    public override void setAdapterDirectProxy(string adapter, Ice.ObjectPrx obj, Ice.Current current)
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

    public override void setReplicatedAdapterDirectProxy(
        string adapter,
        string replica,
        Ice.ObjectPrx obj,
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
