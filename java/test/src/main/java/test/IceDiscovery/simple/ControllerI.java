// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.IceDiscovery.simple;

import test.IceDiscovery.simple.Test.*;

public final class ControllerI extends _ControllerDisp
{
    @Override
    public void
    activateObjectAdapter(String name, String adapterId, String replicaGroupId, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty(name + ".AdapterId", adapterId);
        properties.setProperty(name + ".ReplicaGroupId", replicaGroupId);
        properties.setProperty(name + ".Endpoints", "default");
        Ice.ObjectAdapter oa = communicator.createObjectAdapter(name);
        _adapters.put(name, oa);
        oa.activate();
    }

    @Override
    public void
    deactivateObjectAdapter(String name, Ice.Current current)
    {
        _adapters.get(name).destroy();
        _adapters.remove(name);
    }

    @Override
    public void
    addObject(String oaName, String id, Ice.Current current)
    {
        assert(_adapters.containsKey(oaName));
        Ice.Identity identity = new Ice.Identity();
        identity.name = id;
        _adapters.get(oaName).add(new TestIntfI(), identity);
    }

    @Override
    public void
    removeObject(String oaName, String id, Ice.Current current)
    {
        assert(_adapters.containsKey(oaName));
        Ice.Identity identity = new Ice.Identity();
        identity.name = id;
        _adapters.get(oaName).remove(identity);
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    final private java.util.Map<String, Ice.ObjectAdapter> _adapters = 
        new java.util.HashMap<String, Ice.ObjectAdapter>();
}
