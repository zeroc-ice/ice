//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Server : TestHelper
{
    public class TestIntf : ITestIntf
    {
        public void shutdown(Ice.Current current) => current.Adapter.Communicator.Shutdown();
    }

    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add("test", new TestIntf());
        adapter.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
