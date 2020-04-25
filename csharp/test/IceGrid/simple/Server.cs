//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

using Ice;

public class Server : TestHelper
{
    public override void Run(string[] args)
    {
        var properties = new Dictionary<string, string>();
        properties.ParseArgs(ref args, "TestAdapter");

        using var communicator = Initialize(ref args, properties);
        ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
        adapter.Add(communicator.GetProperty("Identity") ?? "test", new TestIntf());
        try
        {
            adapter.Activate();
        }
        catch (ObjectAdapterDeactivatedException)
        {
        }
        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
