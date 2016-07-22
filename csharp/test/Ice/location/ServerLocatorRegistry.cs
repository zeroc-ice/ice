// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Collections;

public class ServerLocatorRegistry : Test.TestLocatorRegistryDisp_
{
    public ServerLocatorRegistry()
    {
        _adapters = new Hashtable();
        _objects = new Hashtable();
    }

    public override void 
    setAdapterDirectProxyAsync(string adapter, Ice.ObjectPrx obj, Action response, Action<Exception> exception,
                               Ice.Current current)
    {
        if(obj != null)
        {
            _adapters[adapter] = obj;
        }
        else
        {
            _adapters.Remove(adapter);
        }
       response();
    }
  
    public override void
    setReplicatedAdapterDirectProxyAsync(string adapter, string replica, Ice.ObjectPrx obj, Action response,
                                         Action<Exception> exception, Ice.Current current)
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
        response();
    }
  
    public override void
    setServerProcessProxyAsync(string id, Ice.ProcessPrx proxy, Action response, Action<Exception> exception,
                               Ice.Current current)
    {
        response();
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
    
    private Hashtable _adapters;
    private Hashtable _objects;
}
