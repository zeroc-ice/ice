//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.optional;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.optional");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(), com.zeroc.Ice.Util.stringToIdentity("initial"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
