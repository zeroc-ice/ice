// Copyright (c) ZeroC, Inc.

package test.Ice.info;

import com.zeroc.Ice.ConnectionInfo;
import com.zeroc.Ice.Current;
import com.zeroc.Ice.EndpointInfo;
import com.zeroc.Ice.IPConnectionInfo;
import com.zeroc.Ice.IPEndpointInfo;
import com.zeroc.Ice.UDPEndpointInfo;
import com.zeroc.Ice.WSConnectionInfo;

import test.Ice.info.Test.TestIntf;

import java.util.HashMap;
import java.util.Map;

public class TestI implements TestIntf {
    private static IPEndpointInfo getIPEndpointInfo(EndpointInfo info) {
        for (EndpointInfo p = info; p != null; p = p.underlying) {
            if (p instanceof IPEndpointInfo) {
                return (IPEndpointInfo) p;
            }
        }
        return null;
    }

    private static IPConnectionInfo getIPConnectionInfo(
            ConnectionInfo info) {
        for (ConnectionInfo p = info; p != null; p = p.underlying) {
            if (p instanceof IPConnectionInfo) {
                return (IPConnectionInfo) p;
            }
        }
        return null;
    }

    TestI() {
    }

    @Override
    public void shutdown(Current current) {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public Map<String, String> getEndpointInfoAsContext(Current c) {
        Map<String, String> ctx = new HashMap<>();
        EndpointInfo info = c.con.getEndpoint().getInfo();
        ctx.put("timeout", Integer.toString(info.timeout));
        ctx.put("compress", info.compress ? "true" : "false");
        ctx.put("datagram", info.datagram() ? "true" : "false");
        ctx.put("secure", info.datagram() ? "true" : "false");
        ctx.put("type", Integer.toString(info.type()));

        IPEndpointInfo ipinfo = getIPEndpointInfo(info);
        ctx.put("host", ipinfo.host);
        ctx.put("port", Integer.toString(ipinfo.port));

        if (ipinfo instanceof UDPEndpointInfo) {
            UDPEndpointInfo udp = (UDPEndpointInfo) ipinfo;
            ctx.put("mcastInterface", udp.mcastInterface);
            ctx.put("mcastTtl", Integer.toString(udp.mcastTtl));
        }

        return ctx;
    }

    @Override
    public Map<String, String> getConnectionInfoAsContext(Current c) {
        Map<String, String> ctx = new HashMap<>();
        ConnectionInfo info = c.con.getInfo();
        ctx.put("adapterName", info.adapterName);
        ctx.put("incoming", info.incoming ? "true" : "false");

        IPConnectionInfo ipinfo = getIPConnectionInfo(info);
        ctx.put("localAddress", ipinfo.localAddress);
        ctx.put("localPort", Integer.toString(ipinfo.localPort));
        ctx.put("remoteAddress", ipinfo.remoteAddress);
        ctx.put("remotePort", Integer.toString(ipinfo.remotePort));

        if (info instanceof WSConnectionInfo) {
            WSConnectionInfo wsinfo = (WSConnectionInfo) info;
            for (Map.Entry<String, String> e : wsinfo.headers.entrySet()) {
                ctx.put("ws." + e.getKey(), e.getValue());
            }
        }

        return ctx;
    }
}
