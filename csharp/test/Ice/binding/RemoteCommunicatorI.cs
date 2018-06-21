// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using Test;

public class RemoteCommunicatorI : RemoteCommunicatorDisp_
{
    public override RemoteObjectAdapterPrx
    createObjectAdapter(string name, string endpts, Ice.Current current)
    {
        int retry = 5;
        while(true)
        {
            try
            {
                Ice.Communicator communicator = current.adapter.getCommunicator();
                string endpoints = endpts;
                if(endpoints.IndexOf("-p") < 0)
                {
                    endpoints = TestHelper.getTestEndpoint(communicator.getProperties(), _nextPort++, endpoints);
                }

                communicator.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                Ice.ObjectAdapter adapter = communicator.createObjectAdapterWithEndpoints(name, endpoints);
                return RemoteObjectAdapterPrxHelper.uncheckedCast(
                    current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
            }
            catch(Ice.SocketException)
            {
                if(--retry == 0)
                {
                    throw;
                }
            }
        }
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

    private int _nextPort = 10;
};
