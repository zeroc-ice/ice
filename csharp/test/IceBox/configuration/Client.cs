//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Client : Test.TestHelper
{
    public override void run(string[] args)
    {
        Ice.Properties properties = createTestProperties(ref args);
        properties.setProperty("Ice.Default.Host", "127.0.0.1");
        using(var communicator = initialize(properties))
        {
            AllTests.allTests(this);
            //
            // Shutdown the IceBox server.
            //
            Ice.ObjectPrx prx = communicator.stringToProxy("DemoIceBox/admin -f Process:default -p 9996");
            Ice.ProcessPrxHelper.uncheckedCast(prx).shutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Client>(args);
    }
}
