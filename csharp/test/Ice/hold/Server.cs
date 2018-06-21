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
        using(var communicator = initialize(ref args))
        {
            Timer timer = new Timer();

            communicator.getProperties().setProperty("TestAdapter1.Endpoints", getTestEndpoint(0));
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter1.ThreadPool.Serialize", "0");
            Ice.ObjectAdapter adapter1 = communicator.createObjectAdapter("TestAdapter1");
            adapter1.add(new HoldI(timer, adapter1), Ice.Util.stringToIdentity("hold"));

            communicator.getProperties().setProperty("TestAdapter2.Endpoints", getTestEndpoint(1));
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Size", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeMax", "5");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.SizeWarn", "0");
            communicator.getProperties().setProperty("TestAdapter2.ThreadPool.Serialize", "1");
            Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter2");
            adapter2.add(new HoldI(timer, adapter2), Ice.Util.stringToIdentity("hold"));

            adapter1.activate();
            adapter2.activate();

            communicator.waitForShutdown();

            timer.shutdown();
            timer.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
