// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

public class Server extends test.Util.Application
{
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("communicator");
        adapter.add(new RemoteCommunicatorI(), id);
        adapter.activate();

        // Disable ready print for further adapters.
        communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");

        return WAIT;
    }

    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.acm");
        r.initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        r.initData.properties.setProperty("Ice.Warn.Connections", "0");
        r.initData.properties.setProperty("Ice.ACM.Timeout", "1");
        return r;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
