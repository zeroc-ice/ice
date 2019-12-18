//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);
        properties["Ice.ServerIdleTime"] = "30";
        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties["Ice.TCP.RcvSize"] = "50000";
        try
        {
            var dispatcher = new Dispatcher();

            using (var communicator = initialize(properties, dispatcher.dispatch))
            {
                communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
                communicator.SetProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
                communicator.SetProperty("ControllerAdapter.ThreadPool.Size", "1");

                Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
                Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

                adapter.Add(new TestI(), "test");
                adapter.Activate();
                adapter2.Add(new TestControllerI(adapter), "testController");
                adapter2.Activate();

                communicator.waitForShutdown();
            }
        }
        finally
        {
            Dispatcher.terminate();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
