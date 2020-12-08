// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var observer = new CommunicatorObserver();

            Dictionary<string, string> properties = CreateTestProperties(ref args);
            bool ice1 = GetTestProtocol(properties) == Protocol.Ice1;

            properties["Ice.Admin.Endpoints"] = ice1 ? "tcp -h 127.0.0.1" : "ice+tcp://127.0.0.1:0";
            properties["Ice.Admin.InstanceName"] = "colocated";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.InvocationMaxAttempts"] = "2";

            await using Communicator communicator = Initialize(properties, observer: observer);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            // Don't activate OA to ensure collocation is used.

            IMetricsPrx metrics = AllTests.Run(this, observer, colocated: true);
            await metrics.ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Collocated>(args);
    }
}
