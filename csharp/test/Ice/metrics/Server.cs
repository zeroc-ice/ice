
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
        properties["Ice.Admin.Endpoints"] = "tcp";
        properties["Ice.Admin.InstanceName"] = "server";
        properties["Ice.Warn.Connections"] = "0";
        properties["Ice.Warn.Dispatch"] = "0";
        properties["Ice.MessageSizeMax"] = "50000";
        properties["Ice.Default.Host"] = "127.0.0.1";

        using var communicator = initialize(properties);
        communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add("metrics", new Metrics());
        adapter.Activate();

        communicator.SetProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
        Ice.ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
        controllerAdapter.Add("controller", new Controller(adapter));
        controllerAdapter.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Server>(args);
}
