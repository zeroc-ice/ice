// Copyright (c) ZeroC, Inc.

package test.Ice.location;

import com.zeroc.Ice.AdapterNotFoundException;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectNotFoundException;
import com.zeroc.Ice.ObjectPrx;
import com.zeroc.Ice.ProcessPrx;

import test.Ice.location.Test.TestLocatorRegistry;

import java.util.HashMap;
import java.util.concurrent.CompletableFuture;
import java.util.concurrent.CompletionStage;

public class ServerLocatorRegistry implements TestLocatorRegistry {
    @Override
    public CompletionStage<Void> setAdapterDirectProxyAsync(
            String adapter, ObjectPrx object, Current current) {
        if (object != null) {
            _adapters.put(adapter, object);
        } else {
            _adapters.remove(adapter);
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> setReplicatedAdapterDirectProxyAsync(
            String adapter, String replica, ObjectPrx object, Current current) {
        if (object != null) {
            _adapters.put(adapter, object);
            _adapters.put(replica, object);
        } else {
            _adapters.remove(adapter);
            _adapters.remove(replica);
        }
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public CompletionStage<Void> setServerProcessProxyAsync(
            String id, ProcessPrx proxy, Current current) {
        return CompletableFuture.completedFuture((Void) null);
    }

    @Override
    public void addObject(ObjectPrx object, Current current) {
        _objects.put(object.ice_getIdentity(), object);
    }

    public ObjectPrx getAdapter(String adapter) throws AdapterNotFoundException {
        ObjectPrx obj = _adapters.get(adapter);
        if (obj == null) {
            throw new AdapterNotFoundException();
        }
        return obj;
    }

    public ObjectPrx getObject(Identity id) throws ObjectNotFoundException {
        ObjectPrx obj = _objects.get(id);
        if (obj == null) {
            throw new ObjectNotFoundException();
        }
        return obj;
    }

    private final HashMap<String, ObjectPrx> _adapters = new HashMap<>();
    private final HashMap<Identity, ObjectPrx> _objects = new HashMap<>();
}
