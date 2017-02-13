// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.info;

import java.io.PrintWriter;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.OpaqueEndpointInfo;
import com.zeroc.Ice.SSLEndpointType;
import com.zeroc.Ice.TCPConnectionInfo;
import com.zeroc.Ice.TCPEndpointInfo;
import com.zeroc.Ice.TCPEndpointType;
import com.zeroc.Ice.UDPConnectionInfo;
import com.zeroc.Ice.UDPEndpointInfo;
import com.zeroc.Ice.UDPEndpointType;
import com.zeroc.Ice.WSConnectionInfo;
import com.zeroc.Ice.WSEndpointInfo;
import com.zeroc.Ice.WSEndpointType;
import com.zeroc.Ice.WSSEndpointType;

import test.Ice.info.Test.TestIntfPrx;

public class AllTests
{
    private static void test(boolean b)
    {
        if(!b)
        {
            throw new RuntimeException();
        }
    }

    private static TCPEndpointInfo getTCPEndpointInfo(EndpointInfo info)
    {
        for(EndpointInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof TCPEndpointInfo)
            {
                return (TCPEndpointInfo)p;
            }
        }
        return null;
    }

    private static TCPConnectionInfo getTCPConnectionInfo(ConnectionInfo info)
    {
        for(ConnectionInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof TCPConnectionInfo)
            {
                return (TCPConnectionInfo)p;
            }
        }
        return null;
    }

    public static void allTests(test.Util.Application app)
    {
        com.zeroc.Ice.Communicator communicator=app.communicator();
        PrintWriter out = app.getWriter();
        out.print("testing proxy endpoint information... ");
        out.flush();
        {
            com.zeroc.Ice.ObjectPrx p1 = communicator.stringToProxy(
                                "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress 10.10.10.10:" +
                                "udp -h udphost -p 10001 --interface eth0 --ttl 5 --sourceAddress 10.10.10.10:" +
                                "opaque -e 1.8 -t 100 -v ABCD");

            Endpoint[] endps = p1.ice_getEndpoints();
            EndpointInfo info = endps[0].getInfo();
            TCPEndpointInfo tcpEndpoint = (TCPEndpointInfo)getTCPEndpointInfo(info);
            test(tcpEndpoint.host.equals("tcphost"));
            test(tcpEndpoint.port == 10000);
            test(tcpEndpoint.timeout == 1200);
            test(tcpEndpoint.sourceAddress.equals("10.10.10.10"));
            test(tcpEndpoint.compress);
            test(!tcpEndpoint.datagram());
            test(tcpEndpoint.type() == TCPEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == SSLEndpointType.value && tcpEndpoint.secure() ||
                 tcpEndpoint.type() == WSEndpointType.value && !tcpEndpoint.secure() ||
                 tcpEndpoint.type() == WSSEndpointType.value && tcpEndpoint.secure());

            test(tcpEndpoint.type() == TCPEndpointType.value && info instanceof TCPEndpointInfo ||
                 tcpEndpoint.type() == SSLEndpointType.value && info instanceof com.zeroc.IceSSL.EndpointInfo ||
                 tcpEndpoint.type() == WSEndpointType.value && info instanceof WSEndpointInfo ||
                 tcpEndpoint.type() == WSSEndpointType.value && info instanceof WSEndpointInfo);

            UDPEndpointInfo udpEndpoint = (UDPEndpointInfo)endps[1].getInfo();
            test(udpEndpoint.host.equals("udphost"));
            test(udpEndpoint.port == 10001);
            test(udpEndpoint.mcastInterface.equals("eth0"));
            test(udpEndpoint.mcastTtl == 5);
            test(udpEndpoint.sourceAddress.equals("10.10.10.10"));
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == UDPEndpointType.value);

            OpaqueEndpointInfo opaqueEndpoint = (OpaqueEndpointInfo)endps[2].getInfo();
            test(opaqueEndpoint.rawEncoding.equals(new com.zeroc.Ice.EncodingVersion((byte)1, (byte)8)));
        }
        out.println("ok");

        String defaultHost = communicator.getProperties().getProperty("Ice.Default.Host");
        out.print("test object adapter endpoint information... ");
        out.flush();
        {
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -t 15000:udp");
            com.zeroc.Ice.ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");

            Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.length == 2);
            Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(java.util.Arrays.equals(endpoints, publishedEndpoints));

            TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo());
            test(tcpEndpoint.type() == TCPEndpointType.value || tcpEndpoint.type() == SSLEndpointType.value ||
                 tcpEndpoint.type() == WSEndpointType.value || tcpEndpoint.type() == WSSEndpointType.value);
            test(tcpEndpoint.host.equals(defaultHost));
            test(tcpEndpoint.port > 0);
            test(tcpEndpoint.timeout == 15000);

            UDPEndpointInfo udpEndpoint = (UDPEndpointInfo)endpoints[1].getInfo();
            test(udpEndpoint.host.equals(defaultHost));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            endpoints = new Endpoint[]{endpoints[0]};
            test(endpoints.length == 1);
            adapter.setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(java.util.Arrays.equals(endpoints, publishedEndpoints));

            adapter.destroy();

            int port = app.getTestPort(1);
            communicator.getProperties().setProperty("TestAdapter.Endpoints", "default -h * -p " + port);
            communicator.getProperties().setProperty("TestAdapter.PublishedEndpoints", app.getTestEndpoint(1));
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.length == 1);

            for(Endpoint endpoint : endpoints)
            {
                tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo());
                test(tcpEndpoint.port == port);
            }

            tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo());
            test(tcpEndpoint.host.equals("127.0.0.1"));
            test(tcpEndpoint.port == port);

            adapter.destroy();
        }
        out.println("ok");

        com.zeroc.Ice.ObjectPrx base = communicator.stringToProxy("test:" +
                                                                  app.getTestEndpoint(0) + ":" +
                                                                  app.getTestEndpoint(0, "udp"));
        TestIntfPrx testIntf = TestIntfPrx.checkedCast(base);

        int endpointPort = app.getTestPort(0);

        out.print("test connection endpoint information... ");
        out.flush();
        {
            EndpointInfo info = base.ice_getConnection().getEndpoint().getInfo();
            TCPEndpointInfo tcpinfo = getTCPEndpointInfo(info);
            test(tcpinfo.port == endpointPort);
            test(!tcpinfo.compress);
            test(tcpinfo.host.equals(defaultHost));

            java.util.Map<String, String> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx.get("host").equals(tcpinfo.host));
            test(ctx.get("compress").equals("false"));
            int port = Integer.parseInt(ctx.get("port"));
            test(port > 0);

            info = base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            UDPEndpointInfo udp = (UDPEndpointInfo)info;
            test(udp.port == endpointPort);
            test(udp.host.equals(defaultHost));
        }
        out.println("ok");

        out.print("testing connection information... ");
        out.flush();
        {
            com.zeroc.Ice.Connection connection = base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            TCPConnectionInfo info = getTCPConnectionInfo(connection.getInfo());
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
            TCPConnectionInfo info2 = (TCPConnectionInfo)info.clone();

            java.util.Map<String, String> ctx = testIntf.getConnectionInfoAsContext();
            test(ctx.get("incoming").equals("true"));
            test(ctx.get("adapterName").equals("TestAdapter"));
            test(ctx.get("remoteAddress").equals(info.localAddress));
            test(ctx.get("localAddress").equals(info.remoteAddress));
            test(ctx.get("remotePort").equals(Integer.toString(info.localPort)));
            test(ctx.get("localPort").equals(Integer.toString(info.remotePort)));

            if(base.ice_getConnection().type().equals("ws") || base.ice_getConnection().type().equals("wss"))
            {
                java.util.Map<String, String> headers = ((WSConnectionInfo)connection.getInfo()).headers;
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

            UDPConnectionInfo udpinfo = (UDPConnectionInfo)connection.getInfo();
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
