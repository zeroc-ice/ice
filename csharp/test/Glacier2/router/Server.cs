// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

namespace Glacier2.router;

public class Server : Test.TestHelper
{
    public override void run(string[] args)
    {
        using Ice.Communicator communicator = initialize(ref args);
        communicator.getProperties().setProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("CallbackAdapter");

        //
        // The test allows "c1" as category.
        //
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c1/callback"));

        //
        // The test allows "c2" as category.
        //
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c2/callback"));

        //
        // The test rejects "c3" as category.
        //
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("c3/callback"));

        //
        // The test allows the prefixed userid.
        //
        adapter.add(new CallbackI(), Ice.Util.stringToIdentity("_userid/callback"));
        adapter.activate();
        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
