// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Glacier2.router;

public class Server extends test.TestHelper

{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Glacier2.router");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");

            //
            // The test allows "c1" as category.
            //
            adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("c1/callback"));

            //
            // The test allows "c2" as category.
            //
            adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("c2/callback"));

            //
            // The test rejects "c3" as category.
            //
            adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("c3/callback"));

            //
            // The test allows the prefixed userid.
            //
            adapter.add(new CallbackI(), com.zeroc.Ice.Util.stringToIdentity("_userid/callback"));
            adapter.activate();
            communicator.waitForShutdown();
        }
    }
}
