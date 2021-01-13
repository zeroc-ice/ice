// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Exceptions
{
    public class ServerAMD : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Dictionary<string, string> properties = Communicator.GetProperties();
            Communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(0));
            Communicator.SetProperty("TestAdapter2.Endpoints", GetTestEndpoint(1));
            Communicator.SetProperty("TestAdapter2.IncomingFrameMaxSize", "0");
            Communicator.SetProperty("TestAdapter3.Endpoints", GetTestEndpoint(2));
            Communicator.SetProperty("TestAdapter3.IncomingFrameMaxSize", "1K");

            ObjectAdapter adapter = Communicator.CreateObjectAdapter("TestAdapter");
            ObjectAdapter adapter2 = Communicator.CreateObjectAdapter("TestAdapter2");
            ObjectAdapter adapter3 = Communicator.CreateObjectAdapter("TestAdapter3");
            var obj = new AsyncThrower();
            ZeroC.Ice.IObjectPrx prx = adapter.Add("thrower", obj, ZeroC.Ice.IObjectPrx.Factory);
            adapter2.Add("thrower", obj);
            adapter3.Add("thrower", obj);
            await adapter.ActivateAsync();
            await adapter2.ActivateAsync();
            await adapter3.ActivateAsync();

            await using var communicator2 = new Communicator(properties);
            communicator2.SetProperty("ForwarderAdapter.Endpoints", GetTestEndpoint(3));
            communicator2.SetProperty("ForwarderAdapter.IncomingFrameMaxSize", "0");
            ObjectAdapter forwarderAdapter = communicator2.CreateObjectAdapter("ForwarderAdapter");
            forwarderAdapter.Add("forwarder", new Forwarder(IObjectPrx.Parse(GetTestProxy("thrower"), communicator2)));
            await forwarderAdapter.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            Dictionary<string, string> properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Dispatch"] = "0";
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.IncomingFrameMaxSize"] = "10K";

            await using var communicator = CreateCommunicator(properties);
            return await RunTestAsync<ServerAMD>(communicator, args);
        }
    }
}
