// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Ice.Communicator communicator = Initialize(ref args);
            await communicator.ActivateAsync();
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: server testdir");
            }

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0, "tcp"));
            communicator.SetProperty("TestAdapter.AcceptNonSecure", "Always");
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("factory", new ServerFactory(args[0] + "/../certs"));
            await adapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
