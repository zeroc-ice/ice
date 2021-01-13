// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Threading;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Echo
{
    public class Server : TestHelper
    {
        private class Echo : IEcho
        {
            public void Shutdown(Current current, CancellationToken cancel) =>
                current.Communicator.ShutdownAsync();
        }

        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            var blob = new BlobjectI();
            adapter.AddDefault(blob);
            adapter.Add("__echo", new Echo());
            await adapter.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
