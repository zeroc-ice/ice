//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System;
using System.Collections.Generic;
using ZeroC.Ice;
using Test;

namespace ZeroC.IceSSL.Test.Configuration
{
    internal sealed class SSLServer : IServer
    {
        internal SSLServer(Communicator communicator) => _communicator = communicator;

        public void NoCert(Current current)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection!;
                TestHelper.Assert(tcpConnection.Endpoint.IsSecure);
                TestHelper.Assert(tcpConnection.RemoteCertificate == null);
            }
            catch (Exception)
            {
                TestHelper.Assert(false);
            }
        }

        public void CheckCert(string subjectDN, string issuerDN, Current current)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection!;
                TestHelper.Assert(tcpConnection.Endpoint.IsSecure);
                TestHelper.Assert(tcpConnection.RemoteCertificate != null);
                TestHelper.Assert(tcpConnection.RemoteCertificate.Subject.Equals(subjectDN));
                TestHelper.Assert(tcpConnection.RemoteCertificate.Issuer.Equals(issuerDN));
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void CheckCipher(string cipher, Current current)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection!;
                TestHelper.Assert(tcpConnection.Endpoint.IsSecure);
                TestHelper.Assert(tcpConnection.NegotiatedCipherSuite!.ToString()!.Equals(cipher));
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        internal void Destroy() => _communicator.Dispose();

        private readonly Communicator _communicator;
    }

    internal sealed class ServerFactory : IServerFactory
    {
        public ServerFactory(string defaultDir) => _defaultDir = defaultDir;

        public IServerPrx CreateServer(
            Dictionary<string, string> properties,
            bool requireClientCertificate,
            Current current)
        {
            properties["IceSSL.DefaultDir"] = _defaultDir;
            var communicator = new Communicator(
                properties,
                tlsServerOptions: new TlsServerOptions()
                {
                    RequireClientCertificate = requireClientCertificate
                });
            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("ServerAdapter", "ssl");
            var server = new SSLServer(communicator);
            var prx = adapter.AddWithUUID(server, IServerPrx.Factory);
            _servers[prx.Identity] = server;
            adapter.Activate();
            return prx;
        }

        public void DestroyServer(IServerPrx? srv, Current current)
        {
            if (_servers.TryGetValue(srv!.Identity, out SSLServer? server))
            {
                server.Destroy();
                _servers.Remove(srv.Identity);
            }
        }

        public void Shutdown(Current current)
        {
            TestHelper.Assert(_servers.Count == 0);
            current.Adapter.Communicator.ShutdownAsync();
        }

        private readonly string _defaultDir;
        private Dictionary<Identity, SSLServer> _servers = new Dictionary<Identity, SSLServer>();
    }
}
