//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Client : TestHelper
    {
        public override void Run(string[] args)
        {
            var observer = new CommunicatorObserver();

            var properties = CreateTestProperties(ref args);
            properties["Ice.Admin.Endpoints"] = "tcp";
            properties["Ice.Admin.InstanceName"] = "client";
            properties["Ice.Admin.DelayCreation"] = "1";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Default.Host"] = "127.0.0.1";

            using var communicator = Initialize(properties, observer: observer);
            IMetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Client>(args);
    }
}
