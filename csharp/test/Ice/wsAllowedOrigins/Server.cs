// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace Ice.wsAllowedOrigins;

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        // The C# test framework's TestHelper only consumes --Test.* args; this test relies on the framework passing
        // --TestAdapter.AllowedOrigins=..., so we consume the TestAdapter prefix explicitly here.
        args = communicator.getProperties().parseCommandLineOptions("TestAdapter", args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "ws"));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.activate();
        serverReady();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
