// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.AMI
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            Communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            adapter.Add("test", new TestIntf());
            adapter.Add("test2", new TestIntf2());
            await adapter.ActivateAsync();

            ObjectAdapter adapter2 = Communicator.CreateObjectAdapter("TestAdapter2", serializeDispatch: true);
            adapter2.Add("serialized", new TestIntf());
            await adapter2.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            // This test kills connections, so we don't want warnings.
            properties["Ice.Warn.Connections"] = "0";
            // Limit the recv buffer size, this test relies on the socket send() blocking after sending a given amount
            // of data.
            properties["Ice.TCP.RcvSize"] = "50K";
            // The client sends large payloads to block in send()
            properties["Ice.IncomingFrameMaxSize"] = "15M";

            await using var communicator = CreateCommunicator(properties);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
