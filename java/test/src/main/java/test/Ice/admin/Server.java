// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.admin;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.admin");
        properties.setProperty("Ice.Warn.Dispatch", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint());
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new RemoteCommunicatorFactoryI(), com.zeroc.Ice.Util.stringToIdentity("factory"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
