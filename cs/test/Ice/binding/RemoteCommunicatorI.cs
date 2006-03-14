// **********************************************************************
//
// Copyright (c) 2003-2006 ZeroC, Inc. All rights reserved.
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
	com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
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
