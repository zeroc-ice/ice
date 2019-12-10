//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Diagnostics;
using System.Reflection;
using Ice;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Default.Host", "127.0.0.1");
        using (var communicator = initialize(properties))
        {
            AllTests.allTests(this);
            //
            // Shutdown the IceBox server.
            //
            ProcessPrx.Parse("DemoIceBox/admin -f Process:default -p 9996", communicator).shutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }

}
