//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Slicing.Objects
{
    public class Server : TestHelper
    {
        public override async Task Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", $"{GetTestEndpoint(0)} -t 2000");
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("Test", new TestIntf());
            adapter.Add("Test2", new TestIntf2());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
