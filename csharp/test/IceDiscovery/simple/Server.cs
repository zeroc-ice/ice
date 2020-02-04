//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        int num = 0;
        try
        {
            num = int.Parse(args[0]);
        }
        catch (FormatException)
        {
        }

        communicator.SetProperty("ControlAdapter.Endpoints", getTestEndpoint(num));
        communicator.SetProperty("ControlAdapter.AdapterId", $"control{num}");
        communicator.SetProperty("ControlAdapter.ThreadPool.Size", "1");

        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
        adapter.Add($"controller{num}", new Controller());
        adapter.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.runTest<Server>(args);
}
