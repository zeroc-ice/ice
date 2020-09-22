// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.Warn.Connections"] = "0";
            await using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("retry", new Retry());
            await adapter.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
