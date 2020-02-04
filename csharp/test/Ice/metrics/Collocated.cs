//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Ice;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated : TestHelper
{
    public override void run(string[] args)
    {
        var observer = new CommunicatorObserver();

        var properties = createTestProperties(ref args);
        properties["Ice.Admin.Endpoints"] = "tcp";
        properties["Ice.Admin.InstanceName"] = "client";
        properties["Ice.Admin.DelayCreation"] = "1";
        properties["Ice.Warn.Connections"] = "0";
        properties["Ice.Warn.Dispatch"] = "0";
        properties["Ice.Default.Host"] = "127.0.0.1";

        using (var communicator = initialize(properties, observer: observer))
        {
            communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            communicator.SetProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            ObjectAdapter controllerAdapter = communicator.CreateObjectAdapter("ControllerAdapter");
            controllerAdapter.Add("controller", new Controller(adapter));
            //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

            IMetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
    }

    public static int Main(string[] args)
    {
        return TestDriver.runTest<Collocated>(args);
    }
}
