// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hold;

public class Server extends test.Util.Application
{
    @Override
    public int run(String[] args)
    {
        com.zeroc.Ice.Communicator communicator = communicator();
        java.util.Timer timer = new java.util.Timer();

        com.zeroc.Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
        adapter1.add(new HoldI(timer, adapter1), com.zeroc.Ice.Util.stringToIdentity("hold"));

        com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
        adapter2.add(new HoldI(timer, adapter2), com.zeroc.Ice.Util.stringToIdentity("hold"));

        adapter1.activate();
        adapter2.activate();

        serverReady();
        communicator.waitForShutdown();

        timer.cancel();

        return 0;
    }

    @Override
    protected com.zeroc.Ice.InitializationData getInitData(String[] args, java.util.List<String> rArgs)
    {
        com.zeroc.Ice.InitializationData initData = super.getInitData(args, rArgs);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.hold");
        initData.properties.setProperty("TestAdapter1.Endpoints", getTestEndpoint(initData.properties, 0));
        initData.properties.setProperty("TestAdapter1.ThreadPool.Size", "5");
        initData.properties.setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
        initData.properties.setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
        initData.properties.setProperty("TestAdapter1.ThreadPool.Serialize", "0");

        initData.properties.setProperty("TestAdapter2.Endpoints", getTestEndpoint(initData.properties, 1));
        initData.properties.setProperty("TestAdapter2.ThreadPool.Size", "5");
        initData.properties.setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
        initData.properties.setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
        initData.properties.setProperty("TestAdapter2.ThreadPool.Serialize", "1");
        return initData;
    }

    public static void main(String[] args)
    {
        Server app = new Server();
        int result = app.main("Server", args);
        System.gc();
        System.exit(result);
    }
}
