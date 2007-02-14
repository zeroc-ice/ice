// **********************************************************************
//
// Copyright (c) 2003-2007 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class RemoteCommunicatorI : RemoteCommunicatorDisp_
{
    public override RemoteObjectAdapterPrx
    createObjectAdapter(string name, string endpoints, Ice.Current current)
    {
        Ice.Communicator com = current.adapter.getCommunicator();
        if(com.getProperties().getPropertyAsIntWithDefault("Ice.ThreadPerConnection", 0) == 0)
        {
            com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
        }
        Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, endpoints);
        return RemoteObjectAdapterPrxHelper.uncheckedCast(
            current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
    }

    public override void
    deactivateObjectAdapter(RemoteObjectAdapterPrx adapter, Ice.Current current)
    {
        adapter.deactivate(); // Collocated call.
    }

    public override void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
};
