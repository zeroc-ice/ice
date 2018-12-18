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
import test.Ice.acm.Test.RemoteObjectAdapterPrxHelper;
import test.Ice.acm.Test._RemoteCommunicatorDisp;

public class RemoteCommunicatorI extends _RemoteCommunicatorDisp
{
    public RemoteObjectAdapterPrx
    createObjectAdapter(int timeout, int close, int heartbeat, Ice.Current current)
    {
        Ice.Communicator com = current.adapter.getCommunicator();
        Ice.Properties properties = com.getProperties();
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
        Ice.ObjectAdapter adapter = com.createObjectAdapterWithEndpoints(name, protocol + " -h \"" + host + "\"");
        return RemoteObjectAdapterPrxHelper.uncheckedCast(
            current.adapter.addWithUUID(new RemoteObjectAdapterI(adapter)));
    }

    public void
    shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }
};
