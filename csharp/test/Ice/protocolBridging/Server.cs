//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.ProtocolBridging
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await using Communicator communicator = Initialize(ref args);
            communicator.SetProperty("TestAdapterForwarder.Endpoints", GetTestEndpoint(0));

            string transport = GetTestTransport();

            if (GetTestProtocol() == Protocol.Ice1)
            {
                communicator.SetProperty("TestAdapterSame.Endpoints", $"{transport} -h localhost");
                communicator.SetProperty("TestAdapterOther.Endpoints", $"ice+{transport}://localhost:0");
            }
            else
            {
                communicator.SetProperty("TestAdapterSame.Endpoints", $"ice+{transport}://localhost:0");
                communicator.SetProperty("TestAdapterOther.Endpoints", $"{transport} -h localhost");
            }

            ObjectAdapter adapterForwarder = communicator.CreateObjectAdapter("TestAdapterForwarder");
            ObjectAdapter adapterSame = communicator.CreateObjectAdapter("TestAdapterSame");
            ObjectAdapter adapterOther = communicator.CreateObjectAdapter("TestAdapterOther");

            ITestIntfPrx samePrx = adapterSame.Add("TestSame", new TestI(), ITestIntfPrx.Factory);
            ITestIntfPrx otherPrx = adapterOther.Add("TestOther", new TestI(), ITestIntfPrx.Factory);

            adapterForwarder.Add("ForwardSame", new Forwarder(samePrx));
            adapterForwarder.Add("ForwardOther", new Forwarder(otherPrx));

            await adapterForwarder.ActivateAsync();
            await adapterSame.ActivateAsync();
            await adapterOther.ActivateAsync();
            ServerReady();
            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
