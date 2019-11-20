//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;

namespace Ice
{
    namespace info
    {
        public class TestI : Test.TestIntf
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

            public void shutdown(Ice.Current current)
            {
                current.adapter.getCommunicator().shutdown();
            }

            public Dictionary<string, string> getEndpointInfoAsContext(Ice.Current c)
            {
                Dictionary<string, string> ctx = new Dictionary<string, string>();
                Ice.EndpointInfo info = c.con.getEndpoint().getInfo();
                ctx["timeout"] = info.timeout.ToString();
                ctx["compress"] = info.compress ? "true" : "false";
                ctx["datagram"] = info.datagram() ? "true" : "false";
                ctx["secure"] = info.datagram() ? "true" : "false";
                ctx["type"] = info.type().ToString();

                Ice.IPEndpointInfo ipinfo = getIPEndpointInfo(info);
                ctx["host"] = ipinfo.host;
                ctx["port"] = ipinfo.port.ToString();

                if (ipinfo is Ice.UDPEndpointInfo)
                {
                    Ice.UDPEndpointInfo udp = (Ice.UDPEndpointInfo)ipinfo;
                    ctx["mcastInterface"] = udp.mcastInterface;
                    ctx["mcastTtl"] = udp.mcastTtl.ToString();
                }

                return ctx;
            }

            public Dictionary<string, string> getConnectionInfoAsContext(Ice.Current c)
            {
                Dictionary<string, string> ctx = new Dictionary<string, string>();
                Ice.ConnectionInfo info = c.con.getInfo();
                ctx["adapterName"] = info.adapterName;
                ctx["incoming"] = info.incoming ? "true" : "false";

                Ice.IPConnectionInfo ipinfo = getIPConnectionInfo(info);
                ctx["localAddress"] = ipinfo.localAddress;
                ctx["localPort"] = ipinfo.localPort.ToString();
                ctx["remoteAddress"] = ipinfo.remoteAddress;
                ctx["remotePort"] = ipinfo.remotePort.ToString();

                if (info is Ice.WSConnectionInfo)
                {
                    Ice.WSConnectionInfo wsinfo = (Ice.WSConnectionInfo)info;
                    foreach (KeyValuePair<string, string> e in wsinfo.headers)
                    {
                        ctx["ws." + e.Key] = e.Value;
                    }
                }

                return ctx;
            }
        }
    }
}
