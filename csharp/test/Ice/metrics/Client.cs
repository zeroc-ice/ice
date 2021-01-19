// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Client : TestHelper
    {
        private static CommunicatorObserverI? _observer;

        public override Task RunAsync(string[] args) => AllTests.RunAsync(this, _observer!, colocated: false);

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            bool ice1 = GetTestProtocol(properties) == Protocol.Ice1;
            properties["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0";
            properties["Ice.Admin.InstanceName"] = "client";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.ConnectTimeout"] = "3s";
            properties["Ice.InvocationMaxAttempts"] = "2";

            _observer = new CommunicatorObserverI();
            await using var communicator = CreateCommunicator(properties, _observer);
            await communicator.ActivateAsync();
            return await RunTestAsync<Client>(communicator, args);
        }
    }
}
