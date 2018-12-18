// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.exceptions;

public class Server extends test.TestHelper
{
    public void run(String[] args)
    {
        Ice.InitializationData initData = new Ice.InitializationData();
        // For this test, we need a dummy logger, otherwise the
        // assertion test will print an error message.
        //
        initData.logger = new DummyLogger();

        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Warn.Dispatch", "0");
        initData.properties.setProperty("Ice.Warn.Connections", "0");
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.exceptions");
        initData.properties.setProperty("Ice.MessageSizeMax", "10"); // 10KB max

        try(Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter2.MessageSizeMax", "0");
            communicator.getProperties().setProperty("TestAdapter3.Endpoints", getTestEndpoint(2));
            communicator.getProperties().setProperty("TestAdapter3.MessageSizeMax", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            Ice.ObjectAdapter adapter3 = communicator.createObjectAdapter("TestAdapter3");
            Ice.Object object = new ThrowerI();

            adapter.add(object, Ice.Util.stringToIdentity("thrower"));
            adapter2.add(object, Ice.Util.stringToIdentity("thrower"));
            adapter3.add(object, Ice.Util.stringToIdentity("thrower"));

            adapter.activate();
            adapter2.activate();
            adapter3.activate();

            serverReady();
            communicator.waitForShutdown();
        }
    }
}
