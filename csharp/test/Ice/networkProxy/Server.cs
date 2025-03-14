// Copyright (c) ZeroC, Inc.

using System.Reflection;

[assembly: CLSCompliant(true)]

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : Test.TestHelper
{
    private class TestI : Test.TestIntfDisp_
    {
        public override void shutdown(Ice.Current current)
        {
            current.adapter.getCommunicator().shutdown();
        }
    }

    public override void run(string[] args)
    {
        using var communicator = initialize(ref args);
        communicator.getProperties().setProperty("TestAdapter.Endpoints", getTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");
        adapter.add(new TestI(), Ice.Util.stringToIdentity("test"));
        adapter.activate();

        communicator.waitForShutdown();
    }

    public static Task<int> Main(string[] args) =>
        Test.TestDriver.runTestAsync<Server>(args);
}
