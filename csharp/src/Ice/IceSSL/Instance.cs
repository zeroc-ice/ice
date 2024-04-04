// Copyright (c) ZeroC, Inc.

using System.Net.Security;
using System.Security.Authentication;
using System.Security.Cryptography.X509Certificates;

namespace IceSSL;

internal class Instance : IceInternal.ProtocolInstance
{
    internal Instance(SSLEngine engine, short type, string protocol) :
        base(engine.communicator(), type, protocol, true) => _engine = engine;

    internal SSLEngine engine() => _engine;

    internal int securityTraceLevel() => _engine.securityTraceLevel();

    internal string securityTraceCategory() => _engine.securityTraceCategory();

    internal bool initialized() => _engine.initialized();

    internal X509Certificate2Collection certs() => _engine.certs();

    internal SslProtocols protocols() => _engine.protocols();

    internal int checkCRL() => _engine.checkCRL();

    internal void traceStream(SslStream stream, string connInfo) => _engine.traceStream(stream, connInfo);

    internal void verifyPeer(string address, ConnectionInfo info, string description) =>
        _engine.verifyPeer(address, info, description);

    internal Ice.InitializationData initializationData() =>
        IceInternal.Util.getInstance(_engine.communicator()).initializationData();

    private SSLEngine _engine;
}
