//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
using Ice.retry.Test;

namespace Ice.retry
{
    public class Collocated : TestHelper
    {
        public override void Run(string[] args)
        {
            var observer = Instrumentation.GetObserver();
            var properties = CreateTestProperties(ref args);
            properties["Ice.RetryIntervals"] = "0 1 10 1";
            properties["Ice.Warn.Dispatch"] = "0";
            //
            // This test kills connections, so we don't want warnings.
            //
            properties["Ice.Warn.Connections"] = "0";
            using var communicator = Initialize(properties, observer: observer);
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            properties["Ice.RetryIntervals"] = "0 1 10000";
            using var communicator2 = Initialize(properties, observer: observer);
            communicator.CreateObjectAdapter().Add("retry", new Retry());
            communicator2.CreateObjectAdapter().Add("retry", new Retry());
            AllTests.allTests(this, communicator, communicator2, "retry").shutdown();
        }
        public static int Main(string[] args) => TestDriver.RunTest<Collocated>(args);
    }
}
