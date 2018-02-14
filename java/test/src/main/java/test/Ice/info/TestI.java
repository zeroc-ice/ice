// **********************************************************************
//
// Copyright (c) 2003-2018 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.info;
import test.Ice.info.Test._TestIntfDisp;

public class TestI extends _TestIntfDisp
{
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

        Ice.IPEndpointInfo ipinfo = (Ice.IPEndpointInfo)info;
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

        Ice.IPConnectionInfo ipinfo = (Ice.IPConnectionInfo)info;
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

        if(info instanceof IceSSL.WSSConnectionInfo)
        {
            IceSSL.WSSConnectionInfo wssinfo = (IceSSL.WSSConnectionInfo)info;
            for(java.util.Map.Entry<String, String> e : wssinfo.headers.entrySet())
            {
                ctx.put("ws." + e.getKey(), e.getValue());
            }
        }

        return ctx;
    }
}
