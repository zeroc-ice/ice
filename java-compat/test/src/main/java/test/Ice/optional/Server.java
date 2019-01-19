//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.optional;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.optional");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new InitialI(), Ice.Util.stringToIdentity("initial"));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
