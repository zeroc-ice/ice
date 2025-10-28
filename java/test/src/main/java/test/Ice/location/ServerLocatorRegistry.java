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

public class ServerLocatorRegistry implements TestLocatorRegistry {
    @Override
    public void setAdapterDirectProxy(String adapter, ObjectPrx object, Current current) {
        ++_setRequestCount;
        if (object != null) {
            _adapters.put(adapter, object);
        } else {
            _adapters.remove(adapter);
        }
    }

    @Override
    public void setReplicatedAdapterDirectProxy(String adapter, String replica, ObjectPrx object, Current current) {
        ++_setRequestCount;
        if (object != null) {
            _adapters.put(adapter, object);
            _adapters.put(replica, object);
        } else {
            _adapters.remove(adapter);
            _adapters.remove(replica);
        }
    }

    @Override
    public void setServerProcessProxy(String id, ProcessPrx proxy, Current current) {
        ++_setRequestCount;
    }

    @Override
    public void addObject(ObjectPrx object, Current current) {
        _objects.put(object.ice_getIdentity(), object);
    }

    @Override
    public int getSetRequestCount(Current current) {
        return _setRequestCount;
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
    private int _setRequestCount;
}
