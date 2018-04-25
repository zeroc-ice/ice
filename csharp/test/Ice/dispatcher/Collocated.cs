// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
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
        //adapter.activate(); // Don't activate OA to ensure collocation is used.
        adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
        //adapter2.activate(); // Don't activate OA to ensure collocation is used.

        AllTests.allTests(this);
        return 0;
    }

    protected override Ice.InitializationData getInitData(ref string[] args)
    {
        Ice.InitializationData initData = base.getInitData(ref args);
        initData.properties.setProperty("Ice.Warn.AMICallback", "0");
        initData.dispatcher = new Dispatcher().dispatch;
        return initData;
    }

    public static int Main(string[] args)
    {
        Collocated app = new Collocated();
        try
        {
            return app.runmain(args);
        }
        finally
        {
            Dispatcher.terminate();
        }
    }
}
