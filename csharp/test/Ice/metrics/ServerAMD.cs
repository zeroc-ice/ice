//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class ServerAMD : TestHelper
    {
        public override async Task Run(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Admin.Endpoints"] = "tcp";
            properties["Ice.Admin.InstanceName"] = "server";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.MessageSizeMax"] = "50000";
            properties["Ice.Default.Host"] = "127.0.0.1";

            using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1));
            ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
            controllerAdapter.Add("controller", new Controller(() => {
                ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
                adapter.Add("metrics", new MetricsAsync());
                return adapter;
            }));
            await controllerAdapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTest<ServerAMD>(args);
    }
}
