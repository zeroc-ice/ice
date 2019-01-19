//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.acm;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.acm");
        properties.setProperty("Ice.Warn.Connections", "0");
        properties.setProperty("Ice.ACM.Timeout", "1");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Identity id = Ice.Util.stringToIdentity("communicator");
            adapter.add(new RemoteCommunicatorI(), id);
            adapter.activate();

            // Disable ready print for further adapters.
            communicator.getProperties().setProperty("Ice.PrintAdapterReady", "0");
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
