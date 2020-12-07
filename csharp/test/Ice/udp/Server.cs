// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Text;
using System.Threading.Tasks;
using Test;

namespace ZeroC.Ice.Test.UDP
{
    public class Server : TestHelper
    {
        public override async Task RunAsync(string[] args)
        {
            Dictionary<string, string>? properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.UDP.RcvSize"] = "16K";

            await using Communicator communicator = Initialize(properties);
            await communicator.ActivateAsync();
            int num = 0;
            try
            {
                num = args.Length == 1 ? int.Parse(args[0]) : 0;
            }
            catch (FormatException)
            {
            }

            communicator.SetProperty("ControlAdapter.Endpoints", GetTestEndpoint(num, Transport));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
            adapter.Add("control", new TestIntf());
            await adapter.ActivateAsync();
            ServerReady();
            if (num == 0)
            {
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(num, "udp"));
                communicator.SetProperty("TestAdapter.AcceptNonSecure", "Always");
                ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter");
                adapter2.Add("test", new TestIntf());
                await adapter2.ActivateAsync();
            }

            var endpoint = new StringBuilder();

            // Use loopback to prevent other machines to answer.
            if (Host.Contains(":"))
            {
                endpoint.Append("udp -h \"ff15::1:1\"");
                if (OperatingSystem.IsWindows() ||
                    OperatingSystem.IsMacOS())
                {
                    endpoint.Append(" --interface \"::1\"");
                }
            }
            else
            {
                endpoint.Append("udp -h 239.255.1.1");
                if (OperatingSystem.IsWindows() ||
                    OperatingSystem.IsMacOS())
                {
                    endpoint.Append(" --interface 127.0.0.1");
                }
            }
            endpoint.Append(" -p ");
            endpoint.Append(GetTestBasePort(properties) + 10);
            communicator.SetProperty("McastTestAdapter.Endpoints", endpoint.ToString());
            communicator.SetProperty("McastTestAdapter.AcceptNonSecure", "Always");
            ObjectAdapter mcastAdapter = communicator.CreateObjectAdapter("McastTestAdapter");
            mcastAdapter.Add("test", new TestIntf());
            await mcastAdapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
