//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Threading.Tasks;
using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Metrics
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            var observer = new CommunicatorObserver();

            Dictionary<string, string>? properties = CreateTestProperties(ref args);
            properties["Ice.Admin.Endpoints"] = "tcp";
            properties["Ice.Admin.InstanceName"] = "client";
            properties["Ice.Admin.DelayCreation"] = "1";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.Default.Host"] = "127.0.0.1";

            await using Communicator communicator = Initialize(properties, observer: observer);
            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("metrics", new Metrics());
            //adapter.activate(); // Don't activate OA to ensure collocation is used.

            IMetricsPrx metrics = AllTests.Run(this, observer);
            await metrics.ShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Collocated>(args);
    }
}
