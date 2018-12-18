// **********************************************************************
//
// Copyright (c) 2003-present ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

package test.Ice.info;

import test.Ice.info.Test.TestIntf;

public class TestI implements TestIntf
{
    private static com.zeroc.Ice.IPEndpointInfo getIPEndpointInfo(com.zeroc.Ice.EndpointInfo info)
    {
        for(com.zeroc.Ice.EndpointInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof com.zeroc.Ice.IPEndpointInfo)
            {
                return (com.zeroc.Ice.IPEndpointInfo)p;
            }
        }
        return null;
    }

    private static com.zeroc.Ice.IPConnectionInfo getIPConnectionInfo(com.zeroc.Ice.ConnectionInfo info)
    {
        for(com.zeroc.Ice.ConnectionInfo p = info; p != null; p = p.underlying)
        {
            if(p instanceof com.zeroc.Ice.IPConnectionInfo)
            {
                return (com.zeroc.Ice.IPConnectionInfo)p;
            }
        }
        return null;
    }

    TestI()
    {
    }

    @Override
    public void shutdown(com.zeroc.Ice.Current current)
    {
        current.adapter.getCommunicator().shutdown();
    }

    @Override
    public java.util.Map<String, String> getEndpointInfoAsContext(com.zeroc.Ice.Current c)
    {
        java.util.Map<String, String> ctx = new java.util.HashMap<>();
        com.zeroc.Ice.EndpointInfo info = c.con.getEndpoint().getInfo();
        ctx.put("timeout", Integer.toString(info.timeout));
        ctx.put("compress", info.compress ? "true" : "false");
        ctx.put("datagram", info.datagram() ? "true" : "false");
        ctx.put("secure", info.datagram() ? "true" : "false");
        ctx.put("type", Integer.toString(info.type()));

        com.zeroc.Ice.IPEndpointInfo ipinfo = getIPEndpointInfo(info);
        ctx.put("host", ipinfo.host);
        ctx.put("port", Integer.toString(ipinfo.port));

        if(ipinfo instanceof com.zeroc.Ice.UDPEndpointInfo)
        {
            com.zeroc.Ice.UDPEndpointInfo udp = (com.zeroc.Ice.UDPEndpointInfo)ipinfo;
            ctx.put("mcastInterface", udp.mcastInterface);
            ctx.put("mcastTtl", Integer.toString(udp.mcastTtl));
        }

        return ctx;
    }

    @Override
    public java.util.Map<String, String> getConnectionInfoAsContext(com.zeroc.Ice.Current c)
    {
        java.util.Map<String, String> ctx = new java.util.HashMap<>();
        com.zeroc.Ice.ConnectionInfo info = c.con.getInfo();
        ctx.put("adapterName", info.adapterName);
        ctx.put("incoming", info.incoming ? "true" : "false");

        com.zeroc.Ice.IPConnectionInfo ipinfo = getIPConnectionInfo(info);
        ctx.put("localAddress", ipinfo.localAddress);
        ctx.put("localPort", Integer.toString(ipinfo.localPort));
        ctx.put("remoteAddress", ipinfo.remoteAddress);
        ctx.put("remotePort", Integer.toString(ipinfo.remotePort));

        if(info instanceof com.zeroc.Ice.WSConnectionInfo)
        {
            com.zeroc.Ice.WSConnectionInfo wsinfo = (com.zeroc.Ice.WSConnectionInfo)info;
            for(java.util.Map.Entry<String, String> e : wsinfo.headers.entrySet())
            {
                ctx.put("ws." + e.getKey(), e.getValue());
            }
        }

        return ctx;
    }
}
