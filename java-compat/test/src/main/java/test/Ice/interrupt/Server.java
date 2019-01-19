//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.interrupt;

public class Server extends test.TestHelper
{
    public void
    run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.interrupt");

        //
        // We need to enable the ThreadInterruptSafe property so that Ice is
        // interrupt safe for this test.
        //
        properties.setProperty("Ice.ThreadInterruptSafe", "1");

        //
        // We need to send messages large enough to cause the transport
        // buffers to fill up.
        //
        properties.setProperty("Ice.MessageSizeMax", "20000");

        //
        // opIndempotent raises UnknownException, we disable dispatch
        // warnings to prevent warnings.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.RcvSize", "50000");

        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

            TestControllerI controller = new TestControllerI(adapter);
            adapter.add(new TestI(controller), Ice.Util.stringToIdentity("test"));
            adapter.activate();
            adapter2.add(controller, Ice.Util.stringToIdentity("testController"));
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
