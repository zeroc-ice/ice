//
// Copyright (c) ZeroC, Inc. All rights reserved.
//

using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL
{
    internal class Instance : IceInternal.TransportInstance
    {
        internal Instance(SSLEngine engine, Ice.EndpointType type, string transport)
            : base(engine.Communicator(), type, transport, true) => _engine = engine;

        internal SSLEngine Engine() => _engine;

        internal int SecurityTraceLevel() => _engine.SecurityTraceLevel();

        internal string SecurityTraceCategory() => _engine.SecurityTraceCategory();

        internal bool Initialized() => _engine.Initialized();

        internal X509Certificate2Collection? Certs() => _engine.Certs();

        internal SslProtocols Protocols() => _engine.Protocols();

        internal int CheckCRL() => _engine.CheckCRL();

        internal void TraceStream(System.Net.Security.SslStream stream, string connInfo) =>
            _engine.TraceStream(stream, connInfo);

        internal void VerifyPeer(ConnectionInfo info, string desc) => _engine.VerifyPeer(info, desc);

        private readonly SSLEngine _engine;
    }
}
