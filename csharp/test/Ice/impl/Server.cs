//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Reflection;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using (var communicator = initialize(ref args))
        {
            //
            // We don't want connection warnings because of the timeout test.
            //
            communicator.getProperties().setProperty("Ice.Warn.Connections", "0");

            communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
            adapter.add(new Test.MyDerivedClassI(), Ice.Util.stringToIdentity("test"));
            adapter.activate();

            communicator.waitForShutdown();
        }
    }

    public static int Main(string[] args)
    {
        return Test.TestDriver.runTest<Server>(args);
    }
}
