// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.binding;

import test.Ice.binding.Test.RemoteObjectAdapterPrx;
import test.Ice.binding.Test.RemoteCommunicator;

public class RemoteCommunicatorI implements RemoteCommunicator
{
    public
    RemoteCommunicatorI(test.Util.Application app)
    {
        _app = app;
    }

    @Override
    public RemoteObjectAdapterPrx createObjectAdapter(String name, String endpts, com.zeroc.Ice.Current current)
    {
        int retry = 5;
        while(true)
        {
            try
            {
                String endpoints = endpts;
                if(endpoints.indexOf("-p") < 0)
                {
                    endpoints = _app.getTestEndpoint(_nextPort++, endpoints);
                }

                com.zeroc.Ice.Communicator com = current.adapter.getCommunicator();
                com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                com.zeroc.Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, endpoints);
                return RemoteObjectAdapterPrx.uncheckedCast(current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
            }
            catch(com.zeroc.Ice.SocketException ex)
            {
                if(--retry == 0)
                {
                    throw ex;
                }
            }
        }
    }

    @Override
    public void deactivateObjectAdapter(RemoteObjectAdapterPrx adapter, com.zeroc.Ice.Current current)
    {
        adapter.deactivate(); // Collocated call.
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    private final test.Util.Application _app;
    private int _nextPort = 10;
}
