// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Diagnostics;
using System.Collections.Generic;

public sealed class ControllerI : Test.ControllerDisp_
{
    public override void
    activateObjectAdapter(string name, string adapterId, string replicaGroupId, Ice.Current current)
    {
        Ice.Communicator communicator = current.adapter.getCommunicator();
        Ice.Properties properties = communicator.getProperties();
        properties.setProperty(name + ".AdapterId", adapterId);
        properties.setProperty(name + ".ReplicaGroupId", replicaGroupId);
        properties.setProperty(name + ".Endpoints", "default");
        Ice.ObjectAdapter oa = communicator.createObjectAdapter(name);
        _adapters[name] = oa;
        oa.activate();
    }

    public override void
    deactivateObjectAdapter(string name, Ice.Current current)
    {
        _adapters[name].destroy();
        _adapters.Remove(name);
    }

    public override void
    addObject(string oaName, string id, Ice.Current current)
    {
        Debug.Assert(_adapters.ContainsKey(oaName));
        Ice.Identity identity = new Ice.Identity();
        identity.name = id;
        _adapters[oaName].add(new TestIntfI(), identity);
    }

    public override void
    removeObject(string oaName, string id, Ice.Current current)
    {
        Debug.Assert(_adapters.ContainsKey(oaName));
        Ice.Identity identity = new Ice.Identity();
        identity.name = id;
        _adapters[oaName].remove(identity);
    }

    public override void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private Dictionary<string, Ice.ObjectAdapter> _adapters = new Dictionary<string, Ice.ObjectAdapter>();
}

public sealed class TestIntfI : Test.TestIntfDisp_
{
    public override string 
    getAdapterId(Ice.Current current)
    {
        return current.adapter.getCommunicator().getProperties().getProperty(current.adapter.getName() + ".AdapterId");
    }
}
