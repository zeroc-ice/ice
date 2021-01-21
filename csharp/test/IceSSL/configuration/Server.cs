// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: server testdir");
            }

            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0, "tcp"));
            Communicator.SetProperty("TestAdapter.AcceptNonSecure", "Always");

            Ice.ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("factory", new ServerFactory(args[0] + "/../certs"));
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
