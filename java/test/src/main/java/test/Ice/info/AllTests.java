// Copyright (c) ZeroC, Inc.

package test.Ice.info;

import com.zeroc.Ice.Communicator;
import com.zeroc.Ice.Connection;
import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.EncodingVersion;
import com.zeroc.Ice.Endpoint;
import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.ObjectAdapter;
import com.zeroc.Ice.ObjectPrx;
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
import test.TestHelper;

import java.io.PrintWriter;
import java.security.MessageDigest;
import java.security.NoSuchAlgorithmException;
import java.security.cert.CertificateEncodingException;
import java.util.Arrays;
import java.util.Map;

public class AllTests {
    private static void test(boolean b) {
        if (!b) {
            throw new RuntimeException();
        }
    }

    private static TCPEndpointInfo getTCPEndpointInfo(EndpointInfo info) {
        for (EndpointInfo p = info; p != null; p = p.underlying) {
            if (p instanceof TCPEndpointInfo) {
                return (TCPEndpointInfo) p;
            }
        }
        return null;
    }

    private static TCPConnectionInfo getTCPConnectionInfo(ConnectionInfo info) {
        for (ConnectionInfo p = info; p != null; p = p.underlying) {
            if (p instanceof TCPConnectionInfo) {
                return (TCPConnectionInfo) p;
            }
        }
        return null;
    }

    public static void allTests(TestHelper helper) {
        Communicator communicator = helper.communicator();
        PrintWriter out = helper.getWriter();
        out.print("testing proxy endpoint information... ");
        out.flush();
        {
            ObjectPrx p1 =
                communicator.stringToProxy(
                    "test -t:default -h tcphost -p 10000 -t 1200 -z --sourceAddress"
                        + " 10.10.10.10:udp -h udphost -p 10001 --interface eth0 --ttl 5"
                        + " --sourceAddress 10.10.10.10:opaque -e 1.8 -t 100 -v ABCD");

            Endpoint[] endps = p1.ice_getEndpoints();
            EndpointInfo info = endps[0].getInfo();
            TCPEndpointInfo tcpEndpoint = (TCPEndpointInfo) getTCPEndpointInfo(info);
            test("tcphost".equals(tcpEndpoint.host));
            test(tcpEndpoint.port == 10000);
            test(tcpEndpoint.timeout == 1200);
            test("10.10.10.10".equals(tcpEndpoint.sourceAddress));
            test(tcpEndpoint.compress);
            test(!tcpEndpoint.datagram());
            test(
                tcpEndpoint.type() == TCPEndpointType.value && !tcpEndpoint.secure()
                    || tcpEndpoint.type() == SSLEndpointType.value && tcpEndpoint.secure()
                    || tcpEndpoint.type() == WSEndpointType.value && !tcpEndpoint.secure()
                    || tcpEndpoint.type() == WSSEndpointType.value && tcpEndpoint.secure());

            test(
                tcpEndpoint.type() == TCPEndpointType.value && info instanceof TCPEndpointInfo
                    || tcpEndpoint.type() == SSLEndpointType.value
                    && info instanceof com.zeroc.Ice.SSL.EndpointInfo
                    || tcpEndpoint.type() == WSEndpointType.value
                    && info instanceof WSEndpointInfo
                    || tcpEndpoint.type() == WSSEndpointType.value
                    && info instanceof WSEndpointInfo);

            UDPEndpointInfo udpEndpoint = (UDPEndpointInfo) endps[1].getInfo();
            test("udphost".equals(udpEndpoint.host));
            test(udpEndpoint.port == 10001);
            test("eth0".equals(udpEndpoint.mcastInterface));
            test(udpEndpoint.mcastTtl == 5);
            test("10.10.10.10".equals(udpEndpoint.sourceAddress));
            test(udpEndpoint.timeout == -1);
            test(!udpEndpoint.compress);
            test(!udpEndpoint.secure());
            test(udpEndpoint.datagram());
            test(udpEndpoint.type() == UDPEndpointType.value);

            OpaqueEndpointInfo opaqueEndpoint = (OpaqueEndpointInfo) endps[2].getInfo();
            test(
                opaqueEndpoint.rawEncoding.equals(
                    new EncodingVersion((byte) 1, (byte) 8)));
        }
        out.println("ok");

        out.print("test object adapter endpoint information... ");
        out.flush();
        {
            final String host =
                communicator.getProperties().getIcePropertyAsInt("Ice.IPv6") != 0
                    ? "::1"
                    : "127.0.0.1";
            communicator
                .getProperties()
                .setProperty(
                    "TestAdapter.Endpoints",
                    "tcp -h \"" + host + "\" -t 15000:udp -h \"" + host + "\"");
            ObjectAdapter adapter = communicator.createObjectAdapter("TestAdapter");

            Endpoint[] endpoints = adapter.getEndpoints();
            test(endpoints.length == 2);
            Endpoint[] publishedEndpoints = adapter.getPublishedEndpoints();
            test(Arrays.equals(endpoints, publishedEndpoints));

            TCPEndpointInfo tcpEndpoint = getTCPEndpointInfo(endpoints[0].getInfo());
            test(
                tcpEndpoint.type() == TCPEndpointType.value
                    || tcpEndpoint.type() == SSLEndpointType.value
                    || tcpEndpoint.type() == WSEndpointType.value
                    || tcpEndpoint.type() == WSSEndpointType.value);
            test(tcpEndpoint.host.equals(host));
            test(tcpEndpoint.port > 0);
            test(tcpEndpoint.timeout == 15000);

            UDPEndpointInfo udpEndpoint = (UDPEndpointInfo) endpoints[1].getInfo();
            test(udpEndpoint.host.equals(host));
            test(udpEndpoint.datagram());
            test(udpEndpoint.port > 0);

            endpoints = new Endpoint[]{endpoints[0]};
            test(endpoints.length == 1);
            adapter.setPublishedEndpoints(endpoints);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(Arrays.equals(endpoints, publishedEndpoints));

            adapter.destroy();

            int port = helper.getTestPort(1);
            communicator
                .getProperties()
                .setProperty("TestAdapter.Endpoints", "tcp -h * -p " + port);
            communicator
                .getProperties()
                .setProperty("TestAdapter.PublishedEndpoints", "tcp -h dummy -p " + port);
            adapter = communicator.createObjectAdapter("TestAdapter");

            endpoints = adapter.getEndpoints();
            test(endpoints.length >= 1);
            publishedEndpoints = adapter.getPublishedEndpoints();
            test(publishedEndpoints.length == 1);

            for (Endpoint endpoint : endpoints) {
                tcpEndpoint = getTCPEndpointInfo(endpoint.getInfo());
                test(tcpEndpoint.port == port);
            }

            tcpEndpoint = getTCPEndpointInfo(publishedEndpoints[0].getInfo());
            test("dummy".equals(tcpEndpoint.host));
            test(tcpEndpoint.port == port);

            adapter.destroy();
        }
        out.println("ok");

        ObjectPrx base =
            communicator.stringToProxy(
                "test:"
                    + helper.getTestEndpoint(0)
                    + ":"
                    + helper.getTestEndpoint(0, "udp"));
        TestIntfPrx testIntf = TestIntfPrx.checkedCast(base);

        int endpointPort = helper.getTestPort(0);

        final String defaultHost = communicator.getProperties().getIceProperty("Ice.Default.Host");
        out.print("test connection endpoint information... ");
        out.flush();
        {
            EndpointInfo info = base.ice_getConnection().getEndpoint().getInfo();
            TCPEndpointInfo tcpinfo = getTCPEndpointInfo(info);
            test(tcpinfo.port == endpointPort);
            test(!tcpinfo.compress);
            test(tcpinfo.host.equals(defaultHost));

            Map<String, String> ctx = testIntf.getEndpointInfoAsContext();
            test(ctx.get("host").equals(tcpinfo.host));
            test("false".equals(ctx.get("compress")));
            int port = Integer.parseInt(ctx.get("port"));
            test(port > 0);

            info = base.ice_datagram().ice_getConnection().getEndpoint().getInfo();
            UDPEndpointInfo udp = (UDPEndpointInfo) info;
            test(udp.port == endpointPort);
            test(udp.host.equals(defaultHost));
        }
        out.println("ok");

        out.print("testing connection information... ");
        out.flush();
        {
            Connection connection = base.ice_getConnection();
            connection.setBufferSize(1024, 2048);

            TCPConnectionInfo info = getTCPConnectionInfo(connection.getInfo());
            test(!info.incoming);
            test(info.adapterName.isEmpty());
            test(info.localPort > 0);
            test(info.remotePort == endpointPort);
            if ("127.0.0.1".equals(defaultHost)) {
                test(info.remoteAddress.equals(defaultHost));
                test(info.localAddress.equals(defaultHost));
            }
            test(info.rcvSize >= 1024);
            test(info.sndSize >= 2048);

            Map<String, String> ctx = testIntf.getConnectionInfoAsContext();
            test("true".equals(ctx.get("incoming")));
            test("TestAdapter".equals(ctx.get("adapterName")));
            test(ctx.get("remoteAddress").equals(info.localAddress));
            test(ctx.get("localAddress").equals(info.remoteAddress));
            test(ctx.get("remotePort").equals(Integer.toString(info.localPort)));
            test(ctx.get("localPort").equals(Integer.toString(info.remotePort)));

            if ("ws".equals(base.ice_getConnection().type())
                || "wss".equals(base.ice_getConnection().type())) {
                var wssInfo = (WSConnectionInfo) connection.getInfo();
                Map<String, String> headers = wssInfo.headers;
                test("websocket".equals(headers.get("Upgrade")));
                test("Upgrade".equals(headers.get("Connection")));
                test("ice.zeroc.com".equals(headers.get("Sec-WebSocket-Protocol")));
                test(headers.get("Sec-WebSocket-Accept") != null);

                test("websocket".equals(ctx.get("ws.Upgrade")));
                test("Upgrade".equals(ctx.get("ws.Connection")));
                test("ice.zeroc.com".equals(ctx.get("ws.Sec-WebSocket-Protocol")));
                test("13".equals(ctx.get("ws.Sec-WebSocket-Version")));
                test(ctx.get("ws.Sec-WebSocket-Key") != null);

                if ("wss".equals(base.ice_getConnection().type())) {
                    checkPeerCertificate((com.zeroc.Ice.SSL.ConnectionInfo) wssInfo.underlying);
                }
            } else if ("ssl".equals(base.ice_getConnection().type())) {
                checkPeerCertificate((com.zeroc.Ice.SSL.ConnectionInfo) connection.getInfo());
            }

            connection = base.ice_datagram().ice_getConnection();
            connection.setBufferSize(2048, 1024);

            UDPConnectionInfo udpinfo = (UDPConnectionInfo) connection.getInfo();
            test(!udpinfo.incoming);
            test(udpinfo.adapterName.isEmpty());
            test(udpinfo.localPort > 0);
            test(udpinfo.remotePort == endpointPort);
            if ("127.0.0.1".equals(defaultHost)) {
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

    static void checkPeerCertificate(com.zeroc.Ice.SSL.ConnectionInfo info) {
        test(info.certs.length > 0);
        try {
            byte[] thumbprint =
                MessageDigest.getInstance("SHA-1").digest(info.certs[0].getEncoded());

            // The SHA1 Thumbprint of the server certificate used in the test.
            // 45:B0:40:4F:12:CF:3F:E6:37:CF:EE:9C:53:C8:89:C9:60:22:76:DF
            byte[] expected = {
                (byte) 0x45,
                (byte) 0xB0,
                (byte) 0x40,
                (byte) 0x4F,
                (byte) 0x12,
                (byte) 0xCF,
                (byte) 0x3F,
                (byte) 0xE6,
                (byte) 0x37,
                (byte) 0xCF,
                (byte) 0xEE,
                (byte) 0x9C,
                (byte) 0x53,
                (byte) 0xC8,
                (byte) 0x89,
                (byte) 0xC9,
                (byte) 0x60,
                (byte) 0x22,
                (byte) 0x76,
                (byte) 0xDF,
            };
            test(Arrays.equals(thumbprint, expected));
        } catch (NoSuchAlgorithmException e) {
            test(false);
        } catch (CertificateEncodingException e) {
            test(false);
        }
    }

    private AllTests() {}
}
