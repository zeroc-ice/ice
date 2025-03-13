// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.retry;

public class Client : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new Ice.InitializationData();
        initData.observer = Instrumentation.getObserver();

        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        using var communicator = initialize(initData);
        //
        // Configure a second communicator for the invocation timeout
        // + retry test, we need to configure a large retry interval
        // to avoid time-sensitive failures.
        //
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
        initData.observer = Instrumentation.getObserver();
        using var communicator2 = initialize(initData);
        var retry = await AllTests.allTests(this, communicator, communicator2, "retry:" + getTestEndpoint(0));
        await retry.shutdownAsync();
    }

    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Client>(args);
}
