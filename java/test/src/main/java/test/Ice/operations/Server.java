//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.operations");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new MyDerivedClassI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.add(new BI(), com.zeroc.Ice.Util.stringToIdentity("b"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
