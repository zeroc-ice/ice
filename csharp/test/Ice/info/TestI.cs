//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Collections.Generic;
using Test;

namespace ZeroC.Ice.Test.Info
{
    public class TestIntf : ITestIntf
    {
        public void shutdown(Current current) => current.Adapter.Communicator.Shutdown();

        public IReadOnlyDictionary<string, string> getEndpointInfoAsContext(Current current)
        {
            TestHelper.Assert(current.Connection != null);
            var ctx = new Dictionary<string, string>();
            Endpoint endpoint = current.Connection.Endpoint;
            ctx["timeout"] = endpoint.Timeout.ToString();
            ctx["compress"] = endpoint.HasCompressionFlag ? "true" : "false";
            ctx["datagram"] = endpoint.IsDatagram ? "true" : "false";
            ctx["secure"] = endpoint.IsDatagram ? "true" : "false";
            ctx["transport"] = endpoint.Transport.ToString();

            IPEndpoint? ipEndpoint = endpoint as IPEndpoint;
            TestHelper.Assert(ipEndpoint != null);
            ctx["host"] = ipEndpoint.Host;
            ctx["port"] = ipEndpoint.Port.ToString();

            if (ipEndpoint is UdpEndpoint udpEndpoint)
            {
                ctx["mcastInterface"] = udpEndpoint.McastInterface;
                ctx["mcastTtl"] = udpEndpoint.McastTtl.ToString();
            }

            return ctx;
        }

        public IReadOnlyDictionary<string, string> getConnectionInfoAsContext(Current current)
        {
            TestHelper.Assert(current.Connection != null);
            var ctx = new Dictionary<string, string>();
            ctx["adapterName"] = current.Connection.Adapter?.Name ?? "";
            ctx["incoming"] = current.Connection.IsIncoming ? "true" : "false";

            var ipConnection = current.Connection as IPConnection;
            TestHelper.Assert(ipConnection != null);
            ctx["localAddress"] = ipConnection.LocalEndpoint?.Address.ToString() ?? "";
            ctx["localPort"] = ipConnection.LocalEndpoint?.Port.ToString() ?? "";
            ctx["remoteAddress"] = ipConnection.RemoteEndpoint?.Address.ToString() ?? "";
            ctx["remotePort"] = ipConnection.RemoteEndpoint?.Port.ToString() ?? "";

            if ((current.Connection as WSConnection)?.Headers is IReadOnlyDictionary<string, string> headers)
            {
                foreach ((string key, string value) in headers)
                {
                    ctx[$"ws.{key}"] = value;
                }
            }
            return ctx;
        }
    }
}
