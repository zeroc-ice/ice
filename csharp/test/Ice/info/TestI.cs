//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

namespace Ice.info
{
    public class TestIntf : Test.ITestIntf
    {
        private static IPEndpointInfo? getIPEndpointInfo(Ice.EndpointInfo info)
        {
            for (EndpointInfo? i = info; i != null; i = i.Underlying)
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
            for (ConnectionInfo? i = info; i != null; i = i.Underlying)
            {
                if (i is IPConnectionInfo)
                {
                    return (IPConnectionInfo)i;
                }
            }
            return null;
        }

        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public Dictionary<string, string> getEndpointInfoAsContext(Current current)
        {
            Debug.Assert(current.Connection != null);
            Dictionary<string, string> ctx = new Dictionary<string, string>();
            Ice.EndpointInfo info = current.Connection.Endpoint.GetInfo();
            ctx["timeout"] = info.Timeout.ToString();
            ctx["compress"] = info.Compress ? "true" : "false";
            ctx["datagram"] = info.Datagram() ? "true" : "false";
            ctx["secure"] = info.Datagram() ? "true" : "false";
            ctx["type"] = info.Type().ToString();

            IPEndpointInfo? ipinfo = getIPEndpointInfo(info);
            Debug.Assert(ipinfo != null);
            ctx["host"] = ipinfo.Host;
            ctx["port"] = ipinfo.Port.ToString();

            if (ipinfo is UDPEndpointInfo)
            {
                UDPEndpointInfo udp = (UDPEndpointInfo)ipinfo;
                ctx["mcastInterface"] = udp.McastInterface;
                ctx["mcastTtl"] = udp.McastTtl.ToString();
            }

            return ctx;
        }

        public Dictionary<string, string> getConnectionInfoAsContext(Current current)
        {
            Debug.Assert(current.Connection != null);
            Dictionary<string, string> ctx = new Dictionary<string, string>();
            ConnectionInfo info = current.Connection.GetConnectionInfo();
            ctx["adapterName"] = info.AdapterName;
            ctx["incoming"] = info.Incoming ? "true" : "false";

            IPConnectionInfo? ipinfo = getIPConnectionInfo(info);
            Debug.Assert(ipinfo != null);
            ctx["localAddress"] = ipinfo.LocalAddress;
            ctx["localPort"] = ipinfo.LocalPort.ToString();
            ctx["remoteAddress"] = ipinfo.RemoteAddress;
            ctx["remotePort"] = ipinfo.RemotePort.ToString();

            if (info is WSConnectionInfo)
            {
                WSConnectionInfo wsinfo = (WSConnectionInfo)info;
                foreach (KeyValuePair<string, string> e in wsinfo.Headers)
                {
                    ctx["ws." + e.Key] = e.Value;
                }
            }

            return ctx;
        }
    }
}
