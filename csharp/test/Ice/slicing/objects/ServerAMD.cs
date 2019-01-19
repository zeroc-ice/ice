//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Warn.Dispatch", "0");
        using(var communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 2000");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), Ice.Util.stringToIdentity("Test"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
