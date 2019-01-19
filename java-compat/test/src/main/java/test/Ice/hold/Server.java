//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.hold;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.hold");
        try(Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("Ice.Package.Test", "test.Ice.hold");
            communicator.getProperties().setProperty("TestAdapter1.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");

            communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");

            java.util.Timer timer = new java.util.Timer();

            Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
            adapter1.add(new HoldI(timer, adapter1), Ice.Util.stringToIdentity("hold"));

            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            adapter2.add(new HoldI(timer, adapter2), Ice.Util.stringToIdentity("hold"));

            adapter1.activate();
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();

            timer.cancel();
        }
    }
}
