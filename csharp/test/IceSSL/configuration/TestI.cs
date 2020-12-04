// Copyright (c) ZeroC, Inc. All rights reserved.

using System.Collections.Generic;
using System.Threading;
using Test;
using ZeroC.Ice;

namespace ZeroC.IceSSL.Test.Configuration
{
    internal sealed class SSLServer : IServer
    {
        internal SSLServer(Communicator communicator) => _communicator = communicator;

        public void NoCert(Current current, CancellationToken cancel)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection;
                TestHelper.Assert(tcpConnection.IsEncrypted);
                TestHelper.Assert(tcpConnection.RemoteCertificate == null);
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void CheckCert(string subjectDN, string issuerDN, Current current, CancellationToken cancel)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection;
                var ice1 = current.Adapter.Protocol == Protocol.Ice1;
                TestHelper.Assert(tcpConnection.IsEncrypted);
                TestHelper.Assert(tcpConnection.RemoteCertificate != null);
                TestHelper.Assert(tcpConnection.RemoteCertificate.Subject.Equals(subjectDN));
                TestHelper.Assert(tcpConnection.RemoteCertificate.Issuer.Equals(issuerDN));
            }
            catch
            {
                TestHelper.Assert(false);
            }
        }

        public void CheckCipher(string cipher, Current current, CancellationToken cancel)
        {
            try
            {
                var tcpConnection = (TcpConnection)current.Connection;
                var ice1 = current.Adapter.Protocol == Protocol.Ice1;
                TestHelper.Assert(tcpConnection.IsEncrypted);
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
        private readonly string _defaultDir;
        private readonly Dictionary<Identity, SSLServer> _servers = new();

        public ServerFactory(string defaultDir) => _defaultDir = defaultDir;

        public IServerPrx CreateServer(
            Dictionary<string, string> properties,
            bool requireClientCertificate,
            Current current, CancellationToken cancel)
        {
            properties["IceSSL.DefaultDir"] = _defaultDir;
            var communicator = new Communicator(
                properties,
                tlsServerOptions: new TlsServerOptions()
                {
                    RequireClientCertificate = requireClientCertificate
                });

            bool ice1 = TestHelper.GetTestProtocol(communicator.GetProperties()) == Protocol.Ice1;
            string host = TestHelper.GetTestHost(communicator.GetProperties());

            string serverEndpoint = TestHelper.GetTestEndpoint(
                properties: communicator.GetProperties(),
                num: 1,
                transport: ice1 ? "ssl" : "tcp",
                ephemeral: host != "localhost");

            ObjectAdapter adapter = communicator.CreateObjectAdapterWithEndpoints("ServerAdapter", serverEndpoint);
            var server = new SSLServer(communicator);
            IServerPrx prx = adapter.AddWithUUID(server, IServerPrx.Factory);
            _servers[prx.Identity] = server;
            adapter.Activate();
            return prx;
        }

        public void DestroyServer(IServerPrx? srv, Current current, CancellationToken cancel)
        {
            if (_servers.TryGetValue(srv!.Identity, out SSLServer? server))
            {
                server.Destroy();
                _servers.Remove(srv.Identity);
            }
        }

        public void Shutdown(Current current, CancellationToken cancel)
        {
            TestHelper.Assert(_servers.Count == 0);
            current.Communicator.ShutdownAsync();
        }
    }
}
