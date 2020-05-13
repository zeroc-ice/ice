//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

public class Server : TestHelper
{
    public override void Run(string[] args)
    {
        using var communicator = Initialize(ref args);
        int num = 0;
        try
        {
            num = int.Parse(args[0]);
        }
        catch (FormatException)
        {
        }

        communicator.SetProperty("ControlAdapter.Endpoints", GetTestEndpoint(num));
        communicator.SetProperty("ControlAdapter.AdapterId", $"control{num}");

        Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
        adapter.Add($"controller{num}", new Controller());
        adapter.Activate();

        communicator.WaitForShutdown();
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
