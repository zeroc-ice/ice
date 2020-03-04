//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        var properties = createTestProperties(ref args);

        //
        // For this test, we want to disable retries.
        //
        properties["Ice.RetryIntervals"] = "-1";

        //
        // This test kills connections, so we don't want warnings.
        //
        properties["Ice.Warn.Connections"] = "0";

        // This test relies on filling the TCP send/recv buffer, so
        // we rely on a fixed value for these buffers.
        properties["Ice.TCP.SndSize"] = "50000";

        //
        // Setup the test transport plug-in.
        //
        string? transport;
        if (!properties.TryGetValue("Ice.Default.Transport", out transport))
        {
            transport = "tcp";
        }
        properties["Ice.Default.Transport"] = $"test-{transport}";

        using var communicator = initialize(properties);
        Plugin plugin = new Plugin(communicator);
        plugin.Initialize();
        communicator.AddPlugin("Test", plugin);
        Test.IBackgroundPrx background = AllTests.allTests(this);
        background.shutdown();
    }

    public static int Main(string[] args) => Test.TestDriver.runTest<Client>(args);
}
