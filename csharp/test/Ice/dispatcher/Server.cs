//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
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

            using var communicator = initialize(properties, dispatcher.dispatch);
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.SetProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.SetProperty("ControllerAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            Ice.ObjectAdapter adapter2 = communicator.CreateObjectAdapter("ControllerAdapter");

            adapter.Add("test", new TestIntf());
            adapter.Activate();
            adapter2.Add("testController", new TestController(adapter));
            adapter2.Activate();

            communicator.WaitForShutdown();
        }
        finally
        {
            Dispatcher.terminate();
        }
    }

    public static int Main(string[] args) => TestDriver.runTest<Server>(args);
}
