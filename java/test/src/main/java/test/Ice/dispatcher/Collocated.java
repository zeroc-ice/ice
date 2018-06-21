// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.dispatcher;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.InitializationData initData = new com.zeroc.Ice.InitializationData();
        Dispatcher dispatcher = new Dispatcher();
        initData.properties = createTestProperties(args);
        initData.properties.setProperty("Ice.Package.Test", "test.Ice.dispatcher");
        initData.dispatcher = dispatcher;
        try(com.zeroc.Ice.Communicator communicator = initialize(initData))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1, "tcp"));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(dispatcher), com.zeroc.Ice.Util.stringToIdentity("test"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            adapter2.add(new TestControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("testController"));
            //adapter2.activate(); // Don't activate OA to ensure collocation is used.

            AllTests.allTests(this, dispatcher);
        }
        dispatcher.terminate();
    }
}
