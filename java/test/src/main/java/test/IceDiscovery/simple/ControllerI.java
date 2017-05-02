// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

public final class ControllerI implements Controller
{
    @Override
    public void activateObjectAdapter(String name, String adapterId, String replicaGroupId,
                                      com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.Communicator communicator = current.adapter.getCommunicator();
        com.zeroc.Ice.Properties properties = communicator.getProperties();
        properties.setProperty(name + ".AdapterId", adapterId);
        properties.setProperty(name + ".ReplicaGroupId", replicaGroupId);
        properties.setProperty(name + ".Endpoints", "default");
        com.zeroc.Ice.ObjectAdapter oa = communicator.createObjectAdapter(name);
        _adapters.put(name, oa);
        oa.activate();
    }

    @Override
    public void deactivateObjectAdapter(String name, com.zeroc.Ice.Current current)
    {
        _adapters.get(name).destroy();
        _adapters.remove(name);
    }

    @Override
    public void addObject(String oaName, String id, com.zeroc.Ice.Current current)
    {
        assert(_adapters.containsKey(oaName));
        com.zeroc.Ice.Identity identity = new com.zeroc.Ice.Identity();
        identity.name = id;
        _adapters.get(oaName).add(new TestIntfI(), identity);
    }

    @Override
    public void removeObject(String oaName, String id, com.zeroc.Ice.Current current)
    {
        assert(_adapters.containsKey(oaName));
        com.zeroc.Ice.Identity identity = new com.zeroc.Ice.Identity();
        identity.name = id;
        _adapters.get(oaName).remove(identity);
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    final private java.util.Map<String, com.zeroc.Ice.ObjectAdapter> _adapters =
        new java.util.HashMap<String, com.zeroc.Ice.ObjectAdapter>();
}
