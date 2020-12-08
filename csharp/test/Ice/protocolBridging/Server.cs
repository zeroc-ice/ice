// Copyright (c) ZeroC, Inc. All rights reserved.

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
            await communicator.ActivateAsync();
            communicator.SetProperty("TestAdapterForwarder.Endpoints", GetTestEndpoint(0));

            var ice1Endpoint = TestHelper.GetTestEndpoint(
                new Dictionary<string, string>
                {
                    ["Test.Host"] = communicator.GetProperty("Test.Host")!,
                    ["Test.Protocol"] = "ice1",
                    ["Test.Transport"] = communicator.GetProperty("Test.Transport")!,
                },
                ephemeral: true);

            var ice2Endpoint = TestHelper.GetTestEndpoint(
                new Dictionary<string, string>
                {
                    ["Test.Host"] = communicator.GetProperty("Test.Host")!,
                    ["Test.Protocol"] = "ice2",
                    ["Test.Transport"] = communicator.GetProperty("Test.Transport")!,
                },
                ephemeral: true);

            if (Protocol == Protocol.Ice1)
            {
                communicator.SetProperty("TestAdapterSame.Endpoints", ice1Endpoint);
                communicator.SetProperty("TestAdapterOther.Endpoints", ice2Endpoint);
            }
            else
            {
                communicator.SetProperty("TestAdapterSame.Endpoints", ice2Endpoint);
                communicator.SetProperty("TestAdapterOther.Endpoints", ice1Endpoint);
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
