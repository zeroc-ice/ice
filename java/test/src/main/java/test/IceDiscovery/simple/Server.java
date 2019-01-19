//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.IceDiscovery.simple;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        java.util.List<String> rargs = new java.util.ArrayList<String>();
        com.zeroc.Ice.Properties properties = createTestProperties(args, rargs);
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            int num = Integer.parseInt(rargs.get(0));
            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
            communicator.getProperties().setProperty("ControlAdapter.AdapterId", "control" + num);
            communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

            com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("ControlAdapter");
            adapter.add(new ControllerI(), com.zeroc.Ice.Util.stringToIdentity("controller" + num));
            adapter.activate();
            serverReady();
            communicator.waitForShutdown();
        }
    }
}
