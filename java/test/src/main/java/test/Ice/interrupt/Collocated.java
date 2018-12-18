// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.interrupt;

public class Collocated extends test.TestHelper
{
    public void run(String[] args)
    {
        com.zeroc.Ice.Properties properties = createTestProperties(args);
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
        // opIdempotent raises UnknownException, we disable dispatch
        // warnings to prevent warnings.
        //
        properties.setProperty("Ice.Warn.Dispatch", "0");

        try(com.zeroc.Ice.Communicator communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            com.zeroc.Ice.ObjectAdapter adapter = communicator().createObjectAdapter("TestAdapter");
            com.zeroc.Ice.ObjectAdapter adapter2 = communicator().createObjectAdapter("ControllerAdapter");
            TestControllerI controller = new TestControllerI(adapter);
            adapter.add(new TestI(controller), com.zeroc.Ice.Util.stringToIdentity("test"));
            //adapter.activate(); // Don't activate OA to ensure collocation is used.
            adapter2.add(controller, com.zeroc.Ice.Util.stringToIdentity("testController"));
            //adapter2.activate(); // Don't activate OA to ensure collocation is used.
            AllTests.allTests(this);
        }
        catch(InterruptedException ex)
        {
            throw new RuntimeException(ex);
        }
    }
}
