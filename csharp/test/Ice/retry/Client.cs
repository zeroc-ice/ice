// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args) => AllTests.RunAsync(this, false);

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            await using var communicator = CreateCommunicator(properties, observer: Instrumentation.GetObserver());
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
