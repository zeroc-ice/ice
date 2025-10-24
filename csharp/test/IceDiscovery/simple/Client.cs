// Copyright (c) ZeroC, Inc.

using System.Globalization;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceDiscoveryTest")]
[assembly: AssemblyDescription("IceDiscovery test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace IceDiscovery.simple;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        int num;
        try
        {
            num = args.Length == 1 ? int.Parse(args[0], CultureInfo.InvariantCulture) : 0;
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
