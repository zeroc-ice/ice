//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Reflection;
using Test;

[assembly: AssemblyTitle("IceTest")]
[assembly: AssemblyDescription("Ice test")]
[assembly: AssemblyCompany("ZeroC, Inc.")]

public class Server : TestHelper
{
    private class Echo : IEcho
    {
        public void shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
    }

    public override void Run(string[] args)
    {
        using Ice.Communicator communicator = Initialize(ref args);
        communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        var blob = new BlobjectI();
        adapter.AddDefault(blob);
        adapter.Add("__echo", new Echo());
        adapter.Activate();
        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
