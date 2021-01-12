// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.UDP
{
    public class Client : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await AllTests.RunAsync(this);

            int num;
            try
            {
                num = args.Length == 1 ? int.Parse(args[0]) : 1;
            }
            catch (FormatException)
            {
                num = 1;
            }

            for (int i = 0; i < num; ++i)
            {
                ITestIntfPrx.Parse(GetTestProxy("control", i, Transport), Communicator).Shutdown();
            }
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.UDP.SndSize"] = "16K";

            await using var communicator = CreateCommunicator(properties);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
