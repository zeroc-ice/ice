// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;
using ZeroC.Ice;

namespace ZeroC.Glacier2.Test.Router
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Test.Protocol"] = "ice1";

            await using Communicator communicator = Initialize(properties);
            await communicator.ActivateAsync();
            communicator.SetProperty("CallbackAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("CallbackAdapter");

            // The test allows "c1" as category.
            adapter.Add("c1/callback", new Callback());

            // The test allows "c2" as category.
            adapter.Add("c2/callback", new Callback());

            // The test rejects "c3" as category.
            adapter.Add("c3/callback", new Callback());

            // The test allows the prefixed userid.
            adapter.Add("_userid/callback", new Callback());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
