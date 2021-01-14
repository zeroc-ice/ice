// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading.Tasks;
using ZeroC.Test;

namespace ZeroC.Ice.Test.ProtocolBridging
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            await Communicator.ActivateAsync();
            Communicator.SetProperty("TestAdapterForwarder.Endpoints", GetTestEndpoint(0));

            var ice1Endpoint = TestHelper.GetTestEndpoint(
                new Dictionary<string, string>
                {
                    ["Test.Host"] = Communicator.GetProperty("Test.Host")!,
                    ["Test.Protocol"] = "ice1",
                    ["Test.Transport"] = Communicator.GetProperty("Test.Transport")!,
                },
                ephemeral: true);

            var ice2Endpoint = TestHelper.GetTestEndpoint(
                new Dictionary<string, string>
                {
                    ["Test.Host"] = Communicator.GetProperty("Test.Host")!,
                    ["Test.Protocol"] = "ice2",
                    ["Test.Transport"] = Communicator.GetProperty("Test.Transport")!,
                },
                ephemeral: true);

            if (Protocol == Protocol.Ice1)
            {
                Communicator.SetProperty("TestAdapterSame.Endpoints", ice1Endpoint);
                Communicator.SetProperty("TestAdapterOther.Endpoints", ice2Endpoint);
            }
            else
            {
                Communicator.SetProperty("TestAdapterSame.Endpoints", ice2Endpoint);
                Communicator.SetProperty("TestAdapterOther.Endpoints", ice1Endpoint);
            }

            ObjectAdapter adapterForwarder = Communicator.CreateObjectAdapter("TestAdapterForwarder");
            ObjectAdapter adapterSame = Communicator.CreateObjectAdapter("TestAdapterSame");
            ObjectAdapter adapterOther = Communicator.CreateObjectAdapter("TestAdapterOther");

            ITestIntfPrx samePrx = adapterSame.Add("TestSame", new TestI(), ITestIntfPrx.Factory);
            ITestIntfPrx otherPrx = adapterOther.Add("TestOther", new TestI(), ITestIntfPrx.Factory);

            adapterForwarder.Add("ForwardSame", new Forwarder(samePrx));
            adapterForwarder.Add("ForwardOther", new Forwarder(otherPrx));

            await adapterForwarder.ActivateAsync();
            await adapterSame.ActivateAsync();
            await adapterOther.ActivateAsync();

            ServerReady();
            await Communicator.ShutdownComplete;
        }

        public static async Task<int> Main(string[] args)
        {
            await using var communicator = CreateCommunicator(ref args);
            return await RunTestAsync<Server>(communicator, args);
        }
    }
}
