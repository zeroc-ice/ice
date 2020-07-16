//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Timeout
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);

            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            // The client sends large messages to cause the transport buffers to fill up.
            properties["Ice.MessageSizeMax"] = "20M";

            // Limit the recv buffer size, this test relies on the socket send() blocking after sending a given
            // amount of data.
            properties["Ice.TCP.RcvSize"] = "50K";

            await using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1));

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default);
            var adapter = communicator.CreateObjectAdapter("TestAdapter",
                                                           taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter.Add("timeout", new Timeout());
            adapter.Activate();

            var controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
            controllerAdapter.Add("controller", new Controller(schedulerPair.ExclusiveScheduler));
            controllerAdapter.Activate();

            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
