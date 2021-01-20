// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Collocated : TestHelper
    {
        private static CommunicatorObserverI? _observer;

        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            // Don't activate OA to ensure collocation is used.

            await AllTests.RunAsync(this, _observer!, colocated: true);
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            bool ice1 = GetTestProtocol(properties) == Protocol.Ice1;
            properties["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0";
            properties["Ice.Admin.InstanceName"] = "colocated";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.InvocationMaxAttempts"] = "2";

            _observer = new CommunicatorObserverI();
            await using var communicator = CreateCommunicator(properties, _observer);
            return await RunTestAsync<Collocated>(communicator, args);
        }
    }
}
