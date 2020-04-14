//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Diagnostics;
using System.Collections.Generic;
using Test;

public sealed class Controller : IController
{
    public void
    activateObjectAdapter(string name, string adapterId, string replicaGroupId, Ice.Current current)
    {
        Ice.Communicator communicator = current.Adapter.Communicator;
        communicator.SetProperty($"{name}.AdapterId", adapterId);
        communicator.SetProperty($"{name}.ReplicaGroupId", replicaGroupId);
        communicator.SetProperty($"{name}.Endpoints", "default");
        Ice.ObjectAdapter oa = communicator.CreateObjectAdapter(name);
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
        TestHelper.Assert(_adapters.ContainsKey(oaName));
        _adapters[oaName].Add(id, new TestIntf());
    }

    public void
    removeObject(string oaName, string id, Ice.Current current)
    {
        TestHelper.Assert(_adapters.ContainsKey(oaName));
        _adapters[oaName].Remove(id);
    }

    public void
    shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();

    private Dictionary<string, Ice.ObjectAdapter> _adapters = new Dictionary<string, Ice.ObjectAdapter>();
}

public sealed class TestIntf : ITestIntf
{
    public string
    getAdapterId(Ice.Current current) =>
        current.Adapter.Communicator.GetProperty($"{current.Adapter.Name}.AdapterId") ?? "";
}
