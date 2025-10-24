// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace IceGrid.simple;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        if (args.Any(v => v == "--with-deploy"))
        {
            AllTests.allTestsWithDeploy(this);
        }
        else
        {
            AllTests.allTests(this);
        }
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
