// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// **********************************************************************

package test.IceDiscovery.simple;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.StringSeqHolder argsH = new Ice.StringSeqHolder(args);
        Ice.Properties properties = createTestProperties(argsH);
        try(Ice.Communicator communicator = initialize(properties))
        {
            int num = 0;
            try
            {
                num = Integer.parseInt(argsH.value[0]);
            }
            catch(NumberFormatException ex)
            {
            }

            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
            communicator.getProperties().setProperty("ControlAdapter.AdapterId", "control" + num);
            communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new ControllerI(), Ice.Util.stringToIdentity("controller" + num));
            adapter.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
