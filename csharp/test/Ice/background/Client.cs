// Copyright (c) ZeroC, Inc.

namespace Ice.background;

public class Client : Test.TestHelper
{
    public override async Task runAsync(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);

        //
        // For this test, we want to disable retries.
        //
        properties.setProperty("Ice.RetryIntervals", "-1");

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        properties.setProperty("Ice.TCP.SndSize", "50000");

        //
        // Setup the test transport plug-in.
        //
        properties.setProperty("Ice.Default.Protocol", "test-" + properties.getIceProperty("Ice.Default.Protocol"));

        await using Ice.Communicator communicator = initialize(properties);
        var plugin = new PluginI(communicator);
        plugin.initialize();
        communicator.getPluginManager().addPlugin("Test", plugin);
        Test.BackgroundPrx background = await AllTests.allTests(this);
        background.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
