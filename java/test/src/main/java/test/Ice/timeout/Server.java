//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.timeout;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.timeout");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.RcvSize", "50000");

        //
        // The client sends large messages to cause the transport
        // buffers to fill up.
        //
        properties.setProperty("Ice.MessageSizeMax", "20000");

        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TimeoutI(), com.zeroc.Ice.Util.stringToIdentity("timeout"));
            adapter.activate();

            com.zeroc.Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(new ControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
