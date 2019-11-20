//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Reflection;
using Ice;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated : Test.TestHelper
{
    public override void run(string[] args)
    {
        CommunicatorObserverI observer = new CommunicatorObserverI();
        Ice.InitializationData initData = new Ice.InitializationData();
        initData.observer = observer;
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");

        using (var communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new MetricsI(), "metrics");
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.Add(new ControllerI(adapter), "controller");
            //controllerAdapter.activate(); // Don't activate OA to ensure collocation is used.

            Test.MetricsPrx metrics = AllTests.allTests(this, observer);
            metrics.shutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Collocated>(args);
    }
}
