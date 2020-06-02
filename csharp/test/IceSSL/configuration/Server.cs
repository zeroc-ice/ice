//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            using var communicator = Initialize(ref args);
            if (args.Length < 1)
            {
                throw new ArgumentException("Usage: server testdir");
            }

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0, "tcp"));
            ZeroC.Ice.ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("factory", new ServerFactory(args[0] + "/../certs"));
            adapter.Activate();

            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
