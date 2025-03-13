// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        if (args.Length < 1)
        {
            throw new ArgumentException("Usage: client testdir");
        }

        PlatformTests.allTests(this, args[0]);
        Test.ServerFactoryPrx factory;
        factory = AllTests.allTests(this, args[0]);
        factory.shutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
