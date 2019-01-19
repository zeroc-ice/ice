//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.slicing.exceptions;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.slicing.exceptions.server");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0) + " -t 2000");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Object object = new TestI();
            adapter.add(object, Ice.Util.stringToIdentity("Test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
