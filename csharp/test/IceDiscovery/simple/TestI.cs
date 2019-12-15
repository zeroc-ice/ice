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
        Ice.Communicator communicator = current.Adapter.Communicator;
        Ice.Properties properties = communicator.Properties;
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
        _adapters[oaName].Add(new TestIntfI(), id);
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
        current.Adapter.Communicator.shutdown();
    }

    private Dictionary<string, Ice.ObjectAdapter> _adapters = new Dictionary<string, Ice.ObjectAdapter>();
}

public sealed class TestIntfI : Test.TestIntf
{
    public string
    getAdapterId(Ice.Current current)
    {
        return current.Adapter.Communicator.Properties.getProperty(current.Adapter.GetName() + ".AdapterId");
    }
}
