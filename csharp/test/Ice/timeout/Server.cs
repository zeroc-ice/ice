// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        //
        // This test kills connections, so we don't want warnings.
        //
        properties.setProperty("Ice.Warn.Connections", "0");

        //
        // The client sends large messages to cause the transport
        // buffers to fill up.
        //
        properties.setProperty("Ice.MessageSizeMax", "20000");

        //
        // Limit the recv buffer size, this test relies on the socket
        // send() blocking after sending a given amount of data.
        //
        properties.setProperty("Ice.TCP.RcvSize", "50000");

        using(var communicator = initialize(properties))
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("ControllerAdapter.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("ControllerAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new TimeoutI(), Ice.Util.stringToIdentity("timeout"));
            adapter.activate();

            Ice.ObjectAdapter controllerAdapter = communicator.createObjectAdapter("ControllerAdapter");
            controllerAdapter.add(new ControllerI(adapter), Ice.Util.stringToIdentity("controller"));
            controllerAdapter.activate();

            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
