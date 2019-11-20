//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            communicator.getProperties().parseCommandLineOptions("TestAdapter", args);
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            string id = communicator.getProperties().getPropertyWithDefault("Identity", "test");
            adapter.Add(new TestI(), Ice.Util.stringToIdentity(id));
            try
            {
                adapter.activate();
            }
            catch (Ice.ObjectAdapterDeactivatedException)
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
