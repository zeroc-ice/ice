//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Ice;
using Test;

public class Collocated : TestHelper
{
    public override void Run(string[] args)
    {
        var observer = new CommunicatorObserver();

        var properties = CreateTestProperties(ref args);
        properties["Ice.Admin.Endpoints"] = "tcp";
        properties["Ice.Admin.InstanceName"] = "client";
        properties["Ice.Admin.DelayCreation"] = "1";
        properties["Ice.Warn.Connections"] = "0";
        properties["Ice.Warn.Dispatch"] = "0";
        properties["Ice.Default.Host"] = "127.0.0.1";

        using (var communicator = Initialize(properties, observer: observer))
        {
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            IMetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
    }

    public static int Main(string[] args)
    {
        return TestDriver.RunTest<Collocated>(args);
    }
}
