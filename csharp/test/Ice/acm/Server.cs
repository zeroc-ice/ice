// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.ACM
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.ACM.Timeout"] = "1s";
            await using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("TestAdapter.ACM.Timeout", "infinite");
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("communicator", new RemoteCommunicator());
            await adapter.ActivateAsync();
            ServerReady();
            communicator.SetProperty("Ice.PrintAdapterReady", "0");
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
