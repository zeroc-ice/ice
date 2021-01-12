// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            bool ice1 = GetTestProtocol(properties) == Protocol.Ice1;

            properties["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0";
            properties["Ice.Admin.InstanceName"] = "server";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.IncomingFrameMaxSize"] = "50M";

            await using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            await adapter.ActivateAsync();

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default);
            var adapter2 = communicator.CreateObjectAdapterWithEndpoints("TestAdapterExclusiveTS", GetTestEndpoint(2),
                taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter2.Add("metrics", new Metrics());
            await adapter2.ActivateAsync();

            communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1));
            ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");

            controllerAdapter.Add("controller", new Controller(schedulerPair.ExclusiveScheduler));
            await controllerAdapter.ActivateAsync();
            await communicator.ShutdownComplete;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
