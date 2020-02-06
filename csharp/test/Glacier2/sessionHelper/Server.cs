//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;
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
            communicator.SetProperty("DeactivatedAdapter.Endpoints", getTestEndpoint(1));
            communicator.CreateObjectAdapter("DeactivatedAdapter");

            communicator.SetProperty("CallbackAdapter.Endpoints", getTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("CallbackAdapter");
            var callbackI = new Callback();
            adapter.Add("callback", callbackI);
            adapter.Activate();
            communicator.WaitForShutdown();
        }
    }

    public static int Main(string[] args) => TestDriver.runTest<Server>(args);
}
