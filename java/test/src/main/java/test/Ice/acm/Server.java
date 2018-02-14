// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.acm;

public class Server extends test.Util.Application
{
    public int
    run(String[] args)
    {
        Ice.Communicator communicator = communicator();
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Identity id = communicator.stringToIdentity("communicator");
        adapter.add(new RemoteCommunicatorI(), id);
        adapter.activate();

        // Disable ready print for further adapters.
        communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");

        return WAIT;
    }

    protected Ice.InitializationData getInitData(Ice.StringSeqHolder argsH)
    {
        Ice.InitializationData initData = createInitializationData() ;
        initData.properties = Ice.Util.createProperties(argsH);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.acm");
        initData.properties.setProperty("TestAdapter.Endpoints", "default -p 12010");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.ACM.Timeout", "1");
        return initData;
    }

    public static void
    main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
