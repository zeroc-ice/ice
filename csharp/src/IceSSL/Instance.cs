// **********************************************************************
//
// Copyright (c) 2003-2017 ZeroC, Inc. All rights reserved.
//
// This copy of Ice is licensed to you under the terms described in the
// ICE_LICENSE file included in this distribution.
//
// **********************************************************************

namespace IceSSL
{
    using System.Security.Authentication;
    using System.Security.Cryptography.X509Certificates;

    internal class Instance : IceInternal.ProtocolInstance
    {
        internal Instance(SSLEngine engine, short type, string protocol) :
            base(engine.communicator(), type, protocol, true)
        {
            _engine = engine;
        }

        internal SSLEngine engine()
        {
            return _engine;
        }

        internal int securityTraceLevel()
        {
            return _engine.securityTraceLevel();
        }

        internal string securityTraceCategory()
        {
            return _engine.securityTraceCategory();
        }

        internal bool initialized()
        {
            return _engine.initialized();
        }

        internal X509Certificate2Collection certs()
        {
            return _engine.certs();
        }

        internal SslProtocols protocols()
        {
            return _engine.protocols();
        }

        internal int checkCRL()
        {
            return _engine.checkCRL();
        }

        internal void traceStream(System.Net.Security.SslStream stream, string connInfo)
        {
            _engine.traceStream(stream, connInfo);
        }

        internal void verifyPeer(string address, NativeConnectionInfo info, string desc)
        {
            _engine.verifyPeer(address, info, desc);
        }

        private SSLEngine _engine;
    }
}
