//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.operations;

public class AMDServer extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        //
        // It's possible to have batch oneway requests dispatched
        // after the adapter is deactivated due to thread
        // scheduling so we suppress this warning.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");
        properties.setProperty("Ice.Package.Test", "test.Ice.operations.AMD");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new AMDMyDerivedClassI(), Ice.Util.stringToIdentity("test"));
            adapter.add(new AMDBI(), Ice.Util.stringToIdentity("b"));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
