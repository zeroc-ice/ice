//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceSSL.configuration;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.IceSSL.configuration");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "tcp"));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.Identity id = Ice.Util.stringToIdentity("factory");
            adapter.add(new ServerFactoryI(), id);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
