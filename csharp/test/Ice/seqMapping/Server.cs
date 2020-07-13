//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.SeqMapping
{
    public class Server : TestHelper
    {
        public override async Task Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            var adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new MyClass());
            await adapter.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
