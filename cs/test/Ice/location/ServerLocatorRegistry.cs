// **********************************************************************
//
// Copyright (c) 2003
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

using System.Collections;

public class ServerLocatorRegistry : Ice.LocatorRegistry_Disp
{
    public ServerLocatorRegistry()
    {
        _adapters = new Hashtable();
	_objects = new Hashtable();
    }

    public override void setAdapterDirectProxy(string adapter, Ice.ObjectPrx obj, Ice.Current current)
    {
        _adapters[adapter] = obj;
    }

    public override void setServerProcessProxy(string id, Ice.ProcessPrx proxy, Ice.Current current)
    {
	// TODO: missing implementation
    }

    public virtual Ice.ObjectPrx getAdapter(string adapter)
    {
        object obj = _adapters[adapter];
        if(obj == null)
        {
            throw new Ice.AdapterNotFoundException();
        }
        return (Ice.ObjectPrx)obj;
    }
    
    public virtual Ice.ObjectPrx getObject(Ice.Identity id)
    {
        object obj = _objects[id];
        if(obj == null)
        {
            throw new Ice.ObjectNotFoundException();
        }
        return (Ice.ObjectPrx)obj;
    }
    
    public virtual void addObject(Ice.ObjectPrx obj)
    {
        _objects[obj.ice_getIdentity()] = obj;
    }
    
    private Hashtable _adapters;
    private Hashtable _objects;
}
