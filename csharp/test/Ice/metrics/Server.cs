//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

public class Server : TestHelper
{
    public override void Run(string[] args)
    {
        Dictionary<string, string> properties = CreateTestProperties(ref args);
        properties["Ice.Admin.Endpoints"] = "tcp";
        properties["Ice.Admin.InstanceName"] = "server";
        properties["Ice.Warn.Connections"] = "0";
        properties["Ice.Warn.Dispatch"] = "0";
        properties["Ice.MessageSizeMax"] = "50000";
        properties["Ice.Default.Host"] = "127.0.0.1";

        using Ice.Communicator communicator = Initialize(properties);
        communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

        communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1));
        Ice.ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
        controllerAdapter.Add("controller", new Controller(() => {
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            return adapter;
        }));
        controllerAdapter.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
