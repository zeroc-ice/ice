//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using System.Diagnostics;

namespace Ice.info
{
    public class TestIntf : Test.ITestIntf
    {
        private static IPEndpoint? getIPEndpoint(Ice.Endpoint endpoint)
        {
            for (Endpoint? e = endpoint; e != null; e = e.Underlying)
            {
                if (e is IPEndpoint)
                {
                    return (IPEndpoint)e;
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
            Ice.Endpoint endpoint = current.Connection.Endpoint;
            ctx["timeout"] = endpoint.Timeout.ToString();
            ctx["compress"] = endpoint.HasCompressionFlag ? "true" : "false";
            ctx["datagram"] = endpoint.IsDatagram ? "true" : "false";
            ctx["secure"] = endpoint.IsDatagram ? "true" : "false";
            ctx["type"] = endpoint.Type.ToString();

            IPEndpoint? ipEndpoint = getIPEndpoint(endpoint);
            Debug.Assert(ipEndpoint != null);
            ctx["host"] = ipEndpoint.Host;
            ctx["port"] = ipEndpoint.Port.ToString();

            if (ipEndpoint is UdpEndpoint udpEndpoint)
            {
                ctx["mcastInterface"] = udpEndpoint.McastInterface;
                ctx["mcastTtl"] = udpEndpoint.McastTtl.ToString();
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
