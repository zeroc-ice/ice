//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Server : TestHelper
{
    public override void Run(string[] args)
    {
        var properties = CreateTestProperties(ref args);
        properties["Ice.Warn.Dispatch"] = "0";
        using var communicator = Initialize(properties);
        communicator.SetProperty("TestAdapter.Endpoints", $"{GetTestEndpoint(0)} -t 2000");
        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add("Test", new TestIntf());
        adapter.Activate();
        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
