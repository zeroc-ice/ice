// Copyright (c) ZeroC, Inc.

namespace Ice.info;

public class TestI : Test.TestIntfDisp_
{
    private static Ice.IPEndpointInfo getIPEndpointInfo(Ice.EndpointInfo info)
    {
        for (; info != null; info = info.underlying)
        {
            if (info is Ice.IPEndpointInfo)
            {
                return info as Ice.IPEndpointInfo;
            }
        }
        return null;
    }

    private static Ice.IPConnectionInfo getIPConnectionInfo(Ice.ConnectionInfo info)
    {
        for (; info != null; info = info.underlying)
        {
            if (info is Ice.IPConnectionInfo)
            {
                return info as Ice.IPConnectionInfo;
            }
        }
        return null;
    }

    public override void shutdown(Ice.Current current) => current.adapter.getCommunicator().shutdown();

    public override Dictionary<string, string> getEndpointInfoAsContext(Ice.Current current)
    {
        var ctx = new Dictionary<string, string>();
        Ice.EndpointInfo info = current.con.getEndpoint().getInfo();
        ctx["compress"] = info.compress ? "true" : "false";
        ctx["datagram"] = info.datagram() ? "true" : "false";
        ctx["secure"] = info.datagram() ? "true" : "false";
        ctx["type"] = $"P{info.type()}";

        Ice.IPEndpointInfo ipinfo = getIPEndpointInfo(info);
        ctx["host"] = ipinfo.host;
        ctx["port"] = $"{ipinfo.port}";

        return ctx;
    }

    public override Dictionary<string, string> getConnectionInfoAsContext(Ice.Current current)
    {
        var ctx = new Dictionary<string, string>();
        Ice.ConnectionInfo info = current.con.getInfo();
        ctx["adapterName"] = info.adapterName;
        ctx["incoming"] = info.incoming ? "true" : "false";

        Ice.IPConnectionInfo ipinfo = getIPConnectionInfo(info);
        ctx["localAddress"] = ipinfo.localAddress;
        ctx["localPort"] = $"{ipinfo.localPort}";
        ctx["remoteAddress"] = ipinfo.remoteAddress;
        ctx["remotePort"] = $"{ipinfo.remotePort}";

        if (info is Ice.WSConnectionInfo wsinfo)
        {
            foreach (KeyValuePair<string, string> e in wsinfo.headers)
            {
                ctx["ws." + e.Key] = e.Value;
            }
        }

        return ctx;
    }
}
