// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        int num;
        try
        {
            num = args.Length == 1 ? Int32.Parse(args[0]) : 0;
        }
        catch (FormatException)
        {
            num = 0;
        }
        AllTests.allTests(this, num);
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
