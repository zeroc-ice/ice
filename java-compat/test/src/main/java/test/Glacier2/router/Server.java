// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Glacier2.router;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");
            adapter.add(new CallbackI(),
                        Ice.Util.stringToIdentity("c1/callback")); // The test allows "c1" as category.
            adapter.add(new CallbackI(),
                        Ice.Util.stringToIdentity("c2/callback")); // The test allows "c2" as category.
            adapter.add(new CallbackI(),
                        Ice.Util.stringToIdentity("c3/callback")); // The test rejects "c3" as category.
            adapter.add(new CallbackI(),
                        Ice.Util.stringToIdentity("_userid/callback")); // The test allows the prefixed userid.
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
