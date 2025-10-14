// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Admin.Endpoints", "tcp");
        properties.setProperty("Ice.Admin.InstanceName", "server");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.MessageSizeMax", "50000");
        properties.setProperty("Ice.Default.Host", "127.0.0.1");

        using var communicator = initialize(properties);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new MetricsI(), Ice.Util.stringToIdentity("metrics"));
        adapter.activate();

        communicator.getProperties().setProperty("ForwardingAdapter.Endpoints", getTestEndpoint(1));
        Ice.ObjectAdapter forwardingAdapter = communicator.createObjectAdapter("ForwardingAdapter");
        forwardingAdapter.addDefaultServant(adapter.dispatchPipeline, "");
        forwardingAdapter.activate();

        communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(2));
        Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
        controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
        controllerAdapter.activate();

        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
