// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice.Instrumentation;

namespace ZeroC.Ice.Test.Retry
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            ICommunicatorObserver? observer = Instrumentation.GetObserver();
            Dictionary<string, string>? properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";
            await using Communicator communicator = Initialize(properties, observer: observer);
            await communicator.ActivateAsync();

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.CreateObjectAdapter("TestAdapter").Add("retry", new Retry());
            await AllTests.Run(this, communicator, true).ShutdownAsync();
        }
        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Collocated>(args);
    }
}
