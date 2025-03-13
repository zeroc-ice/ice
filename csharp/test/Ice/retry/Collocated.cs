// Copyright (c) ZeroC, Inc.

using Test;

namespace Ice.retry;

public class Collocated : TestHelper
{
    public override async Task runAsync(string[] args)
    {
        var initData = new InitializationData();
        initData.observer = Instrumentation.getObserver();

        initData.properties = createTestProperties(ref args);
        initData.properties.setProperty("Ice.RetryIntervals", "0 1 10 1");
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");

        //
        // This test kills connections, so we don't want warnings.
        //
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        using (var communicator = initialize(initData))
        {
            //
            // Configure a second communicator for the invocation timeout
            // + retry test, we need to configure a large retry interval
            // to avoid time-sensitive failures.
            //
            initData.properties.setProperty("Ice.RetryIntervals", "0 1 10000");
            using (var communicator2 = initialize(initData))
            {
                communicator.createObjectAdapter("").add(new RetryI(), Ice.Util.stringToIdentity("retry"));
                communicator2.createObjectAdapter("").add(new RetryI(), Ice.Util.stringToIdentity("retry"));

                Test.RetryPrx retry = await AllTests.allTests(this, communicator, communicator2, "retry");
                retry.shutdown();
            }
        }
    }
    public static Task<int> Main(string[] args) =>
        TestDriver.runTestAsync<Collocated>(args);
}
