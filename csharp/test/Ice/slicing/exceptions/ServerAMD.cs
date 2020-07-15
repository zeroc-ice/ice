//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Slicing.Exceptions
{
    public class ServerAMD : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            await using Communicator communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", $"{GetTestEndpoint(0)} -t 2000");
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("Test", new TestIntfAsync());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<ServerAMD>(args);
    }
}
