// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice-E is licensed to you under the terms described in the
// ICEE_LICENSE file included in this distribution.
//
// **********************************************************************

public class ServerLocatorRegistry extends Ice._LocatorRegistryDisp
{
    public void
    setAdapterDirectProxy(String adapter, Ice.ObjectPrx object, Ice.Current current)
    {
        if(object == null)
        {
            _adapters.remove(adapter);
        }
        else
        {
            _adapters.put(adapter, object);
        }
    }

    public void
    setReplicatedAdapterDirectProxy(String adapter, String replicaId, Ice.ObjectPrx object, Ice.Current current)
    {
        if(object == null)
        {
            _adapters.remove(adapter);
            _adapters.remove(replicaId);
        }
        else
        {
            _adapters.put(adapter, object);
            _adapters.put(replicaId, object);
        }
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

    public void
    addObject(Ice.ObjectPrx object)
    {
	_objects.put(object.ice_getIdentity(), object);
    }
    
    private java.util.Hashtable _adapters = new java.util.Hashtable();
    private java.util.Hashtable _objects = new java.util.Hashtable();
}
