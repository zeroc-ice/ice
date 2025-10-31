// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace Ice.networkProxy;

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Communicator communicator = initialize(ref args);
        AllTests.allTests(this);
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Client>(args);
}
