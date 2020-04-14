//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace Ice.info
{
    public class AllTests
    {
        private static TcpEndpoint? getTCPEndpoint(Endpoint endpoint)
        {
            Endpoint? underlying = endpoint;
            for (; underlying != null; underlying = underlying.Underlying)
            {
                if (underlying is TcpEndpoint)
                {
                    return (TcpEndpoint)underlying;
                }
            }
            return null;
        }

        private static TCPConnectionInfo? getTCPConnectionInfo(ConnectionInfo info)
        {
            ConnectionInfo? underlying = info;
            for (; underlying != null; underlying = underlying.Underlying)
            {
                if (info is TCPConnectionInfo)
                {
                    return (TCPConnectionInfo)underlying;
                }
            }
            return null;
        }

        public static void allTests(global::Test.TestHelper helper)
        {
            Communicator? communicator = helper.Communicator();
            TestHelper.Assert(communicator != null);
            var output = helper.GetWriter();
            output.Write("testing proxy endpoint information... ");
            output.Flush();
            {
                var p1 = IObjectPrx.Parse(
                                "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                                "opaque -e 1.8 -t 100 -v ABCD", communicator);

                var endps = p1.Endpoints;

                Endpoint endpoint = endps[0];
                TcpEndpoint? tcpEndpoint = getTCPEndpoint(endpoint);
                TestHelper.Assert(tcpEndpoint != null);
                TestHelper.Assert(tcpEndpoint.Host.Equals("tcphost"));
                TestHelper.Assert(tcpEndpoint.Port == 10000);
                TestHelper.Assert(tcpEndpoint.SourceAddress!.ToString().Equals("10.10.10.10"));
                TestHelper.Assert(tcpEndpoint.Timeout == 1200);
                TestHelper.Assert(tcpEndpoint.HasCompressionFlag);
                TestHelper.Assert(!tcpEndpoint.IsDatagram);

                TestHelper.Assert(tcpEndpoint.Type == EndpointType.TCP && !tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.SSL && tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.WS && !tcpEndpoint.IsSecure ||
                        tcpEndpoint.Type == EndpointType.WSS && tcpEndpoint.IsSecure);
                TestHelper.Assert(tcpEndpoint.Type == EndpointType.TCP && endpoint is TcpEndpoint ||
                        tcpEndpoint.Type == EndpointType.SSL && endpoint is IceSSL.Endpoint ||
                        tcpEndpoint.Type == EndpointType.WS && endpoint is WSEndpoint ||
                        tcpEndpoint.Type == EndpointType.WSS && endpoint is WSEndpoint);

                UdpEndpoint udpEndpoint = (UdpEndpoint)endps[1];
                TestHelper.Assert(udpEndpoint.Host.Equals("udphost"));
                TestHelper.Assert(udpEndpoint.Port == 10001);
                TestHelper.Assert(udpEndpoint.McastInterface.Equals("eth0"));
                TestHelper.Assert(udpEndpoint.McastTtl == 5);
                TestHelper.Assert(udpEndpoint.SourceAddress!.ToString().Equals("10.10.10.10"));
                TestHelper.Assert(udpEndpoint.Timeout == -1);
                TestHelper.Assert(!udpEndpoint.HasCompressionFlag);
                TestHelper.Assert(!udpEndpoint.IsSecure);
                TestHelper.Assert(udpEndpoint.IsDatagram);
                TestHelper.Assert(udpEndpoint.Type == EndpointType.UDP);

                OpaqueEndpoint opaqueEndpoint = (OpaqueEndpoint)endps[2];
                TestHelper.Assert(opaqueEndpoint.Bytes.Length > 0);
                TestHelper.Assert(opaqueEndpoint.Encoding.Equals(new Encoding(1, 8)));
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

                var endpoints = adapter.GetEndpoints();
                TestHelper.Assert(endpoints.Count == 2);
                var publishedEndpoints = adapter.GetPublishedEndpoints();
                TestHelper.Assert(global::Test.Collections.Equals(endpoints, publishedEndpoints));

                TcpEndpoint? tcpEndpoint = getTCPEndpoint(endpoints[0]);
                TestHelper.Assert(tcpEndpoint != null);
                TestHelper.Assert(
                        tcpEndpoint.Type == EndpointType.TCP ||
                        tcpEndpoint.Type == EndpointType.SSL ||
                        tcpEndpoint.Type == EndpointType.WS ||
                        tcpEndpoint.Type == EndpointType.WSS);

                TestHelper.Assert(tcpEndpoint.Host.Equals(host));
                TestHelper.Assert(tcpEndpoint.Port > 0);
                TestHelper.Assert(tcpEndpoint.Timeout == 15000);

                UdpEndpoint udpEndpoint = (UdpEndpoint)endpoints[1];
                TestHelper.Assert(udpEndpoint.Host.Equals(host));
                TestHelper.Assert(udpEndpoint.IsDatagram);
                TestHelper.Assert(udpEndpoint.Port > 0);

                endpoints = new List<Endpoint> { endpoints[0] };
                TestHelper.Assert(endpoints.Count == 1);

                adapter.SetPublishedEndpoints(endpoints);
                publishedEndpoints = adapter.GetPublishedEndpoints();
                TestHelper.Assert(Collections.Equals(endpoints, publishedEndpoints));

                adapter.Destroy();

                int port = helper.GetTestPort(1);
                communicator.SetProperty("TestAdapter.Endpoints", $"default -h * -p {port}");
                communicator.SetProperty("TestAdapter.PublishedEndpoints", helper.GetTestEndpoint(1));
                adapter = communicator.CreateObjectAdapter("TestAdapter");

                endpoints = adapter.GetEndpoints();
                TestHelper.Assert(endpoints.Count >= 1);
                publishedEndpoints = adapter.GetPublishedEndpoints();
                TestHelper.Assert(publishedEndpoints.Count == 1);

                foreach (var endpoint in endpoints)
                {
                    tcpEndpoint = getTCPEndpoint(endpoint);
                    TestHelper.Assert(tcpEndpoint!.Port == port);
                }

                tcpEndpoint = getTCPEndpoint(publishedEndpoints[0]);
                TestHelper.Assert(tcpEndpoint!.Host == "127.0.0.1");
                TestHelper.Assert(tcpEndpoint!.Port == port);

                adapter.Destroy();
            }
            output.WriteLine("ok");

            int endpointPort = helper.GetTestPort(0);

            var testIntf = Test.ITestIntfPrx.Parse("test:" +
                                            helper.GetTestEndpoint(0) + ":" +
                                            helper.GetTestEndpoint(0, "udp"), communicator);

            string defaultHost = communicator.GetProperty("Ice.Default.Host") ?? "";

            output.Write("test connection endpoint information... ");
            output.Flush();
            {
                Endpoint endpoint = testIntf.GetConnection().Endpoint;
                TcpEndpoint? tcpEndpoint = getTCPEndpoint(endpoint);
                TestHelper.Assert(tcpEndpoint != null);
                TestHelper.Assert(tcpEndpoint.Port == endpointPort);
                TestHelper.Assert(!tcpEndpoint.HasCompressionFlag);
                TestHelper.Assert(tcpEndpoint.Host.Equals(defaultHost));

                Dictionary<string, string> ctx = testIntf.getEndpointInfoAsContext();
                TestHelper.Assert(ctx["host"].Equals(tcpEndpoint.Host));
                TestHelper.Assert(ctx["compress"].Equals("false"));
                int port = int.Parse(ctx["port"]);
                TestHelper.Assert(port > 0);

                endpoint = testIntf.Clone(invocationMode: InvocationMode.Datagram).GetConnection().Endpoint;
                UdpEndpoint udp = (UdpEndpoint)endpoint;
                TestHelper.Assert(udp.Port == endpointPort);
                TestHelper.Assert(udp.Host.Equals(defaultHost));
            }
            output.WriteLine("ok");

            output.Write("testing connection information... ");
            output.Flush();
            {
                Connection connection = testIntf.GetConnection();
                connection.SetBufferSize(1024, 2048);

                ConnectionInfo info = connection.GetConnectionInfo();
                TCPConnectionInfo ipInfo = getTCPConnectionInfo(info)!;
                TestHelper.Assert(!info.Incoming);
                TestHelper.Assert(info.AdapterName!.Length == 0);
                TestHelper.Assert(ipInfo.RemotePort == endpointPort);
                TestHelper.Assert(ipInfo.LocalPort > 0);
                if (defaultHost.Equals("127.0.0.1"))
                {
                    TestHelper.Assert(ipInfo.LocalAddress.Equals(defaultHost));
                    TestHelper.Assert(ipInfo.RemoteAddress.Equals(defaultHost));
                }
                TestHelper.Assert(ipInfo.RcvSize >= 1024);
                TestHelper.Assert(ipInfo.SndSize >= 2048);

                Dictionary<string, string> ctx = testIntf.getConnectionInfoAsContext();
                TestHelper.Assert(ctx["incoming"].Equals("true"));
                TestHelper.Assert(ctx["adapterName"].Equals("TestAdapter"));
                TestHelper.Assert(ctx["remoteAddress"].Equals(ipInfo.LocalAddress));
                TestHelper.Assert(ctx["localAddress"].Equals(ipInfo.RemoteAddress));
                TestHelper.Assert(ctx["remotePort"].Equals(ipInfo.LocalPort.ToString()));
                TestHelper.Assert(ctx["localPort"].Equals(ipInfo.RemotePort.ToString()));

                if (testIntf.GetConnection().Type().Equals("ws") || testIntf.GetConnection().Type().Equals("wss"))
                {
                    Dictionary<string, string> headers = ((WSConnectionInfo)info).Headers!;
                    TestHelper.Assert(headers["Upgrade"].Equals("websocket"));
                    TestHelper.Assert(headers["Connection"].Equals("Upgrade"));
                    TestHelper.Assert(headers["Sec-WebSocket-Protocol"].Equals("ice.zeroc.com"));
                    TestHelper.Assert(headers["Sec-WebSocket-Accept"] != null);

                    TestHelper.Assert(ctx["ws.Upgrade"].Equals("websocket"));
                    TestHelper.Assert(ctx["ws.Connection"].Equals("Upgrade"));
                    TestHelper.Assert(ctx["ws.Sec-WebSocket-Protocol"].Equals("ice.zeroc.com"));
                    TestHelper.Assert(ctx["ws.Sec-WebSocket-Version"].Equals("13"));
                    TestHelper.Assert(ctx["ws.Sec-WebSocket-Key"] != null);
                }

                connection = testIntf.Clone(invocationMode: InvocationMode.Datagram).GetConnection();
                connection.SetBufferSize(2048, 1024);

                UDPConnectionInfo udpInfo = (UDPConnectionInfo)connection.GetConnectionInfo();
                TestHelper.Assert(!udpInfo.Incoming);
                TestHelper.Assert(udpInfo.AdapterName!.Length == 0);
                TestHelper.Assert(udpInfo.LocalPort > 0);
                TestHelper.Assert(udpInfo.RemotePort == endpointPort);

                if (defaultHost.Equals("127.0.0.1"))
                {
                    TestHelper.Assert(udpInfo.RemoteAddress.Equals(defaultHost));
                    TestHelper.Assert(udpInfo.LocalAddress.Equals(defaultHost));
                }
                TestHelper.Assert(udpInfo.RcvSize >= 2048);
                TestHelper.Assert(udpInfo.SndSize >= 1024);
            }
            output.WriteLine("ok");

            testIntf.shutdown();

            communicator.Shutdown();
            communicator.WaitForShutdown();
        }
    }
}
