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

public class Collocated : Test.TestHelper
{
    public override void run(string[] args)
    {
        try
        {
            Ice.InitializationData initData = new Ice.InitializationData();
            initData.properties = createTestProperties(ref args);
            initData.properties.setProperty("Ice.Warn.AMICallback", "0");
            initData.dispatcher = new Dispatcher().dispatch;

            using(var communicator = initialize(initData))
            {
                communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
                //adapter.activate(); // Don't activate OA to ensure collocation is used.
                adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
                //adapter2.activate(); // Don't activate OA to ensure collocation is used.

                AllTests.allTests(this);
            }
        }
        finally
        {
            Dispatcher.terminate();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Collocated>(args);
    }
}
