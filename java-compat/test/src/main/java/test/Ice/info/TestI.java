//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

package test.Ice.info;
import test.Ice.info.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
    private static Ice.IPEndpointInfo
    getIPEndpointInfo(Ice.EndpointInfo info)
    {
        for(Ice.EndpointInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof Ice.IPEndpointInfo)
            {
                return (Ice.IPEndpointInfo)p;
            }
        }
        return null;
    }

    private static Ice.IPConnectionInfo
    getIPConnectionInfo(Ice.ConnectionInfo info)
    {
        for(Ice.ConnectionInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof Ice.IPConnectionInfo)
            {
                return (Ice.IPConnectionInfo)p;
            }
        }
        return null;
    }

    TestI()
    {
    }

    @Override
    public void shutdown(Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public java.util.Map<String, String> getEndpointInfoAsContext(Ice.Current c)
    {
        java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
        Ice.EndpointInfo info = c.con.getEndpoint().getInfo();
        ctx.put("timeout", Integer.toString(info.timeout));
        ctx.put("compress", info.compress ? "true" : "false");
        ctx.put("datagram", info.datagram() ? "true" : "false");
        ctx.put("secure", info.datagram() ? "true" : "false");
        ctx.put("type", Integer.toString(info.type()));

        Ice.IPEndpointInfo ipinfo = getIPEndpointInfo(info);
        ctx.put("host", ipinfo.host);
        ctx.put("port", Integer.toString(ipinfo.port));

        if(ipinfo instanceof Ice.UDPEndpointInfo)
        {
            Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)ipinfo;
            ctx.put("mcastInterface", udp.mcastInterface);
            ctx.put("mcastTtl", Integer.toString(udp.mcastTtl));
        }

        return ctx;
    }

    @Override
    public java.util.Map<String, String> getConnectionInfoAsContext(Ice.Current c)
    {
        java.util.Map<String, String> ctx = new java.util.HashMap<String, String>();
        Ice.ConnectionInfo info = c.con.getInfo();
        ctx.put("adapterName", info.adapterName);
        ctx.put("incoming", info.incoming ? "true" : "false");

        Ice.IPConnectionInfo ipinfo = getIPConnectionInfo(info);
        ctx.put("localAddress", ipinfo.localAddress);
        ctx.put("localPort", Integer.toString(ipinfo.localPort));
        ctx.put("remoteAddress", ipinfo.remoteAddress);
        ctx.put("remotePort", Integer.toString(ipinfo.remotePort));

        if(info instanceof Ice.WSConnectionInfo)
        {
            Ice.WSConnectionInfo wsinfo = (Ice.WSConnectionInfo)info;
            for(java.util.Map.Entry<String, String> e : wsinfo.headers.entrySet())
            {
                ctx.put("ws." + e.getKey(), e.getValue());
            }
        }

        return ctx;
    }
}
