// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Operations
{
    public class ServerAMD : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new AsyncMyDerivedClass());
            await adapter.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            // We don't want connection warnings because of the timeout test.
            properties["Ice.Warn.Connections"] = "0";

            await using var communicator = CreateCommunicator(properties);
            return await RunTestAsync<ServerAMD>(communicator, args);
        }
    }
}
