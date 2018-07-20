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
        Ice.Properties properties = createTestProperties(args);
        properties.setProperty("Ice.Package.Test", "test.Ice.ami");
        properties.setProperty("Ice.Warn.AMICallback", "0");

        try(Ice.Communicator communicator = initialize(properties))
        {
            properties.setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
            //adapter.activate(); // Collocated test doesn't need to activate the OA

            properties.setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            properties.setProperty("ControllerAdapter.ThreadPool.Size", "1");
            Ice.ObjectAdapter adapter2  = communicator.createObjectAdapter("ControllerAdapter");
            adapter2.add(new TestControllerI(adapter), Ice.Util.stringToIdentity("testController"));
            //adapter2.activate(); // Collocated test doesn't need to activate the OA

            AllTests.allTests(this, true);
        }
    }
}
