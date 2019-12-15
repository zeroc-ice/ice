//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

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

            using (var communicator = initialize(initData))
            {
                communicator.Properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.Properties.setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                communicator.Properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");

                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                adapter.Add(new TestI(), "test");
                //adapter.activate(); // Don't activate OA to ensure collocation is used.
                adapter2.Add(new TestControllerI(adapter), "testController");
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
