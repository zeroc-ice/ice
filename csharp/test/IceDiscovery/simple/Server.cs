//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

using ZeroC.Ice;

namespace ZeroC.IceDiscovery.Test.Simple
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
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

            ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
            adapter.Add($"controller{num}", new Controller());
            await adapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
