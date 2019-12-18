//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.retry.Test;

namespace Ice
{
    namespace retry
    {
        public class Collocated : TestHelper
        {
            public override void run(string[] args)
            {

                var observer = Instrumentation.getObserver();
                var properties = createTestProperties(ref args);
                properties["Ice.RetryIntervals"] = "0 1 10 1";
                properties["Ice.Warn.Dispatch"] = "0";
                //
                // This test kills connections, so we don't want warnings.
                //
                properties["Ice.Warn.Connections"] = "0";
                using (var communicator = initialize(properties, observer: observer))
                {
                    //
                    // Configure a second communicator for the invocation timeout
                    // + retry test, we need to configure a large retry interval
                    // to avoid time-sensitive failures.
                    //
                    properties["Ice.RetryIntervals"] = "0 1 10000";
                    using (var communicator2 = initialize(properties, observer: observer))
                    {
                        communicator.createObjectAdapter("").Add(new RetryI(), "retry");
                        communicator2.createObjectAdapter("").Add(new RetryI(), "retry");
                        AllTests.allTests(this, communicator, communicator2, "retry").shutdown();
                    }
                }
            }
            public static int Main(string[] args)
            {
                return TestDriver.runTest<Collocated>(args);
            }

        }
    }
}
