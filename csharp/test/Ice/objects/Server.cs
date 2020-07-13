//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Objects
{
    public class Server : TestHelper
    {
        public override async Task Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            using var communicator = Initialize(properties);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("initial", new Initial(adapter));
            adapter.Add("F21", new F2());
            var uoet = new UnexpectedObjectExceptionTest();
            adapter.Add("uoet", uoet);
            await adapter.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
