//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.retry;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.retry");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Warn.Connections", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new RetryI(), com.zeroc.Ice.Util.stringToIdentity("retry"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
