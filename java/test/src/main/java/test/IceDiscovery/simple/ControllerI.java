// Copyright (c) ZeroC, Inc.

package test.IceDiscovery.simple;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.Identity;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.Properties;

import test.IceDiscovery.simple.Test.Controller;

import java.util.HashMap;
import java.util.Map;

public final class ControllerI implements Controller {
    @Override
    public void activateObjectAdapter(
            String name, String adapterId, String replicaGroupId, Current current) {
        Communicator communicator = current.adapter.getCommunicator();
        Properties properties = communicator.getProperties();
        properties.setProperty(name + ".AdapterId", adapterId);
        properties.setProperty(name + ".ReplicaGroupId", replicaGroupId);
        properties.setProperty(name + ".Endpoints", "default");
        ObjectAdapter oa = communicator.createObjectAdapter(name);
        _adapters.put(name, oa);
        oa.activate();
    }

    @Override
    public void deactivateObjectAdapter(String name, Current current) {
        _adapters.get(name).destroy();
        _adapters.remove(name);
    }

    @Override
    public void addObject(String oaName, String id, Current current) {
        assert (_adapters.containsKey(oaName));
        Identity identity = new Identity();
        identity.name = id;
        _adapters.get(oaName).add(new TestIntfI(), identity);
    }

    @Override
    public void removeObject(String oaName, String id, Current current) {
        assert (_adapters.containsKey(oaName));
        Identity identity = new Identity();
        identity.name = id;
        _adapters.get(oaName).remove(identity);
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    private final Map<String, ObjectAdapter> _adapters =
        new HashMap<String, ObjectAdapter>();
}
