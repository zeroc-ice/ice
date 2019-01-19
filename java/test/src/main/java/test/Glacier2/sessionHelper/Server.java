//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Glacier2.sessionHelper;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
            communicator.createObjectAdapter("DeactivatedAdapter");

            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");
            adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("callback"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
