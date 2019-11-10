//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;

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
                num = Int32.Parse(args[0]);
            }
            catch (FormatException)
            {
            }

            communicator.getProperties().setProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
            communicator.getProperties().setProperty("ControlAdapter.AdapterId", "control" + num);
            communicator.getProperties().setProperty("ControlAdapter.ThreadPool.Size", "1");

            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
            adapter.add(new ControllerI(), Ice.Util.stringToIdentity("controller" + num));
            adapter.activate();

            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
