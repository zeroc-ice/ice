// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

import test.Ice.acm.Test.RemoteObjectAdapterPrx;
import test.Ice.acm.Test.RemoteCommunicator;

public class RemoteCommunicatorI implements RemoteCommunicator
{
    public RemoteObjectAdapterPrx createObjectAdapter(int timeout, int close, int heartbeat,
                                                      com.zeroc.Ice.Current current)
    {
        com.zeroc.Ice.Communicator com = current.adapter.getCommunicator();
        com.zeroc.Ice.Properties properties = com.getProperties();
        String protocol = properties.getPropertyWithDefault("Ice.Default.Protocol", "tcp");
        String host = properties.getPropertyWithDefault("Ice.Default.Host", "127.0.0.1");

        String name = java.util.UUID.randomUUID().toString();
        if(timeout >= 0)
        {
            properties.setProperty(name + ".ACM.Timeout", Integer.toString(timeout));
        }
        if(close >= 0)
        {
            properties.setProperty(name + ".ACM.Close", Integer.toString(close));
        }
        if(heartbeat >= 0)
        {
            properties.setProperty(name + ".ACM.Heartbeat", Integer.toString(heartbeat));
        }
        properties.setProperty(name + ".ThreadPool.Size", "2");
        com.zeroc.Ice.ObjectAdapter adapter =
            com.createObjectAdapterWithEndpoints(name, protocol + " -h \"" + host + "\"");
        return RemoteObjectAdapterPrx.uncheckedCast(current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
    }

    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
}
