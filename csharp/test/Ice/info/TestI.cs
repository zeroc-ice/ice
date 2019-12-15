//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

namespace Ice
{
    namespace info
    {
        public class TestI : Test.TestIntf
        {
            private static IPEndpointInfo? getIPEndpointInfo(Ice.EndpointInfo info)
            {
                for (EndpointInfo? i = info; i != null; i = i.underlying)
                {
                    if (i is IPEndpointInfo)
                    {
                        return (IPEndpointInfo)i;
                    }
                }
                return null;
            }

            private static IPConnectionInfo? getIPConnectionInfo(ConnectionInfo info)
            {
                for (ConnectionInfo? i = info; i != null; i = i.underlying)
                {
                    if (i is IPConnectionInfo)
                    {
                        return (IPConnectionInfo)i;
                    }
                }
                return null;
            }

            public void shutdown(Ice.Current current)
            {
                current.Adapter.Communicator.shutdown();
            }

            public Dictionary<string, string> getEndpointInfoAsContext(Ice.Current current)
            {
                Debug.Assert(current.Connection != null);
                Dictionary<string, string> ctx = new Dictionary<string, string>();
                Ice.EndpointInfo info = current.Connection.getEndpoint().getInfo();
                ctx["timeout"] = info.timeout.ToString();
                ctx["compress"] = info.compress ? "true" : "false";
                ctx["datagram"] = info.datagram() ? "true" : "false";
                ctx["secure"] = info.datagram() ? "true" : "false";
                ctx["type"] = info.type().ToString();

                IPEndpointInfo? ipinfo = getIPEndpointInfo(info);
                Debug.Assert(ipinfo != null);
                ctx["host"] = ipinfo.host;
                ctx["port"] = ipinfo.port.ToString();

                if (ipinfo is UDPEndpointInfo)
                {
                    UDPEndpointInfo udp = (UDPEndpointInfo)ipinfo;
                    ctx["mcastInterface"] = udp.mcastInterface;
                    ctx["mcastTtl"] = udp.mcastTtl.ToString();
                }

                return ctx;
            }

            public Dictionary<string, string> getConnectionInfoAsContext(Ice.Current current)
            {
                Debug.Assert(current.Connection != null);
                Dictionary<string, string> ctx = new Dictionary<string, string>();
                ConnectionInfo info = current.Connection.getInfo();
                ctx["adapterName"] = info.adapterName;
                ctx["incoming"] = info.incoming ? "true" : "false";

                IPConnectionInfo? ipinfo = getIPConnectionInfo(info);
                Debug.Assert(ipinfo != null);
                ctx["localAddress"] = ipinfo.localAddress;
                ctx["localPort"] = ipinfo.localPort.ToString();
                ctx["remoteAddress"] = ipinfo.remoteAddress;
                ctx["remotePort"] = ipinfo.remotePort.ToString();

                if (info is WSConnectionInfo)
                {
                    WSConnectionInfo wsinfo = (WSConnectionInfo)info;
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
