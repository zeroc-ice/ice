// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().parseCommandLineOptions("TestAdapter", args);
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        string id = communicator.getProperties().getPropertyWithDefault("Identity", "test");
        adapter.add(new TestI(), Ice.Util.stringToIdentity(id));
        adapter.activate();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
