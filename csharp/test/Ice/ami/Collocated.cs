// **********************************************************************
//
// Copyright (c) 2003-2016 ZeroC, Inc. All rights reserved.
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

public class Collocated : TestCommon.Application
{
    public override int run(string[] args)
    {
        communicator().getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        communicator().getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        communicator().getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
        Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");

        adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
        //adapter.activate(); // Collocated test doesn't need to activate the OA
        adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
        //adapter2.activate(); // Collocated test doesn't need to activate the OA

        AllTests.allTests(this, true);
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.AMICallback", "0");
        //
        // Limit the send buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        initData.properties.setProperty("Ice.TCP.SndSize", "50000");
        return initData;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        return app.runmain(args);
    }
}
