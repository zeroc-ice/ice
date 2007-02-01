// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerLocatorRegistry extends Test._TestLocatorRegistryDisp
{
    public void
    setAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setAdapterDirectProxy cb, String adapter,
                                Ice.ObjectPrx object, Ice.Current current)
    {
        _adapters.put(adapter, object);
        cb.ice_response();
    }

    public void
    setReplicatedAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setReplicatedAdapterDirectProxy cb, String adapter, 
                                          String replica, Ice.ObjectPrx object, Ice.Current current)
    {
        _adapters.put(adapter, object);
        _adapters.put(replica, object);
        cb.ice_response();
    }

    public void
    setServerProcessProxy_async(Ice.AMD_LocatorRegistry_setServerProcessProxy cb, String id, Ice.ProcessPrx proxy,
                                Ice.Current current)
    {
    }

    public void
    addObject(Ice.ObjectPrx object, Ice.Current current)
    {
        _objects.put(object.ice_getIdentity(), object);
    }

    public Ice.ObjectPrx
    getAdapter(String adapter)
        throws Ice.AdapterNotFoundException
    {
        Ice.ObjectPrx obj = (Ice.ObjectPrx)_adapters.get(adapter);
        if(obj == null)
        {
            throw new Ice.AdapterNotFoundException();
        }
        return obj;
    }

    public Ice.ObjectPrx
    getObject(Ice.Identity id)
        throws Ice.ObjectNotFoundException
    {
        Ice.ObjectPrx obj = (Ice.ObjectPrx)_objects.get(id);
        if(obj == null)
        {
            throw new Ice.ObjectNotFoundException();   
        }
        return obj;
    }

    private java.util.HashMap _adapters = new java.util.HashMap();
    private java.util.HashMap _objects = new java.util.HashMap();
}
