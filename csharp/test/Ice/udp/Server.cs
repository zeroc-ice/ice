// Copyright (c) ZeroC, Inc. All rights reserved.

using System;
using System.Collections.Generic;
using System.Runtime.InteropServices;
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

            if (RuntimeInformation.IsOSPlatform(OSPlatform.OSX) &&
                properties.TryGetValue("Ice.IPv6", out string? value) &&
                int.TryParse(value, out int ipv6) && ipv6 > 0)
            {
                // Disable dual mode sockets on macOS, see https://github.com/dotnet/corefx/issues/31182
                properties["Ice.IPv4"] = "0";
            }

            await using Communicator communicator = Initialize(properties);
            int num = 0;
            try
            {
                num = args.Length == 1 ? int.Parse(args[0]) : 0;
            }
            catch (FormatException)
            {
            }

            communicator.SetProperty("ControlAdapter.Endpoints", GetTestEndpoint(num, "tcp"));
            ObjectAdapter adapter = communicator.CreateObjectAdapter("ControlAdapter");
            adapter.Add("control", new TestIntf());
            await adapter.ActivateAsync();
            ServerReady();
            if (num == 0)
            {
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(num, "udp"));
                ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter");
                adapter2.Add("test", new TestIntf());
                await adapter2.ActivateAsync();
            }

            var endpoint = new StringBuilder();

            // Use loopback to prevent other machines to answer.
            if (communicator.GetProperty("Ice.IPv6") == "1")
            {
                endpoint.Append("udp -h \"ff15::1:1\"");
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ||
                    RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                {
                    endpoint.Append(" --interface \"::1\"");
                }
            }
            else
            {
                endpoint.Append("udp -h 239.255.1.1");
                if (RuntimeInformation.IsOSPlatform(OSPlatform.Windows) ||
                    RuntimeInformation.IsOSPlatform(OSPlatform.OSX))
                {
                    endpoint.Append(" --interface 127.0.0.1");
                }
            }
            endpoint.Append(" -p ");
            endpoint.Append(GetTestBasePort(properties) + 10);
            communicator.SetProperty("McastTestAdapter.Endpoints", endpoint.ToString());
            ObjectAdapter mcastAdapter = communicator.CreateObjectAdapter("McastTestAdapter");
            mcastAdapter.Add("test", new TestIntf());
            await mcastAdapter.ActivateAsync();

            await communicator.WaitForShutdownAsync();
        }

        public static Task<int> Main(string[] args) => TestDriver.RunTestAsync<Server>(args);
    }
}
