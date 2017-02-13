// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************
package test.Ice.binding;
import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.Ice.binding.Test.RemoteObjectAdapterPrxHelper;
import test.Ice.binding.Test._RemoteCommunicatorDisp;

public class RemoteCommunicatorI extends _RemoteCommunicatorDisp
{
    @Override
    public RemoteObjectAdapterPrx
    createObjectAdapter(String name, String endpts, Ice.Current current)
    {
        String endpoints = endpts;
        if(endpoints.indexOf("-p") < 0)
        {
            // Use a fixed port if none is specified (bug 2896)
            endpoints += " -h \"" + 
                (current.adapter.getCommunicator().getProperties().getPropertyWithDefault(
                                                                                    "Ice.Default.Host", "127.0.0.1")) +
                "\" -p " + _nextPort++;
        }

        Ice.Communicator com = current.adapter.getCommunicator();
        com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
        Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, endpoints);
        return RemoteObjectAdapterPrxHelper.uncheckedCast(
            current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
    }

    @Override
    public void
    deactivateObjectAdapter(RemoteObjectAdapterPrx adapter, Ice.Current current)
    {
        adapter.deactivate(); // Collocated call.
    }

    @Override
    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private int _nextPort = 10001;
}
