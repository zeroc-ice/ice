//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        using (var communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 2000");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new TestI(), "Test");
            adapter.Activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
