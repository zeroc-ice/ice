// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.ServerIdleTime"] = "120";
            int port = 0;
            for (int i = 0; i < args.Length; i++)
            {
                if (args[i][0] == '-')
                {
                    throw new ArgumentException("Server: unknown option `" + args[i] + "'");
                }

                if (port != 0)
                {
                    throw new ArgumentException("Server: only one port can be specified");
                }

                try
                {
                    port = int.Parse(args[i]);
                }
                catch (FormatException)
                {
                    throw new ArgumentException("Server: invalid port");
                }
            }

            if (port <= 0)
            {
                throw new ArgumentException("Server: no port specified");
            }

            await using Communicator communicator = Initialize(properties);
            await communicator.ActivateAsync();
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(port));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            await adapter.ActivateAsync();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
