// **********************************************************************
//
// Copyright (c) 2002
// ZeroC, Inc.
// Billerica, MA, USA
//
// All Rights Reserved.
//
// Ice is free software; you can redistribute it and/or modify it under
// the terms of the GNU General Public License version 2 as published by
// the Free Software Foundation.
//
// **********************************************************************

public class ServerLocatorRegistry extends Ice._LocatorRegistryDisp
{
    public void
    setAdapterDirectProxy(String adapter, Ice.ObjectPrx object, Ice.Current current)
    {
	_adapters.put(adapter, object);
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
    
    private java.util.HashMap _adapters = new java.util.HashMap();
    private java.util.HashMap _objects = new java.util.HashMap();
}
