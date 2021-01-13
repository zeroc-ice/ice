// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.FaultTolerance
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
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

            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(port));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            await adapter.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.ServerIdleTime"] = "120";

            await using var communicator = CreateCommunicator(properties);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
