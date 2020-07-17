//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Echo
{
    public class Server : TestHelper
    {
        private class Echo : IEcho
        {
            public void Shutdown(Current current) => current.Adapter.Communicator.ShutdownAsync();
        }

        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            var blob = new BlobjectI();
            adapter.AddDefault(blob);
            adapter.Add("__echo", new Echo());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
