// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using ZeroC.Test;

namespace ZeroC.Ice.Test.Info
{
    public class TestIntf : ITestIntf
    {
        public void Shutdown(Current current, CancellationToken cancel) =>
            current.Communicator.ShutdownAsync();

        public IReadOnlyDictionary<string, string> GetEndpointInfoAsContext(Current current, CancellationToken cancel)
        {
            TestHelper.Assert(current.Connection != null);
            var ctx = new Dictionary<string, string>();
            Endpoint endpoint = current.Connection.Endpoint;
            ctx["timeout"] = endpoint["timeout"] ?? "infinite";
            ctx["compress"] = endpoint["compress"] ?? "false";
            ctx["datagram"] = endpoint.IsDatagram ? "true" : "false";
            ctx["secure"] = endpoint.IsAlwaysSecure ? "true" : "false";
            ctx["scheme"] = endpoint.Scheme;

            ctx["host"] = endpoint.Host;
            ctx["port"] = endpoint.Port.ToString();

            if (endpoint.Transport == Transport.UDP)
            {
                ctx["mcastInterface"] = endpoint["interface"]!;
                ctx["mcastTtl"] = endpoint["ttl"]!;
            }

            return ctx;
        }

        public IReadOnlyDictionary<string, string> GetConnectionInfoAsContext(
            Current current,
            CancellationToken cancel)
        {
            TestHelper.Assert(current.Connection != null);
            var ctx = new Dictionary<string, string>
            {
                ["adapterName"] = current.Connection.Adapter?.Name ?? "",
                ["incoming"] = current.Connection.IsIncoming ? "true" : "false"
            };

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
