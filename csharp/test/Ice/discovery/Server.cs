// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Discovery
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            await communicator.ActivateAsync();

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
            adapter.Add($"faceted-controller{num}#abc", new Controller());
            await adapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
