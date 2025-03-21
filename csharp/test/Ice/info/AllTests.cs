// Copyright (c) ZeroC, Inc.

namespace Ice.info;

public class AllTests : global::Test.AllTests
{
    private static Ice.TCPEndpointInfo getTCPEndpointInfo(Ice.EndpointInfo info)
    {
        for (; info != null; info = info.underlying)
        {
            if (info is Ice.TCPEndpointInfo)
            {
                return info as Ice.TCPEndpointInfo;
            }
        }
        return null;
    }

    private static Ice.TCPConnectionInfo getTCPConnectionInfo(Ice.ConnectionInfo info)
    {
        for (; info != null; info = info.underlying)
        {
            if (info is Ice.TCPConnectionInfo)
            {
                return info as Ice.TCPConnectionInfo;
            }
        }
        return null;
    }

    public static void allTests(global::Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        var output = helper.getWriter();
        output.Write("testing proxy endpoint information... ");
        output.Flush();
        {
            Ice.ObjectPrx p1 = communicator.stringToProxy(
                            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                            "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                            "opaque -e 1.8 -t 100 -v ABCD");

            Ice.Endpoint[] endps = p1.ice_getEndpoints();

            Ice.EndpointInfo info = endps[0].getInfo();
            Ice.TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(info);
            test(tcpEndpoint.host == "tcphost");
            test(tcpEndpoint.port == 10000);
            test(tcpEndpoint.sourceAddress == "10.10.10.10");
            test(tcpEndpoint.timeout == 1200);
            test(tcpEndpoint.compress);
            test(!tcpEndpoint.datagram());

            test((tcpEndpoint.type() == Ice.TCPEndpointType.value && !tcpEndpoint.secure()) ||
                 (tcpEndpoint.type() == Ice.SSLEndpointType.value && tcpEndpoint.secure()) ||
                 (tcpEndpoint.type() == Ice.WSEndpointType.value && !tcpEndpoint.secure()) ||
                 (tcpEndpoint.type() == Ice.WSSEndpointType.value && tcpEndpoint.secure()));
            test((tcpEndpoint.type() == Ice.TCPEndpointType.value && info is Ice.TCPEndpointInfo) ||
                 (tcpEndpoint.type() == Ice.SSLEndpointType.value && info is Ice.SSL.EndpointInfo) ||
                 (tcpEndpoint.type() == Ice.WSEndpointType.value && info is Ice.WSEndpointInfo) ||
                 (tcpEndpoint.type() == Ice.WSSEndpointType.value && info is Ice.WSEndpointInfo));

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endps[1].getInfo();
            test(udpEndpoint.host == "udphost");
            test(udpEndpoint.port == 10001);
            test(udpEndpoint.mcastInterface == "eth0");
            test(udpEndpoint.mcastTtl == 5);
            test(udpEndpoint.sourceAddress == "10.10.10.10");
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == 3);

            Ice.OpaqueEndpointInfo opaqueEndpoint = (Ice.OpaqueEndpointInfo)endps[2].getInfo();
            test(opaqueEndpoint.rawBytes.Length > 0);
            test(opaqueEndpoint.rawEncoding.Equals(new Ice.EncodingVersion(1, 8)));
        }
        output.WriteLine("ok");

        Ice.ObjectAdapter adapter;
        output.Write("test object adapter endpoint information... ");
        output.Flush();
        {
            string host = communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1";
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "tcp -h \"" + host +
                "\" -t 15000:udp -h \"" + host + "\"");
            adapter = communicator.createObjectAdapter("TestAdapter");

            Ice.Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.Length == 2);
            Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(Enumerable.SequenceEqual(endpoints, publishedEndpoints));

            Ice.TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo());
            test(tcpEndpoint.type() == Ice.TCPEndpointType.value ||
                 tcpEndpoint.type() == Ice.SSLEndpointType.value ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value ||
                 tcpEndpoint.type() == Ice.WSSEndpointType.value);

            test(tcpEndpoint.host.Equals(host));
            test(tcpEndpoint.port > 0);
            test(tcpEndpoint.timeout == 15000);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endpoints[1].getInfo();
            test(udpEndpoint.host.Equals(host));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            endpoints = new Ice.Endpoint[] { endpoints[0] };
            test(endpoints.Length == 1);
            adapter.setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(Enumerable.SequenceEqual(endpoints, publishedEndpoints));

            adapter.destroy();

            int port = helper.getTestPort(1);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p " + port);
            communicator.getProperties().setProperty("TestAdapter.PublishedHost", "test.zeroc.com");
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.Length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.Length == 1);

            foreach (Ice.Endpoint endpoint in endpoints)
            {
                tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo());
                test(tcpEndpoint.port == port);
            }

            tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo());
            test(tcpEndpoint.host == "test.zeroc.com");
            test(tcpEndpoint.port == port);

            adapter.destroy();
        }
        output.WriteLine("ok");

        int endpointPort = helper.getTestPort(0);

        Ice.ObjectPrx @base = communicator.stringToProxy("test:" +
                                                         helper.getTestEndpoint(0) + ":" +
                                                         helper.getTestEndpoint(0, "udp"));
        var testIntf = Test.TestIntfPrxHelper.checkedCast(@base);

        string defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host");

        output.Write("test connection endpoint information... ");
        output.Flush();
        {
            Ice.EndpointInfo info = @base.ice_getConnection().getEndpoint().getInfo();
            Ice.TCPEndpointInfo tcpinfo = getTCPEndpointInfo(info);
            test(tcpinfo.port == endpointPort);
            test(!tcpinfo.compress);
            test(tcpinfo.host.Equals(defaultHost));

            Dictionary<string, string> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx["host"].Equals(tcpinfo.host));
            test(ctx["compress"] == "false");
            int port = int.Parse(ctx["port"]);
            test(port > 0);

            info = @base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)info;
            test(udp.port == endpointPort);
            test(udp.host.Equals(defaultHost));
        }
        output.WriteLine("ok");

        output.Write("testing connection information... ");
        output.Flush();
        {
            Ice.Connection connection = @base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            Ice.ConnectionInfo info = connection.getInfo();
            Ice.TCPConnectionInfo ipInfo = getTCPConnectionInfo(info);
            test(!info.incoming);
            test(info.adapterName.Length == 0);
            test(ipInfo.remotePort == endpointPort);
            test(ipInfo.localPort > 0);
            if (defaultHost == "127.0.0.1")
            {
                test(ipInfo.localAddress.Equals(defaultHost));
                test(ipInfo.remoteAddress.Equals(defaultHost));
            }
            test(ipInfo.rcvSize >= 1024);
            test(ipInfo.sndSize >= 2048);

            Dictionary<string, string> ctx = testIntf.getConnectionInfoAsContext();
            test(ctx["incoming"] == "true");
            test(ctx["adapterName"] == "TestAdapter");
            test(ctx["remoteAddress"].Equals(ipInfo.localAddress));
            test(ctx["localAddress"].Equals(ipInfo.remoteAddress));
            test(ctx["remotePort"].Equals(ipInfo.localPort.ToString()));
            test(ctx["localPort"].Equals(ipInfo.remotePort.ToString()));

            if (@base.ice_getConnection().type() == "ws" || @base.ice_getConnection().type() == "wss")
            {
                Dictionary<string, string> headers = ((Ice.WSConnectionInfo)info).headers;
                test(headers["Upgrade"] == "websocket");
                test(headers["Connection"] == "Upgrade");
                test(headers["Sec-WebSocket-Protocol"] == "ice.zeroc.com");
                test(headers["Sec-WebSocket-Accept"] != null);

                test(ctx["ws.Upgrade"] == "websocket");
                test(ctx["ws.Connection"] == "Upgrade");
                test(ctx["ws.Sec-WebSocket-Protocol"] == "ice.zeroc.com");
                test(ctx["ws.Sec-WebSocket-Version"] == "13");
                test(ctx["ws.Sec-WebSocket-Key"] != null);

                var sslInfo = info.underlying as Ice.SSL.ConnectionInfo;
                test((@base.ice_getConnection().type() == "ws" && sslInfo == null) ||
                     (@base.ice_getConnection().type() == "wss" && sslInfo != null));
                if (sslInfo != null)
                {
                    test(sslInfo.certs.Length > 0);
                    // The SHA1 Thumbprint of the server certificate used in the test.
                    test(sslInfo.certs[0].Thumbprint == "9E754B7A7BF5E1951CB2A46B565F8BBB8A4A355D");
                }
            }
            else if (@base.ice_getConnection().type() == "ssl")
            {
                var sslInfo = info as Ice.SSL.ConnectionInfo;
                test(sslInfo != null);
                test(sslInfo.certs.Length > 0);
                // The SHA1 Thumbprint of the server certificate used in the test.
                test(sslInfo.certs[0].Thumbprint == "9E754B7A7BF5E1951CB2A46B565F8BBB8A4A355D");
            }

            connection = @base.ice_datagram().ice_getConnection();
            connection.setBufferSize(2048, 1024);

            Ice.UDPConnectionInfo udpInfo = (Ice.UDPConnectionInfo)connection.getInfo();
            test(!udpInfo.incoming);
            test(udpInfo.adapterName.Length == 0);
            test(udpInfo.localPort > 0);
            test(udpInfo.remotePort == endpointPort);

            if (defaultHost == "127.0.0.1")
            {
                test(udpInfo.remoteAddress.Equals(defaultHost));
                test(udpInfo.localAddress.Equals(defaultHost));
            }
            test(udpInfo.rcvSize >= 2048);
            test(udpInfo.sndSize >= 1024);
        }
        output.WriteLine("ok");

        testIntf.shutdown();

        communicator.shutdown();
        communicator.waitForShutdown();
    }
}
