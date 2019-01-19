//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.acm;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.acm");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.ACM.Timeout", "1");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint());
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Identity id = com.zeroc.Ice.Util.stringToIdentity("communicator");
            adapter.add(new RemoteCommunicatorI(), id);
            adapter.activate();

            // Disable ready print for further adapters.
            communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
