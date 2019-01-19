//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.admin;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.admin");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new RemoteCommunicatorFactoryI(), Ice.Util.stringToIdentity("factory"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
