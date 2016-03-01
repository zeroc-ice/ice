// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import test.Ice.location.Test._TestLocatorRegistryDisp;

public class ServerLocatorRegistry extends _TestLocatorRegistryDisp
{
    @Override
    public void
    setAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setAdapterDirectProxy cb, String adapter,
                                Ice.ObjectPrx object, Ice.Current current)
    {
        if(object != null)
        {
            _adapters.put(adapter, object);
        }
        else
        {
            _adapters.remove(adapter);
        }
        cb.ice_response();
    }

    @Override
    public void
    setReplicatedAdapterDirectProxy_async(Ice.AMD_LocatorRegistry_setReplicatedAdapterDirectProxy cb, String adapter, 
                                          String replica, Ice.ObjectPrx object, Ice.Current current)
    {
        if(object != null)
        {
            _adapters.put(adapter, object);
            _adapters.put(replica, object);
        }
        else
        {
            _adapters.remove(adapter);
            _adapters.remove(replica);
        }
        cb.ice_response();
    }

    @Override
    public void
    setServerProcessProxy_async(Ice.AMD_LocatorRegistry_setServerProcessProxy cb, String id, Ice.ProcessPrx proxy,
                                Ice.Current current)
    {
    }

    @Override
    public void
    addObject(Ice.ObjectPrx object, Ice.Current current)
    {
        _objects.put(object.ice_getIdentity(), object);
    }

    public Ice.ObjectPrx
    getAdapter(String adapter)
        throws Ice.AdapterNotFoundException
    {
        Ice.ObjectPrx obj = _adapters.get(adapter);
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
        Ice.ObjectPrx obj = _objects.get(id);
        if(obj == null)
        {
            throw new Ice.ObjectNotFoundException();   
        }
        return obj;
    }

    private java.util.HashMap<String, Ice.ObjectPrx> _adapters = new java.util.HashMap<String, Ice.ObjectPrx>();
    private java.util.HashMap<Ice.Identity, Ice.ObjectPrx> _objects =
        new java.util.HashMap<Ice.Identity, Ice.ObjectPrx>();
}
