// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery.Test.Simple
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string>? properties = CreateTestProperties(ref args);
            // TODO: we currently force ice1 for this test because Test.Protocol is the only way to select
            // the protocol used by object adapters. Once this is fixed, we should run this test with both ice1 and
            // ice2, and use only ice1 for the udp object adapter.
            properties["Test.Protocol"] = "ice1";

            await using Communicator communicator = Initialize(properties);
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

            ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
            adapter.Add($"controller{num}", new Controller());
            await adapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
