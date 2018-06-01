// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.info;

import java.io.PrintWriter;

import test.Ice.info.Test.TestIntfPrx;
import test.Ice.info.Test.TestIntfPrxHelper;

public class AllTests
{
    private static void
    test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static Ice.TCPEndpointInfo
    getTCPEndpointInfo(Ice.EndpointInfo info)
    {
        for(Ice.EndpointInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof Ice.TCPEndpointInfo)
            {
                return (Ice.TCPEndpointInfo)p;
            }
        }
        return null;
    }

    private static Ice.TCPConnectionInfo
    getTCPConnectionInfo(Ice.ConnectionInfo info)
    {
        for(Ice.ConnectionInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof Ice.TCPConnectionInfo)
            {
                return (Ice.TCPConnectionInfo)p;
            }
        }
        return null;
    }

    public static void allTests(test.TestHelper helper)
    {
        Ice.Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing proxy endpoint information... ");
        out.flush();
        {
            Ice.ObjectPrx p1 = communicator.stringToProxy(
                                "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                                "opaque -e 1.8 -t 100 -v ABCD");

            Ice.Endpoint[] endps = p1.ice_getEndpoints();
            Ice.EndpointInfo info = endps[0].getInfo();
            Ice.TCPEndpointInfo tcpEndpoint = (Ice.TCPEndpointInfo)getTCPEndpointInfo(info);
            test(tcpEndpoint.host.equals("tcphost"));
            test(tcpEndpoint.port == 10000);
            test(tcpEndpoint.timeout == 1200);
            test(tcpEndpoint.sourceAddress.equals("10.10.10.10"));
            test(tcpEndpoint.compress);
            test(!tcpEndpoint.datagram());
            test(tcpEndpoint.type() == Ice.TCPEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.SSLEndpointType.value && tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == Ice.WSSEndpointType.value && tcpEndpoint.secure());

            test(tcpEndpoint.type() == Ice.TCPEndpointType.value && info instanceof Ice.TCPEndpointInfo ||
                 tcpEndpoint.type() == Ice.SSLEndpointType.value && info instanceof IceSSL.EndpointInfo ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value && info instanceof Ice.WSEndpointInfo ||
                 tcpEndpoint.type() == Ice.WSSEndpointType.value && info instanceof Ice.WSEndpointInfo);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endps[1].getInfo();
            test(udpEndpoint.host.equals("udphost"));
            test(udpEndpoint.port == 10001);
            test(udpEndpoint.mcastInterface.equals("eth0"));
            test(udpEndpoint.mcastTtl == 5);
            test(udpEndpoint.sourceAddress.equals("10.10.10.10"));
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == Ice.UDPEndpointType.value);

            Ice.OpaqueEndpointInfo opaqueEndpoint = (Ice.OpaqueEndpointInfo)endps[2].getInfo();
            test(opaqueEndpoint.rawEncoding.equals(new Ice.EncodingVersion((byte)1, (byte)8)));
        }
        out.println("ok");

        out.print("test object adapter endpoint information... ");
        out.flush();
        {
            final String host = communicator.getProperties().getPropertyAsInt("Ice.IPv6") != 0 ? "::1" : "127.0.0.1";
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "tcp -h \"" + host +
                "\" -t 15000:udp -h \"" + host + "\"");
            Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");

            Ice.Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.length == 2);
            Ice.Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(java.util.Arrays.equals(endpoints, publishedEndpoints));

            Ice.TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo());
            test(tcpEndpoint.type() == Ice.TCPEndpointType.value || tcpEndpoint.type() == Ice.SSLEndpointType.value ||
                 tcpEndpoint.type() == Ice.WSEndpointType.value || tcpEndpoint.type() == Ice.WSSEndpointType.value);
            test(tcpEndpoint.host.equals(host));
            test(tcpEndpoint.port > 0);
            test(tcpEndpoint.timeout == 15000);

            Ice.UDPEndpointInfo udpEndpoint = (Ice.UDPEndpointInfo)endpoints[1].getInfo();
            test(udpEndpoint.host.equals(host));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            endpoints = new Ice.Endpoint[]{endpoints[0]};
            test(endpoints.length == 1);
            adapter.setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(java.util.Arrays.equals(endpoints, publishedEndpoints));

            adapter.destroy();

            int port = helper.getTestPort(1);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "tcp -h * -p " + port);
            communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", "tcp -h dummy -p " + port);
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.length == 1);

            for(Ice.Endpoint endpoint : endpoints)
            {
                tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo());
                test(tcpEndpoint.port == port);
            }

            tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo());
            test(tcpEndpoint.host.equals("dummy"));
            test(tcpEndpoint.port == port);

            adapter.destroy();
        }
        out.println("ok");

        Ice.ObjectPrx base = communicator.stringToProxy("test:" +
                                                        helper.getTestEndpoint(0) + ":" +
                                                        helper.getTestEndpoint(0, "udp"));
        TestIntfPrx testIntf = TestIntfPrxHelper.checkedCast(base);
        int endpointPort = helper.getTestPort(0);

        final String defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
        out.print("test connection endpoint information... ");
        out.flush();
        {
            Ice.EndpointInfo info = base.ice_getConnection().getEndpoint().getInfo();
            Ice.TCPEndpointInfo tcpinfo = getTCPEndpointInfo(info);
            test(tcpinfo.port == endpointPort);
            test(!tcpinfo.compress);
            test(tcpinfo.host.equals(defaultHost));

            java.util.Map<String, String> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx.get("host").equals(tcpinfo.host));
            test(ctx.get("compress").equals("false"));
            int port = Integer.parseInt(ctx.get("port"));
            test(port > 0);

            info = base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)info;
            test(udp.port == endpointPort);
            test(udp.host.equals(defaultHost));
        }
        out.println("ok");

        out.print("testing connection information... ");
        out.flush();
        {
            Ice.Connection connection = base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            Ice.TCPConnectionInfo info = getTCPConnectionInfo(connection.getInfo());
            test(!info.incoming);
            test(info.adapterName.length() == 0);
            test(info.localPort > 0);
            test(info.remotePort == endpointPort);
            if(defaultHost.equals("127.0.0.1"))
            {
                test(info.remoteAddress.equals(defaultHost));
                test(info.localAddress.equals(defaultHost));
            }
            test(info.rcvSize >= 1024);
            test(info.sndSize >= 2048);

            //
            // Make sure the local slice class is cloneable
            //
            java.lang.Cloneable cloneable = info;
            Ice.TCPConnectionInfo info2 = (Ice.TCPConnectionInfo)info.clone();

            java.util.Map<String, String> ctx = testIntf.getConnectionInfoAsContext();
            test(ctx.get("incoming").equals("true"));
            test(ctx.get("adapterName").equals("TestAdapter"));
            test(ctx.get("remoteAddress").equals(info.localAddress));
            test(ctx.get("localAddress").equals(info.remoteAddress));
            test(ctx.get("remotePort").equals(Integer.toString(info.localPort)));
            test(ctx.get("localPort").equals(Integer.toString(info.remotePort)));

            if(base.ice_getConnection().type().equals("ws") || base.ice_getConnection().type().equals("wss"))
            {
                java.util.Map<String, String> headers = ((Ice.WSConnectionInfo)connection.getInfo()).headers;
                test(headers.get("Upgrade").equals("websocket"));
                test(headers.get("Connection").equals("Upgrade"));
                test(headers.get("Sec-WebSocket-Protocol").equals("ice.zeroc.com"));
                test(headers.get("Sec-WebSocket-Accept") != null);

                test(ctx.get("ws.Upgrade").equals("websocket"));
                test(ctx.get("ws.Connection").equals("Upgrade"));
                test(ctx.get("ws.Sec-WebSocket-Protocol").equals("ice.zeroc.com"));
                test(ctx.get("ws.Sec-WebSocket-Version").equals("13"));
                test(ctx.get("ws.Sec-WebSocket-Key") != null);
            }

            connection = base.ice_datagram().ice_getConnection();
            connection.setBufferSize(2048, 1024);

            Ice.UDPConnectionInfo udpinfo = (Ice.UDPConnectionInfo)connection.getInfo();
            test(!udpinfo.incoming);
            test(udpinfo.adapterName.length() == 0);
            test(udpinfo.localPort > 0);
            test(udpinfo.remotePort == endpointPort);
            if(defaultHost.equals("127.0.0.1"))
            {
                test(udpinfo.remoteAddress.equals(defaultHost));
                test(udpinfo.localAddress.equals(defaultHost));
            }
            test(udpinfo.rcvSize >= 2048);
            test(udpinfo.sndSize >= 1024);
        }
        out.println("ok");

        testIntf.shutdown();

        communicator.shutdown();
        communicator.waitForShutdown();
    }
}
