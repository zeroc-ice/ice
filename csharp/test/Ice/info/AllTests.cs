//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice.info
{
    public class AllTests : global::Test.AllTests
    {
        private static TcpEndpoint getTCPEndpoint(Endpoint endpoint)
        {
            for (; endpoint != null; endpoint = endpoint.Underlying)
            {
                if (endpoint is TcpEndpoint)
                {
                    return endpoint as TcpEndpoint;
                }
            }
            return null;
        }

        private static TCPConnectionInfo getTCPConnectionInfo(ConnectionInfo info)
        {
            for (; info != null; info = info.Underlying)
            {
                if (info is TCPConnectionInfo)
                {
                    return info as TCPConnectionInfo;
                }
            }
            return null;
        }

        public static void allTests(global::Test.TestHelper helper)
        {
            Communicator communicator = helper.communicator();
            var output = helper.getWriter();
            output.Write("testing proxy endpoint information... ");
            output.Flush();
            {
                var p1 = IObjectPrx.Parse(
                                "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                                "opaque -e 1.8 -t 100 -v ABCD", communicator);

                Endpoint[] endps = p1.Endpoints;

                Endpoint endpoint = endps[0];
                TcpEndpoint tcpEndpoint = getTCPEndpoint(endpoint);
                test(tcpEndpoint.Host.Equals("tcphost"));
                test(tcpEndpoint.Port == 10000);
                test(tcpEndpoint.SourceAddress.ToString().Equals("10.10.10.10"));
                test(tcpEndpoint.Timeout == 1200);
                test(tcpEndpoint.HasCompressionFlag);
                test(!tcpEndpoint.IsDatagram);

                test(tcpEndpoint.Type == EndpointType.TCP && !tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.SSL && tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.WS && !tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.WSS && tcpEndpoint.IsSecure);
                test(tcpEndpoint.Type == EndpointType.TCP && endpoint is TcpEndpoint ||
                        tcpEndpoint.Type == EndpointType.SSL && endpoint is IceSSL.Endpoint ||
                        tcpEndpoint.Type == EndpointType.WS && endpoint is WSEndpoint ||
                        tcpEndpoint.Type == EndpointType.WSS && endpoint is WSEndpoint);

                UdpEndpoint udpEndpoint = (UdpEndpoint)endps[1];
                test(udpEndpoint.Host.Equals("udphost"));
                test(udpEndpoint.Port == 10001);
                test(udpEndpoint.McastInterface.Equals("eth0"));
                test(udpEndpoint.McastTtl == 5);
                test(udpEndpoint.SourceAddress.ToString().Equals("10.10.10.10"));
                test(udpEndpoint.Timeout == -1);
                test(!udpEndpoint.HasCompressionFlag);
                test(!udpEndpoint.IsSecure);
                test(udpEndpoint.IsDatagram);
                test(udpEndpoint.Type == EndpointType.UDP);

                OpaqueEndpoint opaqueEndpoint = (OpaqueEndpoint)endps[2];
                test(opaqueEndpoint.Bytes.Length > 0);
                test(opaqueEndpoint.Encoding.Equals(new Encoding(1, 8)));
            }
            output.WriteLine("ok");

            ObjectAdapter adapter;
            output.Write("test object adapter endpoint information... ");
            output.Flush();
            {
                string host = communicator.GetPropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1";
                communicator.SetProperty("TestAdapter.Endpoints", "tcp -h \"" + host +
                    "\" -t 15000:udp -h \"" + host + "\"");
                adapter = communicator.CreateObjectAdapter("TestAdapter");

                Endpoint[] endpoints = adapter.GetEndpoints();
                test(endpoints.Length == 2);
                Endpoint[] publishedEndpoints = adapter.GetPublishedEndpoints();
                test(global::Test.Collections.Equals(endpoints, publishedEndpoints));

                TcpEndpoint tcpEndpoint = getTCPEndpoint(endpoints[0]);
                test(tcpEndpoint.Type == EndpointType.TCP ||
                        tcpEndpoint.Type == EndpointType.SSL ||
                        tcpEndpoint.Type == EndpointType.WS ||
                        tcpEndpoint.Type == EndpointType.WSS);

                test(tcpEndpoint.Host.Equals(host));
                test(tcpEndpoint.Port > 0);
                test(tcpEndpoint.Timeout == 15000);

                UdpEndpoint udpEndpoint = (UdpEndpoint)endpoints[1];
                test(udpEndpoint.Host.Equals(host));
                test(udpEndpoint.IsDatagram);
                test(udpEndpoint.Port > 0);

                endpoints = new Endpoint[] { endpoints[0] };
                test(endpoints.Length == 1);
                adapter.SetPublishedEndpoints(endpoints);
                publishedEndpoints = adapter.GetPublishedEndpoints();
                test(global::Test.Collections.Equals(endpoints, publishedEndpoints));

                adapter.Destroy();

                int port = helper.getTestPort(1);
                communicator.SetProperty("TestAdapter.Endpoints", $"default -h * -p {port}");
                communicator.SetProperty("TestAdapter.PublishedEndpoints", helper.getTestEndpoint(1));
                adapter = communicator.CreateObjectAdapter("TestAdapter");

                endpoints = adapter.GetEndpoints();
                test(endpoints.Length >= 1);
                publishedEndpoints = adapter.GetPublishedEndpoints();
                test(publishedEndpoints.Length == 1);

                foreach (var endpoint in endpoints)
                {
                    tcpEndpoint = getTCPEndpoint(endpoint);
                    test(tcpEndpoint.Port == port);
                }

                tcpEndpoint = getTCPEndpoint(publishedEndpoints[0]);
                test(tcpEndpoint.Host == "127.0.0.1");
                test(tcpEndpoint.Port == port);

                adapter.Destroy();
            }
            output.WriteLine("ok");

            int endpointPort = helper.getTestPort(0);

            var @base = IObjectPrx.Parse("test:" +
                                            helper.getTestEndpoint(0) + ":" +
                                            helper.getTestEndpoint(0, "udp"), communicator);
            var testIntf = Test.ITestIntfPrx.CheckedCast(@base);

            string defaultHost = communicator.GetProperty("Ice.Default.Host") ?? "";

            output.Write("test connection endpoint information... ");
            output.Flush();
            {
                Endpoint endpoint = @base.GetConnection().Endpoint;
                TcpEndpoint tcpEndpoint = getTCPEndpoint(endpoint);
                test(tcpEndpoint.Port == endpointPort);
                test(!tcpEndpoint.HasCompressionFlag);
                test(tcpEndpoint.Host.Equals(defaultHost));

                Dictionary<string, string> ctx = testIntf.getEndpointInfoAsContext();
                test(ctx["host"].Equals(tcpEndpoint.Host));
                test(ctx["compress"].Equals("false"));
                int port = int.Parse(ctx["port"]);
                test(port > 0);

                endpoint = @base.Clone(invocationMode: InvocationMode.Datagram).GetConnection().Endpoint;
                UdpEndpoint udp = (UdpEndpoint)endpoint;
                test(udp.Port == endpointPort);
                test(udp.Host.Equals(defaultHost));
            }
            output.WriteLine("ok");

            output.Write("testing connection information... ");
            output.Flush();
            {
                Connection connection = @base.GetConnection();
                connection.SetBufferSize(1024, 2048);

                ConnectionInfo info = connection.GetConnectionInfo();
                TCPConnectionInfo ipInfo = getTCPConnectionInfo(info);
                test(!info.Incoming);
                test(info.AdapterName.Length == 0);
                test(ipInfo.RemotePort == endpointPort);
                test(ipInfo.LocalPort > 0);
                if (defaultHost.Equals("127.0.0.1"))
                {
                    test(ipInfo.LocalAddress.Equals(defaultHost));
                    test(ipInfo.RemoteAddress.Equals(defaultHost));
                }
                test(ipInfo.RcvSize >= 1024);
                test(ipInfo.SndSize >= 2048);

                Dictionary<string, string> ctx = testIntf.getConnectionInfoAsContext();
                test(ctx["incoming"].Equals("true"));
                test(ctx["adapterName"].Equals("TestAdapter"));
                test(ctx["remoteAddress"].Equals(ipInfo.LocalAddress));
                test(ctx["localAddress"].Equals(ipInfo.RemoteAddress));
                test(ctx["remotePort"].Equals(ipInfo.LocalPort.ToString()));
                test(ctx["localPort"].Equals(ipInfo.RemotePort.ToString()));

                if (@base.GetConnection().Type().Equals("ws") || @base.GetConnection().Type().Equals("wss"))
                {
                    Dictionary<string, string> headers = ((WSConnectionInfo)info).Headers;
                    test(headers["Upgrade"].Equals("websocket"));
                    test(headers["Connection"].Equals("Upgrade"));
                    test(headers["Sec-WebSocket-Protocol"].Equals("ice.zeroc.com"));
                    test(headers["Sec-WebSocket-Accept"] != null);

                    test(ctx["ws.Upgrade"].Equals("websocket"));
                    test(ctx["ws.Connection"].Equals("Upgrade"));
                    test(ctx["ws.Sec-WebSocket-Protocol"].Equals("ice.zeroc.com"));
                    test(ctx["ws.Sec-WebSocket-Version"].Equals("13"));
                    test(ctx["ws.Sec-WebSocket-Key"] != null);
                }

                connection = @base.Clone(invocationMode: InvocationMode.Datagram).GetConnection();
                connection.SetBufferSize(2048, 1024);

                UDPConnectionInfo udpInfo = (UDPConnectionInfo)connection.GetConnectionInfo();
                test(!udpInfo.Incoming);
                test(udpInfo.AdapterName.Length == 0);
                test(udpInfo.LocalPort > 0);
                test(udpInfo.RemotePort == endpointPort);

                if (defaultHost.Equals("127.0.0.1"))
                {
                    test(udpInfo.RemoteAddress.Equals(defaultHost));
                    test(udpInfo.LocalAddress.Equals(defaultHost));
                }
                test(udpInfo.RcvSize >= 2048);
                test(udpInfo.SndSize >= 1024);
            }
            output.WriteLine("ok");

            testIntf.shutdown();

            communicator.Shutdown();
            communicator.WaitForShutdown();
        }
    }
}
