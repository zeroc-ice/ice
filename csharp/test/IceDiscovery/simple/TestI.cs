//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Collections.Generic;
using Test;

public sealed class ControllerI : Test.Controller
{
    public void
    activateObjectAdapter(string name, string adapterId, string replicaGroupId, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.GetCommunicator();
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty(name + ".AdapterId", adapterId);
        properties.setProperty(name + ".ReplicaGroupId", replicaGroupId);
        properties.setProperty(name + ".Endpoints", "default");
        Ice.ObjectAdapter oa = communicator.createObjectAdapter(name);
        _adapters[name] = oa;
        oa.Activate();
    }

    public void
    deactivateObjectAdapter(string name, Ice.Current current)
    {
        _adapters[name].Destroy();
        _adapters.Remove(name);
    }

    public void
    addObject(string oaName, string id, Ice.Current current)
    {
        Debug.Assert(_adapters.ContainsKey(oaName));
        Ice.Identity identity = new Ice.Identity();
        identity.name = id;
        _adapters[oaName].Add(new TestIntfI(), identity);
    }

    public void
    removeObject(string oaName, string id, Ice.Current current)
    {
        Debug.Assert(_adapters.ContainsKey(oaName));
        _adapters[oaName].Remove(id);
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.GetCommunicator().shutdown();
    }

    private Dictionary<string, Ice.ObjectAdapter> _adapters = new Dictionary<string, Ice.ObjectAdapter>();
}

public sealed class TestIntfI : Test.TestIntf
{
    public string
    getAdapterId(Ice.Current current)
    {
        return current.adapter.GetCommunicator().getProperties().getProperty(current.adapter.GetName() + ".AdapterId");
    }
}
