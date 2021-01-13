// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            return AllTests.RunAsync(this);
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.IncomingFrameMaxSize"] = "10K";

            await using var communicator = CreateCommunicator(properties);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
