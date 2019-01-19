//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.networkProxy;

import test.Ice.networkProxy.Test.TestIntf;

public class Server extends test.TestHelper
{
    static public class TestI implements TestIntf
    {
        public void shutdown(com.zeroc.Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.networkProxy");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
