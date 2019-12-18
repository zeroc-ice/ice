//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Text;

using Ice.udp.Test;

namespace Ice
{
    namespace udp
    {
        public class Server : global::Test.TestHelper
        {
            public override void run(string[] args)
            {
                var properties = createTestProperties(ref args);
                properties["Ice.Warn.Connections"] = "0";
                properties["Ice.UDP.RcvSize"] = "16384";

                string? value;
                int ipv6;
                if (IceInternal.AssemblyUtil.isMacOS &&
                    properties.TryGetValue("Ice.IPv6", out value) &&
                    int.TryParse(value, out ipv6) && ipv6 > 0)
                {
                    // Disable dual mode sockets on macOS, see https://github.com/dotnet/corefx/issues/31182
                    properties["Ice.IPv4"] = "0";
                }

                using (var communicator = initialize(properties))
                {
                    int num = 0;
                    try
                    {
                        num = args.Length == 1 ? int.Parse(args[0]) : 0;
                    }
                    catch (FormatException)
                    {
                    }

                    communicator.SetProperty("ControlAdapter.Endpoints", getTestEndpoint(num, "tcp"));
                    Ice.ObjectAdapter adapter = communicator.createObjectAdapter("ControlAdapter");
                    adapter.Add(new TestIntfI(), "control");
                    adapter.Activate();
                    serverReady();
                    if (num == 0)
                    {
                        communicator.SetProperty("TestAdapter.Endpoints", getTestEndpoint(num, "udp"));
                        Ice.ObjectAdapter adapter2 = communicator.createObjectAdapter("TestAdapter");
                        adapter2.Add(new TestIntfI(), "test");
                        adapter2.Activate();
                    }

                    StringBuilder endpoint = new StringBuilder();
                    //
                    // Use loopback to prevent other machines to answer.
                    //
                    if (communicator.GetProperty("Ice.IPv6") == "1")
                    {
                        endpoint.Append("udp -h \"ff15::1:1\"");
                        if (IceInternal.AssemblyUtil.isWindows || IceInternal.AssemblyUtil.isMacOS)
                        {
                            endpoint.Append(" --interface \"::1\"");
                        }
                    }
                    else
                    {
                        endpoint.Append("udp -h 239.255.1.1");
                        if (IceInternal.AssemblyUtil.isWindows || IceInternal.AssemblyUtil.isMacOS)
                        {
                            endpoint.Append(" --interface 127.0.0.1");
                        }
                    }
                    endpoint.Append(" -p ");
                    endpoint.Append(getTestPort(properties, 10));
                    communicator.SetProperty("McastTestAdapter.Endpoints", endpoint.ToString());
                    Ice.ObjectAdapter mcastAdapter = communicator.createObjectAdapter("McastTestAdapter");
                    mcastAdapter.Add(new TestIntfI(), "test");
                    mcastAdapter.Activate();

                    communicator.waitForShutdown();
                }
            }

            public static int Main(string[] args)
            {
                return global::Test.TestDriver.runTest<Server>(args);
            }
        }
    }
}
