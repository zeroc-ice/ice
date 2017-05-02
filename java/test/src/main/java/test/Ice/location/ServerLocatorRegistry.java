// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.location;

import java.util.concurrent.CompletionStage;
import java.util.concurrent.CompletableFuture;

import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectPrx;

import test.Ice.location.Test.TestLocatorRegistry;

public class ServerLocatorRegistry implements TestLocatorRegistry
{
    @Override
    public CompletionStage<Void> setAdapterDirectProxyAsync(String adapter, ObjectPrx object,
                                                            com.zeroc.Ice.Current current)
    {
        if(object != null)
        {
            _adapters.put(adapter, object);
        }
        else
        {
            _adapters.remove(adapter);
        }
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> setReplicatedAdapterDirectProxyAsync(String adapter, String replica,
                                                                      ObjectPrx object, com.zeroc.Ice.Current current)
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
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public CompletionStage<Void> setServerProcessProxyAsync(String id, com.zeroc.Ice.ProcessPrx proxy,
                                                            com.zeroc.Ice.Current current)
    {
        return CompletableFuture.completedFuture((Void)null);
    }

    @Override
    public void addObject(ObjectPrx object, com.zeroc.Ice.Current current)
    {
        _objects.put(object.ice_getIdentity(), object);
    }

    public ObjectPrx getAdapter(String adapter)
        throws com.zeroc.Ice.AdapterNotFoundException
    {
        ObjectPrx obj = _adapters.get(adapter);
        if(obj == null)
        {
            throw new com.zeroc.Ice.AdapterNotFoundException();
        }
        return obj;
    }

    public ObjectPrx getObject(Identity id)
        throws com.zeroc.Ice.ObjectNotFoundException
    {
        ObjectPrx obj = _objects.get(id);
        if(obj == null)
        {
            throw new com.zeroc.Ice.ObjectNotFoundException();
        }
        return obj;
    }

    private java.util.HashMap<String, ObjectPrx> _adapters = new java.util.HashMap<>();
    private java.util.HashMap<Identity, ObjectPrx> _objects = new java.util.HashMap<>();
}
