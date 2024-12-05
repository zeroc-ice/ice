// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Collocated : Test.TestHelper
{
    public override async Task runAsync(string[] args)
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

        using var communicator = initialize(initData);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), Ice.Util.stringToIdentity("metrics"));
        // Don't activate OA to ensure collocation is used.

        communicator.getProperties().setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
        Ice.ObjectAdapter forwardingAdapter = communicator.createObjectAdapter("ForwardingAdapter");
        forwardingAdapter.addDefaultServant(adapter.dispatchPipeline, "");

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
        Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
        // Don't activate OA to ensure collocation is used.

        Test.MetricsPrx metrics = await AllTests.allTests(this, observer);
        metrics.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Collocated>(args);
}
