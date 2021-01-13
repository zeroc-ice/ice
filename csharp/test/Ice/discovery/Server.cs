// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Discovery
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            int num = 0;
            try
            {
                num = int.Parse(args[0]);
            }
            catch (FormatException)
            {
            }

            await Communicator.ActivateAsync();
            Communicator.SetProperty("ControlAdapter.Endpoints", GetTestEndpoint(num));
            Communicator.SetProperty("ControlAdapter.AdapterId", $"control{num}");

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("ControlAdapter");
            adapter.Add($"controller{num}", new Controller());
            adapter.Add($"faceted-controller{num}#abc", new Controller());
            await adapter.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
