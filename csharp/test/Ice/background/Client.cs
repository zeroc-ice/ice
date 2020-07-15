//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Background
{
    public class Client : TestHelper
    {
        public override Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);

            //
            // For this test, we want to disable retries.
            //
            properties["Ice.RetryIntervals"] = "-1";

            //
            // This test kills connections, so we don't want warnings.
            //
            properties["Ice.Warn.Connections"] = "0";

            // This test relies on filling the TCP send/recv buffer, so
            // we rely on a fixed value for these buffers.
            properties["Ice.TCP.SndSize"] = "50000";

            //
            // Setup the test transport plug-in.
            //
            if (!properties.TryGetValue("Ice.Default.Transport", out string? transport))
            {
                transport = "tcp";
            }
            properties["Ice.Default.Transport"] = $"test-{transport}";

            using Communicator communicator = Initialize(properties);
            var plugin = new Plugin(communicator);
            plugin.Initialize();
            communicator.AddPlugin("Test", plugin);
            IBackgroundPrx background = AllTests.allTests(this);
            background.shutdown();
            return Task.CompletedTask;
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
