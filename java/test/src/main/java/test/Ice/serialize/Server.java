// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.serialize;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.serialize");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            com.zeroc.Ice.Identity ident = com.zeroc.Ice.Util.stringToIdentity("initial");
            adapter.add(new InitialI(adapter, ident), ident);
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
