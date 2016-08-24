// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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
    protected GetInitDataResult getInitData(String[] args)
    {
        GetInitDataResult r = super.getInitData(args);
        r.initData.properties.setProperty("Ice.Package.Test", "test.Ice.hold");
        r.initData.properties.setProperty("TestAdapter1.Endpoints", "default -p 12010:udp");
        r.initData.properties.setProperty("TestAdapter1.ThreadPool.Size", "5");
        r.initData.properties.setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
        r.initData.properties.setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
        r.initData.properties.setProperty("TestAdapter1.ThreadPool.Serialize", "0");

        r.initData.properties.setProperty("TestAdapter2.Endpoints", "default -p 12011:udp");
        r.initData.properties.setProperty("TestAdapter2.ThreadPool.Size", "5");
        r.initData.properties.setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
        r.initData.properties.setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
        r.initData.properties.setProperty("TestAdapter2.ThreadPool.Serialize", "1");
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
