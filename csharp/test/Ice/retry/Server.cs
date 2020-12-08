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
            await communicator.ActivateAsync();
            communicator.SetProperty("TestAdapter1.Endpoints", GetTestEndpoint(0));
            var adapter1 = communicator.CreateObjectAdapter("TestAdapter1");
            adapter1.Add("retry", new Retry());
            adapter1.Add("replicated", new Replicated(true));
            adapter1.Add("nonreplicated", new NonReplicated());
            await adapter1.ActivateAsync();

            communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));
            var adapter2 = communicator.CreateObjectAdapter("TestAdapter2");
            adapter2.Add("replicated", new Replicated(false));
            await adapter2.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
