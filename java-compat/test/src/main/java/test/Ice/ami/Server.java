// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.Ice.ami;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.RcvSize", "50000");

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
            adapter.add(new TestII(), Ice.Util.stringToIdentity("test2"));
            adapter.activate();

            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
            Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");
            adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
