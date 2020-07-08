//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Threading.Tasks;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Server : TestHelper
    {
        public override async Task Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: server testdir");
            }

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0, "tcp"));
            Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("factory", new ServerFactory(args[0] + "/../certs"));
            await adapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
