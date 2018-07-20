// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.hold;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.hold");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
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

            com.zeroc.Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
            adapter1.add(new HoldI(timer, adapter1), com.zeroc.Ice.Util.stringToIdentity("hold"));

            com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            adapter2.add(new HoldI(timer, adapter2), com.zeroc.Ice.Util.stringToIdentity("hold"));

            adapter1.activate();
            adapter2.activate();

            serverReady();
            communicator.waitForShutdown();

            timer.cancel();
        }
    }
}
