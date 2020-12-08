// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Info
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            await communicator.ActivateAsync();
            if (Protocol == Protocol.Ice1)
            {
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0) + ":" + GetTestEndpoint(0, "udp"));
                communicator.SetProperty("TestAdapter.AcceptNonSecure", "Always");
            }
            else
            {
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            }
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            await adapter.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
