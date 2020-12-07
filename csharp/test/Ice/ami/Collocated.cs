// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.AMI
{
    public class Collocated : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string>? properties = CreateTestProperties(ref args);

            properties["Ice.Warn.AMICallback"] = "0";
            // Limit the send buffer size, this test relies on the socket send() blocking after sending a given amount
            // of data.
            properties["Ice.TCP.SndSize"] = "50K";

            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";

            await using Communicator communicator = Initialize(properties);
            await communicator.ActivateAsync();

            communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));

            ObjectAdapter adapter = communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            // Don't activate OA to ensure collocation is used.

            ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter2", serializeDispatch: true);
            adapter2.Add("serialized", new TestIntf());
            // Don't activate OA to ensure collocation is used.

            AllTests.Run(this);
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Collocated>(args);
    }
}
