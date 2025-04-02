// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var observer = new CommunicatorObserverI();
        var initData = new Ice.InitializationData();
        initData.observer = observer;
        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.Admin.Endpoints", "tcp");
        initData.properties.setProperty("Ice.Admin.InstanceName", "client");
        initData.properties.setProperty("Ice.Admin.DelayCreation", "1");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Default.Host", "127.0.0.1");

        // speed up connection establishment test
        initData.properties.setProperty("Ice.Connection.Client.ConnectTimeout", "1");

        await using var communicator = initialize(initData);
        Test.MetricsPrx metrics = await AllTests.allTests(this, observer);
        metrics.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
