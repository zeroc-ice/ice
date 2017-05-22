// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        communicator().getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        communicator().getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
        adapter.add(new TestII(), Ice.Util.stringToIdentity("test2"));
        adapter.activate();
        adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
        adapter2.activate();

        communicator().waitForShutdown();
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.ServerIdleTime", "30");

        //
        // Disable collocation optimization to test async/await dispatch.
        //
        initData.properties.setProperty("Ice.Default.CollocationOptimized", "0");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.RcvSize", "50000");
        return initData;
    }

    public static int Main(string[] args)
    {
        Server app = new Server();
        return app.runmain(args);
    }
}
