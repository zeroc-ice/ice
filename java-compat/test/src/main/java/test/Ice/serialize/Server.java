//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.serialize;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.serialize");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            Ice.Identity ident = Ice.Util.stringToIdentity("initial");
            Ice.Object object = new InitialI(adapter, ident);
            adapter.add(object, ident);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
