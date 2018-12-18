// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
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
        int retry = 5;
        while(true)
        {
            try
            {
                String endpoints = endpts;
                if(endpoints.indexOf("-p") < 0)
                {
                    endpoints = test.TestHelper.getTestEndpoint(current.adapter.getCommunicator().getProperties(),
                                                                _nextPort++,
                                                                endpoints);
                }

                Ice.Communicator com = current.adapter.getCommunicator();
                com.getProperties().setProperty(name + ".ThreadPool.Size", "1");
                Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, endpoints);
                Ice.Object obj = new RemoteObjectAdapterI(adapter);
                return RemoteObjectAdapterPrxHelper.uncheckedCast(current.adapter.addWithUUID(obj));
            }
            catch(Ice.SocketException ex)
            {
                if(--retry == 0)
                {
                    throw ex;
                }
            }
        }
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

    private int _nextPort = 10;
}
