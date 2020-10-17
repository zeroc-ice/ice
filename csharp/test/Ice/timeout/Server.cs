// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using System.Collections.Generic;
using Test;
using System;

namespace ZeroC.Ice.Test.Timeout
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string>? properties = CreateTestProperties(ref args);

            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            // The client sends large messages to cause the transport buffers to fill up.
            properties["Ice.IncomingFrameSizeMax"] = "20M";

            // Limit the recv buffer size, this test relies on the socket send() blocking after sending a given
            // amount of data.
            properties["Ice.TCP.RcvSize"] = "50K";

            await using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("ControllerAdapter.Endpoints", GetTestEndpoint(1));

            var schedulerPair = new ConcurrentExclusiveSchedulerPair(TaskScheduler.Default);
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter",
                                                                      taskScheduler: schedulerPair.ExclusiveScheduler);
            adapter.Add("timeout", new Timeout());
            adapter.Activate((request, current, next, cancel) =>
                {
                    if (current.Operation == "checkDeadline")
                    {
                        if (request.BinaryContext.TryGetValue(10, out ReadOnlyMemory<byte> value))
                        {
                            current.Context["deadline"] = value.Read(istr => istr.ReadVarLong()).ToString();
                        }
                    }
                    return next(request, current, cancel);
                });

            ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
            controllerAdapter.Add("controller", new Controller(schedulerPair.ExclusiveScheduler));
            controllerAdapter.Activate();

            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
