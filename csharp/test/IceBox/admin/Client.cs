//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Ice;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void Run(string[] args)
    {
        var properties = CreateTestProperties(ref args);
        properties["Ice.Default.Host"] = "127.0.0.1";
        using var communicator = Initialize(properties);
        AllTests.allTests(this);
        //
        // Shutdown the IceBox server.
        //
        IProcessPrx.Parse("DemoIceBox/admin -f Process:default -p 9996", communicator).Shutdown();
    }

    public static int Main(string[] args) => Test.TestDriver.RunTest<Client>(args);
}
