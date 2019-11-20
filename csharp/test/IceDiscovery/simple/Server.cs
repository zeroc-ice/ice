//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            int num = 0;
            try
            {
                num = int.Parse(args[0]);
            }
            catch (FormatException)
            {
            }

            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
            communicator.getProperties().setProperty("ControlAdapter.AdapterId", $"control{num}");
            communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.Add(new ControllerI(), $"controller{num}");
            adapter.Activate();

            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return TestDriver.runTest<Server>(args);
    }
}
