// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Retry
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();

            Communicator.SetProperty("TestAdapter1.Endpoints", GetTestEndpoint(0));
            var adapter1 = Communicator.CreateObjectAdapter("TestAdapter1");
            adapter1.Add("retry", new Retry());
            adapter1.Add("replicated", new Replicated(true));
            adapter1.Add("nonreplicated", new NonReplicated());
            await adapter1.ActivateAsync();

            Communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));
            var adapter2 = Communicator.CreateObjectAdapter("TestAdapter2");
            adapter2.Add("replicated", new Replicated(false));
            await adapter2.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.Warn.Connections"] = "0";

            await using var communicator = CreateCommunicator(properties);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
