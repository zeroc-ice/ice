//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using System.Collections.Generic;
using Test;

using Ice;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        var properties = new Dictionary<string, string>();
        properties.ParseArgs(ref args, "TestAdapter");

        using (var communicator = initialize(ref args, properties))
        {
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.Add(new TestI(), communicator.GetProperty("Identity") ?? "test");
            try
            {
                adapter.Activate();
            }
            catch (ObjectAdapterDeactivatedException)
            {
            }
            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
