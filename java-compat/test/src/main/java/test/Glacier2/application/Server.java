// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Glacier2.application;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
            communicator.createObjectAdapter("DeactivatedAdapter");

            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");
            adapter.add(new CallbackI(), Ice.Util.stringToIdentity("callback"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
