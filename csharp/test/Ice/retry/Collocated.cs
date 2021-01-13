// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Ice.Instrumentation;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            Communicator.CreateObjectAdapter("TestAdapter").Add("retry", new Retry());

            await AllTests.RunAsync(this, true);
        }

        public static async Task<int> Main(string[] args)
        {
            ICommunicatorObserver? observer = Instrumentation.GetObserver();
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            await using var communicator = CreateCommunicator(properties, observer: observer);
            return await RunTestAsync<Collocated>(communicator, args);
        }
    }
}
