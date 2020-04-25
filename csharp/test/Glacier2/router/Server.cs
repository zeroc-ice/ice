//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using Test;

public class Server : Test.TestHelper
{
    public override void Run(string[] args)
    {
        using (var communicator = Initialize(ref args))
        {
            communicator.SetProperty("CallbackAdapter.Endpoints", GetTestEndpoint(0));
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("CallbackAdapter");

            //
            // The test allows "c1" as category.
            //
            adapter.Add("c1/callback", new Callback());

            //
            // The test allows "c2" as category.
            //
            adapter.Add("c2/callback", new Callback());

            //
            // The test rejects "c3" as category.
            //
            adapter.Add("c3/callback", new Callback());

            //
            // The test allows the prefixed userid.
            //
            adapter.Add("_userid/callback", new Callback());
            adapter.Activate();
            communicator.WaitForShutdown();
        }
    }

    public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
}
