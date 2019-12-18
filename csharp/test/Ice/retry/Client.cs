//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace Ice
{
    namespace retry
    {
        public class Client : TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties["Ice.RetryIntervals"] = "0 1 10 1";

                //
                // This test kills connections, so we don't want warnings.
                //
                properties["Ice.Warn.Connections"] = "0";
                using var communicator = initialize(properties, observer: Instrumentation.getObserver());
                //
                // Configure a second communicator for the invocation timeout
                // + retry test, we need to configure a large retry interval
                // to avoid time-sensitive failures.
                //
                properties["Ice.RetryIntervals"] = "0 1 10000";
                using var communicator2 = initialize(properties, observer: Instrumentation.getObserver());
                AllTests.allTests(this, communicator, communicator2, $"retry:{getTestEndpoint(0)}").shutdown();
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
