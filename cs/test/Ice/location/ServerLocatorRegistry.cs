// **********************************************************************
//
// Copyright (c) 2003 - 2004
// ZeroC, Inc.
// North Palm Beach, FL, USA
//
// All Rights Reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
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
