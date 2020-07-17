//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.RetryIntervals"] = "0 1 10 1";

            //
            // This test kills connections, so we don't want warnings.
            //
            properties["Ice.Warn.Connections"] = "0";
            await using var communicator = Initialize(properties, observer: Instrumentation.GetObserver());
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            properties["Ice.RetryIntervals"] = "0 1 10000";
            await using var communicator2 = Initialize(properties, observer: Instrumentation.GetObserver());
            AllTests.allTests(this, communicator, communicator2, $"retry:{GetTestEndpoint(0)}").Shutdown();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Client>(args);
    }
}
