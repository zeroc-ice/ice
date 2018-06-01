// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.ami;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        properties.setProperty("Ice.Warn.AMICallback", "0");
        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("ControllerAdapter");

            adapter.add(new TestI(), com.zeroc.Ice.Util.stringToIdentity("test"));
            adapter.add(new TestII(), com.zeroc.Ice.Util.stringToIdentity("test2"));
            //adapter.activate(); // Collocated test doesn't need to activate the OA
            adapter2.add(new TestControllerI(adapter), com.zeroc.Ice.Util.stringToIdentity("testController"));
            //adapter2.activate(); // Collocated test doesn't need to activate the OA

            AllTests.allTests(this, true);
        }
    }
}
