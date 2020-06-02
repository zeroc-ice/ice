//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Text;
using Test;

namespace ZeroC.Ice.Test.UDP
{
    public class Server : TestHelper
    {
        public override void Run(string[] args)
        {
            var properties = CreateTestProperties(ref args);
            properties["Ice.Warn.Connections"] = "0";
            properties["Ice.UDP.RcvSize"] = "16384";

            string? value;
            int ipv6;
            if (AssemblyUtil.IsMacOS &&
                properties.TryGetValue("Ice.IPv6", out value) &&
                int.TryParse(value, out ipv6) && ipv6 > 0)
            {
                // Disable dual mode sockets on macOS, see https://github.com/dotnet/corefx/issues/31182
                properties["Ice.IPv4"] = "0";
            }

            using var communicator = Initialize(properties);
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
            adapter.Activate();
            ServerReady();
            if (num == 0)
            {
                communicator.SetProperty("TestAdapter.Endpoints", GetTestEndpoint(num, "udp"));
                ObjectAdapter adapter2 = communicator.CreateObjectAdapter("TestAdapter");
                adapter2.Add("test", new TestIntf());
                adapter2.Activate();
            }

            StringBuilder endpoint = new StringBuilder();
            //
            // Use loopback to prevent other machines to answer.
            //
            if (communicator.GetProperty("Ice.IPv6") == "1")
            {
                endpoint.Append("udp -h \"ff15::1:1\"");
                if (AssemblyUtil.IsWindows || AssemblyUtil.IsMacOS)
                {
                    endpoint.Append(" --interface \"::1\"");
                }
            }
            else
            {
                endpoint.Append("udp -h 239.255.1.1");
                if (AssemblyUtil.IsWindows || AssemblyUtil.IsMacOS)
                {
                    endpoint.Append(" --interface 127.0.0.1");
                }
            }
            endpoint.Append(" -p ");
            endpoint.Append(GetTestPort(properties, 10));
            communicator.SetProperty("McastTestAdapter.Endpoints", endpoint.ToString());
            ObjectAdapter mcastAdapter = communicator.CreateObjectAdapter("McastTestAdapter");
            mcastAdapter.Add("test", new TestIntf());
            mcastAdapter.Activate();

            communicator.WaitForShutdown();
        }

        public static int Main(string[] args) => TestDriver.RunTest<Server>(args);
    }
}
