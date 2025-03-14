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
        if (args.Length < 1)
        {
            throw new ArgumentException("Usage: server testdir");
        }

        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0, "tcp"));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        Ice.Identity id = Ice.Util.stringToIdentity("factory");
        adapter.add(new ServerFactoryI(args[0] + "/../certs"), id);
        adapter.activate();

        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
