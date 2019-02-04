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
                var initData = new Ice.InitializationData();
                initData.observer = Instrumentation.getObserver();

                initData.properties = createTestProperties(ref args);
                initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

                //
                // This test kills connections, so we don't want warnings.
                //
                initData.properties.setProperty("Ice.Warn.Connections", "0");
                using(var communicator = initialize(initData))
                {
                    //
                    // Configure a second communicator for the invocation timeout
                    // + retry test, we need to configure a large retry interval
                    // to avoid time-sensitive failures.
                    //
                    initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
                    initData.observer = Instrumentation.getObserver();
                    using(var communicator2 = initialize(initData))
                    {
                        var retry = AllTests.allTests(this, communicator, communicator2, "retry:" + getTestEndpoint(0));
                        retry.shutdown();
                    }
                }
            }

            public static int Main(string[] args)
            {
                return TestDriver.runTest<Client>(args);
            }
        }
    }
}
