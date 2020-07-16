//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Timeout
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);

            // For this test, we want to disable retries.
            properties["Ice.RetryIntervals"] = "-1";

            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            // Limit the send buffer size, this test relies on the socket send() blocking after sending a given amount
            // of data.
            properties["Ice.TCP.SndSize"] = "50K";
            await using Communicator communicator = Initialize(properties);
            AllTests.allTests(this);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
