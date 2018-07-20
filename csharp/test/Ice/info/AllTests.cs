// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

using System.Collections.Generic;
using Test;

public class AllTests : Test.AllTests
{
    private static Ice.TCPEndpointInfo getTCPEndpointInfo(Ice.EndpointInfo info)
    {
        for(; info != null; info = info.underlying)
        {
            if(info is Ice.TCPEndpointInfo)
            {
                return info as Ice.TCPEndpointInfo;
            }
        }
        return null;
    }

    private static Ice.TCPConnectionInfo getTCPConnectionInfo(Ice.ConnectionInfo info)
    {
        for(; info != null; info = info.underlying)
        {
            if(info is Ice.TCPConnectionInfo)
            {
                return info as Ice.TCPConnectionInfo;
            }
        }
        return null;
    }

    public static void allTests(Test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        Write("testing proxy endpoint information... ");
        Flush();
        {
            Ice.ObjectPrx p1 = communicator.stringToProxy(
                            "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                            "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                            "opaque -e 1.8 -t 100 -v ABCD");

            Ice.Endpoint[] endps = p1.ice_getEndpoints();

            Ice.EndpointInfo info = endps[0].getInfo();
            Ice.TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(info);
            test(tcpEndpoint.host.Equals("tcphost"));
            test(tcpEndpoint.port == 10000);
            test(tcpEndpoint.sourceAddress.Equals("10.10.10.10"));
            test(tcpEndpoint.timeout == 1200);
            test(tcpEndpoint.compress);
            test(!tcpEndpoint.datagram());

            test(tcpEndpoint.type() == Ice.TCPEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.SSLEndpointType.value && tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSSEndpointType.value && tcpEndpoint.secure());
            test(tcpEndpoint.type() == Ice.TCPEndpointType.value && info is Ice.TCPEndpointInfo ||
                 tcpEndpoint.type() == Ice.SSLEndpointType.value && info is IceSSL.EndpointInfo ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value && info is Ice.WSEndpointInfo ||
                 tcpEndpoint.type() == Ice.WSSEndpointType.value && info is Ice.WSEndpointInfo);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endps[1].getInfo();
            test(udpEndpoint.host.Equals("udphost"));
            test(udpEndpoint.port == 10001);
            test(udpEndpoint.mcastInterface.Equals("eth0"));
            test(udpEndpoint.mcastTtl == 5);
            test(udpEndpoint.sourceAddress.Equals("10.10.10.10"));
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == 3);

            Ice.OpaqueEndpointInfo opaqueEndpoint = (Ice.OpaqueEndpointInfo)endps[2].getInfo();
            test(opaqueEndpoint.rawBytes.Length > 0);
            test(opaqueEndpoint.rawEncoding.Equals(new Ice.EncodingVersion(1, 8)));
        }
        WriteLine("ok");

        Ice.ObjectAdapter adapter;
        Write("test object adapter endpoint information... ");
        Flush();
        {
            string host = communicator.getProperties().getPropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1";
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "tcp -h \"" + host +
                "\" -t 15000:udp -h \"" + host + "\"");
            adapter = communicator.createObjectAdapter("TestAdapter");

            Ice.Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.Length == 2);
            Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(IceUtilInternal.Arrays.Equals(endpoints, publishedEndpoints));

            Ice.TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo());
            test(tcpEndpoint.type() == Ice.TCPEndpointType.value || tcpEndpoint.type() == Ice.SSLEndpointType.value ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value || tcpEndpoint.type() == Ice.WSSEndpointType.value);

            test(tcpEndpoint.host.Equals(host));
            test(tcpEndpoint.port > 0);
            test(tcpEndpoint.timeout == 15000);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endpoints[1].getInfo();
            test(udpEndpoint.host.Equals(host));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            endpoints = new Ice.Endpoint[]{endpoints[0]};
            test(endpoints.Length == 1);
            adapter.setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(IceUtilInternal.Arrays.Equals(endpoints, publishedEndpoints));

            adapter.destroy();

            int port = helper.getTestPort(1);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p " + port);
            communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", helper.getTestEndpoint(1));
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.Length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.Length == 1);

            foreach(Ice.Endpoint endpoint in endpoints)
            {
                tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo());
                test(tcpEndpoint.port == port);
            }

            tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo());
            test(tcpEndpoint.host.Equals("127.0.0.1"));
            test(tcpEndpoint.port == port);

            adapter.destroy();
        }
        WriteLine("ok");

        int endpointPort = helper.getTestPort(0);

        Ice.ObjectPrx @base = communicator.stringToProxy("test:" +
                                                         helper.getTestEndpoint(0) + ":" +
                                                         helper.getTestEndpoint(0, "udp"));
        TestIntfPrx testIntf = TestIntfPrxHelper.checkedCast(@base);

        string defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");

        Write("test connection endpoint information... ");
        Flush();
        {
            Ice.EndpointInfo info = @base.ice_getConnection().getEndpoint().getInfo();
            Ice.TCPEndpointInfo tcpinfo = getTCPEndpointInfo(info);
            test(tcpinfo.port == endpointPort);
            test(!tcpinfo.compress);
            test(tcpinfo.host.Equals(defaultHost));

            Dictionary<string, string> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx["host"].Equals(tcpinfo.host));
            test(ctx["compress"].Equals("false"));
            int port = System.Int32.Parse(ctx["port"]);
            test(port > 0);

            info = @base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)info;
            test(udp.port == endpointPort);
            test(udp.host.Equals(defaultHost));
        }
        WriteLine("ok");

        Write("testing connection information... ");
        Flush();
        {
            Ice.Connection connection = @base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            Ice.ConnectionInfo info = connection.getInfo();
            Ice.TCPConnectionInfo ipInfo = getTCPConnectionInfo(info);
            test(!info.incoming);
            test(info.adapterName.Length == 0);
            test(ipInfo.remotePort == endpointPort);
            test(ipInfo.localPort > 0);
            if(defaultHost.Equals("127.0.0.1"))
            {
                test(ipInfo.localAddress.Equals(defaultHost));
                test(ipInfo.remoteAddress.Equals(defaultHost));
            }
            test(ipInfo.rcvSize >= 1024);
            test(ipInfo.sndSize >= 2048);

            Dictionary<string, string> ctx = testIntf.getConnectionInfoAsContext();
            test(ctx["incoming"].Equals("true"));
            test(ctx["adapterName"].Equals("TestAdapter"));
            test(ctx["remoteAddress"].Equals(ipInfo.localAddress));
            test(ctx["localAddress"].Equals(ipInfo.remoteAddress));
            test(ctx["remotePort"].Equals(ipInfo.localPort.ToString()));
            test(ctx["localPort"].Equals(ipInfo.remotePort.ToString()));

            if(@base.ice_getConnection().type().Equals("ws") || @base.ice_getConnection().type().Equals("wss"))
            {
                Dictionary<string, string> headers = ((Ice.WSConnectionInfo)info).headers;
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

            connection = @base.ice_datagram().ice_getConnection();
            connection.setBufferSize(2048, 1024);

            Ice.UDPConnectionInfo udpInfo = (Ice.UDPConnectionInfo)connection.getInfo();
            test(!udpInfo.incoming);
            test(udpInfo.adapterName.Length == 0);
            test(udpInfo.localPort > 0);
            test(udpInfo.remotePort == endpointPort);

            if(defaultHost.Equals("127.0.0.1"))
            {
                test(udpInfo.remoteAddress.Equals(defaultHost));
                test(udpInfo.localAddress.Equals(defaultHost));
            }
            test(udpInfo.rcvSize >= 2048);
            test(udpInfo.sndSize >= 1024);
        }
        WriteLine("ok");

        testIntf.shutdown();

        communicator.shutdown();
        communicator.waitForShutdown();
    }
}
